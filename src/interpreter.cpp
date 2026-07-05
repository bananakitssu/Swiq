#include "interpreter.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>

void Interpreter::run(const std::vector<std::unique_ptr<Stmt>>& statements) {
    registerFunctions(statements);
    registerTypes(statements);
    executeBlock(statements);
}

void Interpreter::registerTypes(const std::vector<std::unique_ptr<Stmt>>& statements) {
    for (const auto& stmt : statements) {
        if (auto t = dynamic_cast<const TypeDeclStmt*>(stmt.get())) {
            types[t->name] = t;
        }
    }
}

void Interpreter::registerFunctions(const std::vector<std::unique_ptr<Stmt>>& statements) {
    for (const auto& stmt : statements) {
        if (auto func = dynamic_cast<const FuncDeclStmt*>(stmt.get())) {
            if (functions.find(func->name) != functions.end()) {
                if (func->overriding) {
                    return;
                }
                throw std::runtime_error("Interpreter error at line " + std::to_string(func->line) + ": function '" + func->name + "' already defined.");
            }
            functions[func->name] = func;
        }
    }
}

void Interpreter::executeBlock(const std::vector<std::unique_ptr<Stmt>>& statements, int type) {
    for (const auto& stmt : statements) {
        execute(stmt.get(), type);
    }
}

void Interpreter::executeSwitcherStmt(const SwitcherStmt* stmt) {
    Value targetVal = evaluate(stmt->controlExpr.get());

    for (const auto& c : stmt->cases) {
        Value caseVal = evaluate(c.matchValue.get());
        
        bool match = false;
        if (targetVal.data.index() == caseVal.data.index()) {
            if (auto s1 = std::get_if<std::string>(&targetVal.data)) {
                match = (*s1 == std::get<std::string>(caseVal.data));
            } else if (auto i1 = std::get_if<long long>(&targetVal.data)) {
                match = (*i1 == std::get<long long>(caseVal.data));
            } else if (auto d1 = std::get_if<double>(&targetVal.data)) {
                match = (*d1 == std::get<double>(caseVal.data));
            } else if (auto b1 = std::get_if<bool>(&targetVal.data)) {
                match = (*b1 == std::get<bool>(caseVal.data));
            }
        }

        if (match) {
            try {
                for (const auto& caseStmt : c.body) {
                    execute(caseStmt.get(), 1);
                }
            } catch (const DestroySignal&) {
                break;
            }
            break;
        }
    }
}

