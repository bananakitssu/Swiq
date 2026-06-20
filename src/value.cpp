#include "value.h"
#include <sstream>

bool isTruthy(const Value& value) {
    if (auto b = std::get_if<bool>(&value.data)) return *b;
    if (auto i = std::get_if<long long>(&value.data)) return *i != 0;
    if (auto d = std::get_if<double>(&value.data)) return *d != 0.0;
    if (auto s = std::get_if<std::string>(&value.data)) return !s->empty();
    if (auto a = std::get_if<std::shared_ptr<ArrayObject>>(&value.data)) return !(*a)->elements.empty();
    if (std::get_if<std::shared_ptr<TypedObject>>(&value.data)) return true; // objects are always truthy
    return false; // null/none/nil is always falsy
}

// Formats a double without ugly trailing zeros (avoids std::to_string's
// fixed 6-decimal-place output), but always keeps at least one decimal
// place so a float never prints identically to an integer.
static std::string formatDouble(double d) {
    std::ostringstream oss;
    oss << d;
    std::string s = oss.str();
    if (s.find('.') == std::string::npos &&
        s.find('e') == std::string::npos &&
        s.find("inf") == std::string::npos &&
        s.find("nan") == std::string::npos) {
        s += ".0";
    }
    return s;
}

std::string valueToDisplayString(const Value& value) {
    if (auto i = std::get_if<long long>(&value.data)) {
        return std::to_string(*i);
    }
    if (auto d = std::get_if<double>(&value.data)) {
        return formatDouble(*d);
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
    if (auto o = std::get_if<std::shared_ptr<TypedObject>>(&value.data)) {
        std::string result = "{ ";
        bool first = true;
        for (const auto& [name, val] : (*o)->fields) {
            if (!first) result += ", ";
            result += name + ": " + valueToDisplayString(val);
            first = false;
        }
        result += " }";
        return result;
    }
    if (std::holds_alternative<std::monostate>(value.data)) {
        return "null";
    }
    return "";
}
