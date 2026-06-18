#pragma once
#include <string>
#include <vector>
#include <memory>
#include <variant>

// Forward-declared so Value can hold a pointer to it before it's fully defined.
struct ArrayObject;

// Value can be a number, string, bool, null, or a reference-counted array.
// The array alternative uses shared_ptr so arrays have reference semantics
// (mutating an array affects every variable referencing it, matching how
// most languages treat array/list assignment).
//
// Both the tree-walking Interpreter and the bytecode VM operate on this
// same Value type, so behavior (string concatenation, truthiness, display
// formatting) stays identical no matter which one is running a program.
struct Value {
    std::variant<long long, std::string, bool, std::shared_ptr<ArrayObject>, std::monostate> data;

    Value() : data(std::monostate{}) {}              // default value = null
    Value(long long v) : data(v) {}
    Value(int v) : data((long long)v) {}
    Value(std::string v) : data(std::move(v)) {}
    Value(bool v) : data(v) {}
    Value(std::shared_ptr<ArrayObject> v) : data(std::move(v)) {}
};

// The actual array storage. `fixed` distinguishes a growable array created
// from a [...] literal (fixed = false, push() works) from a fixed-size
// array created with AllocatedArray(n) (fixed = true, push() is rejected,
// and indexing can never go past the size it was allocated with).
struct ArrayObject {
    std::vector<Value> elements;
    bool fixed = false;
};

bool isTruthy(const Value& value);
std::string valueToDisplayString(const Value& value);