void Interpreter::execute(const Stmt* stmt, int type) {
    if (auto varDecl = dynamic_cast<const VarDeclStmt*>(stmt)) {
        variables[varDecl->name] = evaluate(varDecl->value.get());
        default_variables[varDecl->name] = evaluate(varDecl->value.get());
        protected_variables[varDecl->name] = varDecl->isProtected;
        if (varDecl->isLocal) {
            local_variables.insert(varDecl->name);
        } else {
            local_variables.erase(varDecl->name);
        }
        return;
    }
    
    if (auto tryStmt = dynamic_cast<const TryStmt*>(stmt)) {
        executeTryStmt(tryStmt);
        return;
    }

    if (auto assign = dynamic_cast<const AssignStmt*>(stmt)) {
        if (protected_variables[assign->name]) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(assign->line) +
                                     ": cannot assign to protected variable '" + assign->name + "'");
        }
        if (variables.find(assign->name) == variables.end()) {
            if (archived_variables.find(assign->name) != archived_variables.end()) {
                throw std::runtime_error(
                    "Interpreter error at line " + std::to_string(assign->line) +
                    ": cannot assign to undeclared variable '" + assign->name +
                    "'. Use 'set var' to declare it first.");
            }
        }
        if (archived_variables.find(assign->name) != archived_variables.end()) {
            throw std::runtime_error(
                "Interpreter error at line " + std::to_string(assign->line) +
                ": cannot assign to archived variable '" + assign->name +
                "'. Use 'restore' to unarchive it.");
        }
        variables[assign->name] = evaluate(assign->value.get());
        return;
    }
    
    if (auto switcher = dynamic_cast<const SwitcherStmt*>(stmt)) {
        executeSwitcherStmt(switcher);
        return;
    }
    
    if (auto _destroy = dynamic_cast<const DestroyStmt*>(stmt)) {
	if (type == 1) {
            throw DestroySignal{};
	} else {
	    throw std::runtime_error(
		"Interpreter error at line " + std::to_string(_destroy->line) +
		": cannot have 'destroy', it's for functions, switchers and try statements only.");
	}
    }
    
    if (auto reset = dynamic_cast<const ResetStmt*>(stmt)) {
        if (variables.find(reset->name) == variables.end()) {
            throw std::runtime_error(
                "Cannot reset an unknown variable. Error at line " + std::to_string(reset->line)
            );
        }
        variables[reset->name] = default_variables[reset->name];
        return;
    }
    
    if (auto _delete = dynamic_cast<const DeleteStmt*>(stmt)) {
        if (variables.find(_delete->name) == variables.end()) {
            throw std::runtime_error(
                "Cannot delete an unknown variable. Error at line " + std::to_string(_delete->line)
            );
        }
        if (protected_variables[_delete->name]) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(_delete->line) +
                                     ": cannot delete protected variable '" + _delete->name + "'");
        }
        variables.erase(_delete->name);
        protected_variables.erase(_delete->name);
        return;
    }
    
    if (auto archive = dynamic_cast<const ArchiveStmt*>(stmt)) {
        if (variables.find(archive->name) == variables.end()) {
            throw std::runtime_error(
                "Cannot archive an unknown variable. Error at line " + std::to_string(archive->line)
            );
        }
        archived_variables[archive->name] = variables[archive->name];
        variables.erase(archive->name);
        return;
    }
    
    if (auto restore = dynamic_cast<const RestoreStmt*>(stmt)) {
        if (archived_variables.find(restore->name) == archived_variables.end()) {
            throw std::runtime_error(
                "Cannot restore an variable that has not been archived. Error at line " + std::to_string(restore->line)
            );
        }
        variables[restore->name] = archived_variables[restore->name];
        archived_variables.erase(restore->name);
        return;
    }

    if (auto idxAssign = dynamic_cast<const IndexAssignStmt*>(stmt)) {
        Value arrVal = evaluate(idxAssign->array.get());
        auto arrPtr = std::get_if<std::shared_ptr<ArrayObject>>(&arrVal.data);
        if (!arrPtr) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(idxAssign->line) +
                                      ": cannot index into a non-array value");
        }
        Value idxVal = evaluate(idxAssign->index.get());
        auto idxNum = std::get_if<long long>(&idxVal.data);
        if (!idxNum) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(idxAssign->line) +
                                      ": array index must be a number");
        }
        auto& elements = (*arrPtr)->elements;
        if (*idxNum < 0 || (size_t)*idxNum >= elements.size()) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(idxAssign->line) +
                                      ": array index out of bounds");
        }
        elements[(size_t)*idxNum] = evaluate(idxAssign->value.get());
        return;
    }

    if (auto log = dynamic_cast<const LogStmt*>(stmt)) {
        printValue(evaluate(log->value.get()));
        std::cout << std::endl;
        return;
    }

    if (auto exprStmt = dynamic_cast<const ExprStmt*>(stmt)) {
        evaluate(exprStmt->expr.get());
        return;
    }

    if (auto ifStmt = dynamic_cast<const IfStmt*>(stmt)) {
        if (isTruthy(evaluate(ifStmt->condition.get()))) {
            executeBlock(ifStmt->thenBranch);
        } else {
            executeBlock(ifStmt->elseBranch);
        }
        return;
    }

    if (auto whileStmt = dynamic_cast<const WhileStmt*>(stmt)) {
        while (isTruthy(evaluate(whileStmt->condition.get()))) {
            executeBlock(whileStmt->body, 1);
        }
        return;
    }

    if (auto forStmt = dynamic_cast<const ForStmt*>(stmt)) {
        execute(forStmt->init.get());
        while (isTruthy(evaluate(forStmt->condition.get()))) {
            executeBlock(forStmt->body, 1);
            execute(forStmt->post.get());
        }
        return;
    }

    if (auto funcDecl = dynamic_cast<const FuncDeclStmt*>(stmt)) {
        if (functions.find(funcDecl->name) == functions.end()) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(funcDecl->line) + ": overriding function '" + funcDecl->name + "' which is not defined.");
        }
        if (funcDecl->overriding) {
            functions[funcDecl->name] = funcDecl;
        }
    }

    if (dynamic_cast<const TypeDeclStmt*>(stmt)) {
        // Already registered in registerTypes(); nothing to do at execution time.
        return;
    }

    if (dynamic_cast<const FuncDeclStmt*>(stmt)) {
        // Already registered in registerFunctions(); nothing to do at execution time.
        return;
    }

    if (auto retStmt = dynamic_cast<const ReturnStmt*>(stmt)) {
        Value val = retStmt->value ? evaluate(retStmt->value.get()) : Value();
        throw ReturnSignal{val};
    }

    throw std::runtime_error("Interpreter error: unknown statement type");
}

