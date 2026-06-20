#pragma once
#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <utility>

// Forward-declared so Value can hold pointers to them before they're fully defined.
struct ArrayObject;
struct TypedObject;

// Value can be an integer, a float, a string, a bool, null, a
// reference-counted array, or an instance of a declared type/interface.
// Integers and floats are deliberately kept as separate alternatives
// (long long vs. double) rather than unified into one "number" — this is
// what makes `<integerOnly>` field constraints meaningful, and it also
// means `5 == 5.0` is false (different underlying kinds), consistent with
// how every other pair of differing kinds compares unequal in Swiq.
//
// Arrays and typed objects use shared_ptr so they have reference semantics
// (mutating one affects every variable referencing it).
struct Value {
    std::variant<long long, double, std::string, bool,
                 std::shared_ptr<ArrayObject>, std::shared_ptr<TypedObject>, std::monostate> data;

    Value() : data(std::monostate{}) {}              // default value = null
    Value(long long v) : data(v) {}
    Value(int v) : data((long long)v) {}
    Value(double v) : data(v) {}
    Value(std::string v) : data(std::move(v)) {}
    Value(bool v) : data(v) {}
    Value(std::shared_ptr<ArrayObject> v) : data(std::move(v)) {}
    Value(std::shared_ptr<TypedObject> v) : data(std::move(v)) {}
};

// The actual array storage. `fixed` distinguishes a growable array created
// from a [...] literal (fixed = false, push() works) from a fixed-size
// array created with AllocatedArray(n) (fixed = true, push() is rejected,
// and indexing can never go past the size it was allocated with).
struct ArrayObject {
    std::vector<Value> elements;
    bool fixed = false;
};

// An instance constructed from a `set type`/`set interface` declaration,
// e.g. `{ x: 5 }<T>`. typeName is what typeof() returns for it. Fields are
// stored in declaration order (not a hash map) so printing/iterating is
// deterministic and matches how the type was written.
struct TypedObject {
    std::string typeName;
    std::vector<std::pair<std::string, Value>> fields;

    Value* find(const std::string& name) {
        for (auto& f : fields) {
            if (f.first == name) return &f.second;
        }
        return nullptr;
    }
};

bool isTruthy(const Value& value);
std::string valueToDisplayString(const Value& value);
