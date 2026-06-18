#include "value.h"

bool isTruthy(const Value& value) {
    if (auto b = std::get_if<bool>(&value.data)) return *b;
    if (auto i = std::get_if<long long>(&value.data)) return *i != 0;
    if (auto s = std::get_if<std::string>(&value.data)) return !s->empty();
    if (auto a = std::get_if<std::shared_ptr<ArrayObject>>(&value.data)) return !(*a)->elements.empty();
    return false; // null/none/nil is always falsy
}

std::string valueToDisplayString(const Value& value) {
    if (auto i = std::get_if<long long>(&value.data)) {
        return std::to_string(*i);
    }
    if (auto s = std::get_if<std::string>(&value.data)) {
        return *s;
    }
    if (auto b = std::get_if<bool>(&value.data)) {
        return *b ? "true" : "false";
    }
    if (auto a = std::get_if<std::shared_ptr<ArrayObject>>(&value.data)) {
        std::string result = "[";
        const auto& elements = (*a)->elements;
        for (size_t i = 0; i < elements.size(); i++) {
            if (i > 0) result += ", ";
            result += valueToDisplayString(elements[i]);
        }
        result += "]";
        return result;
    }
    if (std::holds_alternative<std::monostate>(value.data)) {
        return "null";
    }
    return "";
}