void Interpreter::printValue(const Value& value) {
    std::cout << valueToDisplayString(value);
}

// Builds a dotted path string like "Swiq.__ENV__.__VERSION__.__BUILD_NUMBER__"
std::string Interpreter::getMemberPath(const Expr* expr) {
    if (auto var = dynamic_cast<const VariableExpr*>(expr)) {
        return var->name;
    }
    if (auto mem = dynamic_cast<const MemberExpr*>(expr)) {
        return getMemberPath(mem->object.get()) + "." + mem->property;
    }
    throw std::runtime_error("Interpreter error: invalid member access expression");
}

Value Interpreter::callBuiltin(const std::string& name, std::vector<Value>& args, int line) {
    if (name == "len") {
        if (args.size() != 1) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                      ": len() expects exactly 1 argument");
        }
        if (auto a = std::get_if<std::shared_ptr<ArrayObject>>(&args[0].data)) {
            return Value((long long)(*a)->elements.size());
        }
        if (auto s = std::get_if<std::string>(&args[0].data)) {
            return Value((long long)s->size());
        }
        throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                  ": len() requires an array or string");
    }

    if (name == "push") {
        if (args.size() != 2) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                      ": push() expects exactly 2 arguments (array, value)");
        }
        auto a = std::get_if<std::shared_ptr<ArrayObject>>(&args[0].data);
        if (!a) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                      ": push() requires an array as its first argument");
        }
        if ((*a)->fixed) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                      ": push() cannot grow a fixed-size array created with AllocatedArray()");
        }
        (*a)->elements.push_back(args[1]);
        return Value((long long)(*a)->elements.size());
    }

    if (name == "AllocatedArray") {
        if (args.size() != 1) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                      ": AllocatedArray() expects exactly 1 argument (size)");
        }
        auto sizeNum = std::get_if<long long>(&args[0].data);
        if (!sizeNum) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                      ": AllocatedArray() requires a number as its size");
        }
        if (*sizeNum < 0) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                      ": AllocatedArray() size cannot be negative");
        }
        auto obj = std::make_shared<ArrayObject>();
        obj->fixed = true;
        obj->elements.assign((size_t)*sizeNum, Value()); // every slot starts as null
        return Value(obj);
    }

    if (name == "typeof") {
        if (args.size() != 1) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                      ": typeof() expects exactly 1 argument");
        }
        return Value(typeNameOf(args[0]));
    }

    if (name == "readFile") {
        if (args.size() != 1) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                      ": readFile() expects exactly 1 argument (path)");
        }
        auto path = std::get_if<std::string>(&args[0].data);
        if (!path) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                      ": readFile() requires a string path");
        }
        std::ifstream file(*path);
        if (!file) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                                      ": could not open file '" + *path + "'");
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return Value(buffer.str());
    }

    throw std::runtime_error("Interpreter error at line " + std::to_string(line) +
                              ": unknown function '" + name + "'");
}

Value Interpreter::callFunction(const FuncDeclStmt* func, std::vector<Value> args, int callLine) {
    if (args.size() != func->params.size()) {
        throw std::runtime_error("Interpreter error at line " + std::to_string(callLine) +
                                  ": function '" + func->name + "' expects " +
                                  std::to_string(func->params.size()) + " argument(s), got " +
                                  std::to_string(args.size()));
    }

    // Functions run in a fresh, isolated scope. Only parameters and any
    // explicitly listed [captures] are visible — everything else from the
    // outer scope is invisible by default.
    auto savedVars = std::move(variables);
    variables = std::unordered_map<std::string, Value>();

    if (func->capturingAll) {
	variables = std::move(savedVars);
    }

    // Bring in captured variables by their current value.
    for (const auto& name : func->captures) {
        auto it = savedVars.find(name);
        if (it == savedVars.end()) {
            variables = std::move(savedVars); // restore before throwing
            throw std::runtime_error("Interpreter error at line " + std::to_string(callLine) +
                                      ": function '" + func->name + "' captures undefined variable '" +
                                      name + "'");
        }
        variables[name] = it->second;
    }

    // Bind parameters (params take priority if a name overlaps a capture).
    for (size_t i = 0; i < func->params.size(); i++) {
        variables[func->params[i]] = args[i];
    }

    Value result;
    try {
        executeBlock(func->body, 1);
        result = Value(); // no explicit return statement was hit
    } catch (ReturnSignal& r) {
        result = r.value;
    } catch (DestroySignal& d) {
	result = "";
    } catch (...) {
        variables = std::move(savedVars);
        throw;
    }

    // Write captured variables' final values back to the outer scope —
    // this is what makes them behave like real (if limited) closures.
    for (const auto& name : func->captures) {
        savedVars[name] = variables[name];
    }

    variables = std::move(savedVars);
    return result;
}

Value Interpreter::evaluate(const Expr* expr) {
    if (auto num = dynamic_cast<const NumberExpr*>(expr)) {
        return Value(num->value);
    }

    if (auto flt = dynamic_cast<const FloatExpr*>(expr)) {
        return Value(flt->value);
    }

    if (auto unary = dynamic_cast<const UnaryExpr*>(expr)) {
        Value operand = evaluate(unary->operand.get());
        if (unary->op == "-") {
            if (auto i = std::get_if<long long>(&operand.data)) return Value(-*i);
            if (auto d = std::get_if<double>(&operand.data)) return Value(-*d);
            throw std::runtime_error("Interpreter error at line " + std::to_string(unary->line) +
                                      ": unary '-' requires a number");
        }
        throw std::runtime_error("Interpreter error at line " + std::to_string(unary->line) +
                                  ": unknown unary operator '" + unary->op + "'");
    }

    if (auto tobj = dynamic_cast<const TypedObjectExpr*>(expr)) {
        return constructTypedObject(tobj);
    }

    if (auto str = dynamic_cast<const StringExpr*>(expr)) {
        return Value(str->value);
    }

    if (auto boolean = dynamic_cast<const BoolExpr*>(expr)) {
        return Value(boolean->value);
    }

    if (dynamic_cast<const NullExpr*>(expr)) {
        return Value(); // null / none / nil all evaluate to the same thing
    }

    if (auto hex = dynamic_cast<const HexExpr*>(expr)) {
        // Hex literals are stored as plain text (e.g. "0x1F"), not a number.
        // This is intentional: + on them concatenates like any other string,
        // and arithmetic on them fails until .ConvertToNumber() is called.
        return Value(hex->raw);
    }

    if (auto arr = dynamic_cast<const ArrayExpr*>(expr)) {
        auto obj = std::make_shared<ArrayObject>();
        for (const auto& elemExpr : arr->elements) {
            obj->elements.push_back(evaluate(elemExpr.get()));
        }
        return Value(obj); // fixed defaults to false: [...] literals are growable
    }

    if (auto idx = dynamic_cast<const IndexExpr*>(expr)) {
        Value arrVal = evaluate(idx->array.get());
        auto arrPtr = std::get_if<std::shared_ptr<ArrayObject>>(&arrVal.data);
        if (!arrPtr) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(idx->line) +
                                      ": cannot index into a non-array value");
        }
        Value idxVal = evaluate(idx->index.get());
        auto idxNum = std::get_if<long long>(&idxVal.data);
        if (!idxNum) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(idx->line) +
                                      ": array index must be a number");
        }
        auto& elements = (*arrPtr)->elements;
        if (*idxNum < 0 || (size_t)*idxNum >= elements.size()) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(idx->line) +
                                      ": array index out of bounds");
        }
        return elements[(size_t)*idxNum];
    }

    if (auto mcall = dynamic_cast<const MethodCallExpr*>(expr)) {
        Value obj = evaluate(mcall->object.get());

        if (mcall->method == "ConvertToNumber") {
            if (!mcall->args.empty()) {
                throw std::runtime_error("Interpreter error at line " + std::to_string(mcall->line) +
                                          ": ConvertToNumber() takes no arguments");
            }
            auto s = std::get_if<std::string>(&obj.data);
            if (!s) {
                throw std::runtime_error("Interpreter error at line " + std::to_string(mcall->line) +
                                          ": ConvertToNumber() can only be called on a hex (or numeric string) value");
            }
            try {
                long long parsed;
                if (s->size() > 2 && s->rfind("0x", 0) == 0) {
                    parsed = std::stoll(s->substr(2), nullptr, 16);
                } else if (s->size() > 2 && s->rfind("0X", 0) == 0) {
                    parsed = std::stoll(s->substr(2), nullptr, 16);
                } else {
                    parsed = std::stoll(*s, nullptr, 10);
                }
                return Value(parsed);
            } catch (const std::exception&) {
                throw std::runtime_error("Interpreter error at line " + std::to_string(mcall->line) +
                                          ": '" + *s + "' is not a valid number for ConvertToNumber()");
            }
        }

        throw std::runtime_error("Interpreter error at line " + std::to_string(mcall->line) +
                                  ": unknown method '" + mcall->method + "'");
    }

    if (auto call = dynamic_cast<const CallExpr*>(expr)) {
        std::vector<Value> args;
        for (const auto& argExpr : call->args) {
            args.push_back(evaluate(argExpr.get()));
        }

        if (call->callee == "len" || call->callee == "push" || call->callee == "AllocatedArray" ||
            call->callee == "readFile" || call->callee == "typeof") {
            return callBuiltin(call->callee, args, call->line);
        }

        auto it = functions.find(call->callee);
        if (it == functions.end()) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(call->line) +
                                      ": undefined function '" + call->callee + "'");
        }
        return callFunction(it->second, std::move(args), call->line);
    }

    if (auto var = dynamic_cast<const VariableExpr*>(expr)) {
        if (archived_variables.find(var->name) != archived_variables.end()) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(var->line) +
                                      ": the variable '" + var->name + "' is archived, Use 'restore' to unarchive it");
        }
        auto it = variables.find(var->name);
        if (it == variables.end()) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(var->line) +
                                      ": undefined variable '" + var->name + "'");
        }
        return it->second;
    }

    /*if (auto mem = dynamic_cast<const MemberExpr*>(expr)) {
        // First check if the object is a typed instance — if so, field access
        // takes priority over the static Swiq.__ENV__... path lookup.
        Value objVal = evaluate(mem->object.get());
        if (auto o = std::get_if<std::shared_ptr<TypedObject>>(&objVal.data)) {
            Value* field = (*o)->find(mem->property);
            if (!field) {
                throw std::runtime_error("Interpreter error at line " + std::to_string(mem->line) +
                                          ": type '" + (*o)->typeName + "' has no field '" + mem->property + "'");
            }
            return *field;
        }

        // Fall through to Swiq.__ENV__... static path lookup.
        std::string path = getMemberPath(mem);
        static const std::unordered_map<std::string, Value> envMap = {
            {"Swiq.__ENV__.__VERSION__.__BUILD_NUMBER__", Value((long long)1)},
            {"Swiq.__ENV__.__VERSION__.__BUILD_YEAR__",  Value((long long)2026)},
        };
        auto it = envMap.find(path);
        if (it != envMap.end()) return it->second;

        throw std::runtime_error("Interpreter error at line " + std::to_string(mem->line) +
                                  ": unknown or inaccessible property '" + path + "'");
    }*/

	if (auto mem = dynamic_cast<const MemberExpr*>(expr)) {
     	   // 1. Try the static Swiq.__ENV__... path lookup FIRST.
           // getMemberPath only inspects the AST nodes statically; it won't trigger a variable lookup error.
           std::string path = getMemberPath(mem);
           static const std::unordered_map<std::string, Value> envMap = {
               {"Swiq.__ENV__.__VERSION__.__BUILD_NUMBER__", Value((long long)1)},
               {"Swiq.__ENV__.__VERSION__.__BUILD_YEAR__",  Value((long long)2026)},
           };
           auto it = envMap.find(path);
           if (it != envMap.end()) return it->second;

           // 2. Fallback: If it's not a built-in static path, THEN evaluate it as a runtime object instance
           Value objVal = evaluate(mem->object.get());
           if (auto o = std::get_if<std::shared_ptr<TypedObject>>(&objVal.data)) {
               Value* field = (*o)->find(mem->property);
               if (!field) {
                   throw std::runtime_error("Interpreter error at line " + std::to_string(mem->line) +
                                             ": type '" + (*o)->typeName + "' has no field '" + mem->property + "'");
               }
               return *field;
           }

           throw std::runtime_error("Interpreter error at line " + std::to_string(mem->line) +
                                  ": unknown or inaccessible property '" + path + "'");
       }

    if (auto bin = dynamic_cast<const BinaryExpr*>(expr)) {
        Value left = evaluate(bin->left.get());
        Value right = evaluate(bin->right.get());
        const std::string& op = bin->op;

        // Numeric arithmetic — promotes to double if either side is a float
        if (op == "+" && !std::holds_alternative<std::string>(left.data) &&
                         !std::holds_alternative<std::string>(right.data)) {
            bool eitherFloat = std::holds_alternative<double>(left.data) ||
                               std::holds_alternative<double>(right.data);
            if (eitherFloat) {
                auto toDouble = [](const Value& v) -> double {
                    if (auto d = std::get_if<double>(&v.data)) return *d;
                    if (auto i = std::get_if<long long>(&v.data)) return (double)*i;
                    throw std::runtime_error("BinaryExpr: non-numeric in float add");
                };
                return Value(toDouble(left) + toDouble(right));
            }
        }

        if (op == "+" && (std::holds_alternative<std::string>(left.data) ||
                          std::holds_alternative<std::string>(right.data))) {
            return Value(valueToDisplayString(left) + valueToDisplayString(right));
        }

        if (op == "==" || op == "!=") {
            bool equal;
            if (left.data.index() != right.data.index()) {
                equal = false;
            } else if (std::holds_alternative<long long>(left.data)) {
                equal = std::get<long long>(left.data) == std::get<long long>(right.data);
            } else if (std::holds_alternative<double>(left.data)) {
                equal = std::get<double>(left.data) == std::get<double>(right.data);
            } else if (std::holds_alternative<std::string>(left.data)) {
                equal = std::get<std::string>(left.data) == std::get<std::string>(right.data);
            } else if (std::holds_alternative<bool>(left.data)) {
                equal = std::get<bool>(left.data) == std::get<bool>(right.data);
            } else if (std::holds_alternative<std::monostate>(left.data)) {
                equal = true;
            } else {
                equal = std::get<std::shared_ptr<ArrayObject>>(left.data) ==
                        std::get<std::shared_ptr<ArrayObject>>(right.data);
            }
            return Value(op == "==" ? equal : !equal);
        }

        // For -, *, /, <, >, <=, >= — promote to double if either side is float
        {
            bool eitherFloat = std::holds_alternative<double>(left.data) ||
                               std::holds_alternative<double>(right.data);
            if (eitherFloat) {
                auto toDouble = [](const Value& v) -> double {
                    if (auto d = std::get_if<double>(&v.data)) return *d;
                    if (auto i = std::get_if<long long>(&v.data)) return (double)*i;
                    throw std::runtime_error("BinaryExpr: non-numeric operand");
                };
                double l = toDouble(left), r = toDouble(right);
                if (op == "-") return Value(l - r);
                if (op == "*") return Value(l * r);
                if (op == "/") {
                    if (r == 0.0) throw std::runtime_error("Interpreter error at line " +
                        std::to_string(bin->line) + ": division by zero");
                    return Value(l / r);
                }
                if (op == "<")  return Value(l < r);
                if (op == ">")  return Value(l > r);
                if (op == "<=") return Value(l <= r);
                if (op == ">=") return Value(l >= r);
            }
        }

        if (!std::holds_alternative<long long>(left.data) || !std::holds_alternative<long long>(right.data)) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(bin->line) +
                                      ": operator '" + op + "' requires numbers");
        }

        long long l = std::get<long long>(left.data);
        long long r = std::get<long long>(right.data);

        if (op == "+") return Value(l + r);
        if (op == "-") return Value(l - r);
        if (op == "*") return Value(l * r);
        if (op == "/") {
            if (r == 0) throw std::runtime_error("Interpreter error at line " + std::to_string(bin->line) +
                                                   ": division by zero");
            return Value(l / r);
        }
        if (op == "<")  return Value(l < r);
        if (op == ">")  return Value(l > r);
        if (op == "<=") return Value(l <= r);
        if (op == ">=") return Value(l >= r);

        throw std::runtime_error("Interpreter error at line " + std::to_string(bin->line) +
                                  ": unknown operator '" + op + "'");
    }

    throw std::runtime_error("Interpreter error: unknown expression type");
}

std::string Interpreter::typeNameOf(const Value& val) const {
    if (std::holds_alternative<long long>(val.data))  return "Number";
    if (std::holds_alternative<double>(val.data))     return "Float";
    if (std::holds_alternative<std::string>(val.data)) return "String";
    if (std::holds_alternative<bool>(val.data))       return "Boolean";
    if (std::holds_alternative<std::monostate>(val.data)) return "Null";
    if (std::holds_alternative<std::shared_ptr<ArrayObject>>(val.data)) return "Array";
    if (auto o = std::get_if<std::shared_ptr<TypedObject>>(&val.data)) return (*o)->typeName;
    return "Unknown";
}

bool Interpreter::fieldMatchesType(const Value& val, const TypeField& field) const {
    // null always passes — it means "not set yet"
    if (std::holds_alternative<std::monostate>(val.data)) return true;

    if (field.baseType == "Number") {
        if (field.modifier == "integerOnly") {
            return std::holds_alternative<long long>(val.data);
        }
        if (field.modifier == "floatOnly") {
            return std::holds_alternative<double>(val.data);
        }
        // No modifier: both int and float are fine
        return std::holds_alternative<long long>(val.data) ||
               std::holds_alternative<double>(val.data);
    }
    if (field.baseType == "String")  return std::holds_alternative<std::string>(val.data);
    if (field.baseType == "Boolean") return std::holds_alternative<bool>(val.data);
    if (field.baseType == "Array")   return std::holds_alternative<std::shared_ptr<ArrayObject>>(val.data);
    return false;
}

Value Interpreter::constructTypedObject(const TypedObjectExpr* expr) {
    auto it = types.find(expr->typeName);
    if (it == types.end()) {
        throw std::runtime_error("Interpreter error at line " + std::to_string(expr->line) +
                                  ": unknown type or interface '" + expr->typeName + "'");
    }
    const TypeDeclStmt* decl = it->second;

    // Build a name→value map from the supplied fields so we can look them up quickly
    std::unordered_map<std::string, Value> supplied;
    for (const auto& [name, valExpr] : expr->fields) {
        // Check for unknown fields right here
        bool found = false;
        for (const auto& tf : decl->fields) {
            if (tf.name == name) { found = true; break; }
        }
        if (!found) {
            throw std::runtime_error("Interpreter error at line " + std::to_string(expr->line) +
                                      ": field '" + name + "' does not exist in type '" + expr->typeName + "'");
        }
        supplied[name] = evaluate(valExpr.get());
    }

    // Build the typed object in declaration order
    auto obj = std::make_shared<TypedObject>();
    obj->typeName = expr->typeName;

    for (const auto& tf : decl->fields) {
        Value fieldVal;

        auto sit = supplied.find(tf.name);
        if (sit != supplied.end()) {
            // Caller supplied a value — validate it
            fieldVal = sit->second;
            if (!fieldMatchesType(fieldVal, tf)) {
                std::string modifier = tf.modifier.empty() ? "" : "<" + tf.modifier + ">";
                throw std::runtime_error("Interpreter error at line " + std::to_string(expr->line) +
                                          ": field '" + tf.name + "' expects " + tf.baseType + modifier +
                                          " but got " + typeNameOf(fieldVal));
            }
        } else if (tf.defaultValue) {
            // Not supplied but has a declared default — evaluate it
            fieldVal = evaluate(tf.defaultValue.get());
        } else if (decl->isInterface) {
            // Interface fields must always resolve to a value — no nulls allowed
            throw std::runtime_error("Interpreter error at line " + std::to_string(expr->line) +
                                      ": interface '" + expr->typeName + "' field '" + tf.name +
                                      "' has no default and was not supplied");
        }
        // type fields with no supplied value default to null (monostate) implicitly

        obj->fields.emplace_back(tf.name, std::move(fieldVal));
    }

    return Value(obj);
}

void Interpreter::executeTryStmt(const TryStmt* stmt) {
    try {
        executeBlock(stmt->tryBlock, 1);
    } catch (const std::runtime_error& error) {
        if (stmt->errorVarName.empty()) {
            return;
        }

        bool hasOldValue = (variables.find(stmt->errorVarName) != variables.end());
        Value oldValSaved;
        if (hasOldValue) {
            oldValSaved = variables[stmt->errorVarName];
        }

        variables[stmt->errorVarName] = Value(std::string(error.what()));

        try {
            executeBlock(stmt->catchBlock, 1);
        } catch (...) {
            if (hasOldValue) {
                variables[stmt->errorVarName] = oldValSaved;
            } else {
                variables.erase(stmt->errorVarName);
            }
            throw;
        }

        if (hasOldValue) {
            variables[stmt->errorVarName] = oldValSaved;
        } else {
            variables.erase(stmt->errorVarName);
        }
    }
}
