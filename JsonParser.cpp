#include "jsonParser.h"

#include <cctype>
#include <cstring>
#include <stdexcept>

using namespace std::literals;

// support some JSON5: comments, commas after last element,
// no unquoted keys through and other feature. See https://json5.org/

namespace
{
    std::optional<json::Value> parseValue(std::string_view& json);

    void skipWhitespace(std::string_view& json)
    {
        size_t pos = 0;

        while (pos < json.size()) {

            if (std::isspace(static_cast<unsigned char>(json[pos])))
            {
                ++pos; // Skip whitespace characters
                continue;
            }

            if (std::iscntrl(static_cast<unsigned char>(json[pos])))
            {
                // Skip control characters
                ++pos;
                continue;
            }

            if (pos + 2 < json.size() && json[pos] == '/' && json[pos + 1] == '/') {
                // Skip single-line comment
                pos += 2;
                while (pos < json.size() && json[pos] != '\n') {
                    ++pos;
                }
            } else if (pos + 1 < json.size() && json[pos] == '/' && json[pos + 1] == '*') {
                // Skip multi-line comment
                pos += 2;
                while (pos + 1 < json.size() && !(json[pos] == '*' && json[pos + 1] == '/')) {
                    ++pos;
                }
                if (pos + 1 < json.size()) {
                    pos += 2; // Skip closing */
                }
            }
        }
        json.remove_prefix(pos);
    }

    unsigned char peekChar(std::string_view& json)
    {
        while (true) {
            skipWhitespace(json);
            if (json.empty()) {
                return '\0'; // No more characters to peek
            }
            return static_cast<unsigned char>(json.front());
        }
    }

    unsigned char oneOf(std::string_view& json, const char* chars)
    {
        char c = peekChar(json);
        if (std::strchr(chars, c) != nullptr) { // Check if the character is in the set
            json.remove_prefix(1); // Remove the character from the string
            return static_cast<unsigned char>(c);
        }

        return '\0'; // Character not found in the set
    }

    std::string_view parseString(std::string_view& json)
    {
        if (peekChar(json) != '"') {
            return {};
        }

        size_t pos = 1; // Skip the opening quote
        while (pos < json.size() && json[pos] != '"') {
            if (json[pos] == '\\' && pos + 1 < json.size()) {
                pos += 2; // Skip escaped character
            } else {
                ++pos;
            }
        }

        if (pos >= json.size() || json[pos] != '"') {
            return {}; // Missing closing quote
        }

        std::string_view result = json.substr(0, pos);
        json.remove_prefix(pos + 1); // Move past the closing quote
        return result;
    }

    std::string_view parseNumber(std::string_view& json)
    {
        size_t pos = 0;
        while (pos < json.size() && (std::isdigit(static_cast<unsigned char>(json[pos])) || json[pos] == '.' || json[pos] == '-' || json[pos] == '+')) {
            ++pos;
        }

        if (pos == 0) {
            return {}; // No valid number found
        }

        std::string_view result = json.substr(0, pos);
        json.remove_prefix(pos);
        return result;
    }

    std::optional<json::Object> parseObject(std::string_view& json)
    {

        json::Object object;

        while (true) {
            char c = oneOf(json, "\"},");

            switch (c) {
                case ',':
                    continue;
                case '}':
                    return object; // Return the parsed object
                case '"':
                    break; // Continue to parse key-value pairs
                default:
                    return std::nullopt; // Expected a string key or closing brace
            }

            auto key = parseString(json); // Parse key
            if (key.empty()) {
                return std::nullopt; // Error in parsing key
            }

            if (!oneOf(json, ":")) {
                return std::nullopt; // Expected a colon after key
            }
            auto value = parseValue(json); // Parse value
            if (!value) {
                return std::nullopt; // Error in parsing value
            }
            object.emplace(key, *value); // Add key-value pair to the object

            switch(oneOf(json, ",}")) {
                default:
                    return std::nullopt; // No more data to parse
                case ',':
                    continue;
                case '}':
                    return object; // Return the parsed object
            }
            return std::nullopt; // Expected a comma or closing brace
        }
    }

    std::optional<json::Array> parseArray(std::string_view& json)
    {
        json::Array array;

        while (true) {
            char c = oneOf(json, ",]");

            switch (c) {
                case ']':
                    return array; // Return the parsed array
                case ',':
                    continue; // Continue to parse elements
                default:
                    return std::nullopt; // Expected a comma or closing bracket
            }

            auto value = parseValue(json); // Parse value
            if (!value) {
                return std::nullopt; // Error in parsing value
            }
            array.push_back(*value); // Add value to the array

            if (oneOf(json, "]")) {
                return array; // Return the parsed array
            }
        }
    }

    std::optional<json::Value> parsePrimitive(std::string_view& json, const std::string_view primitive)
    {
        if (json.substr(0, primitive.size()) == primitive) {
            auto value = json.substr(0, primitive.size());
            json.remove_prefix(primitive.size());
            return value;
        }
        return std::nullopt;
    }

    std::optional<json::Value> parseValue(std::string_view& json)
    {
        char c = oneOf(json, "\"{[tfn0123456789-+");
        switch(c) {
            case '"':
                return json::Value(parseString(json));
            case '{': {
                auto object = parseObject(json);
                if (object) {
                    return json::Value(*object);
                }
                break;
            }
            case '[': {
                auto array = parseArray(json);
                if (array) {
                    return json::Value(*array);
                }
                break;
            }
            case 't': // true
                return parsePrimitive(json, "true"sv);

            case 'f': // false
                return parsePrimitive(json, "false"sv);

            case 'n': // null
                return parsePrimitive(json, "null"sv);

            default:
                if (std::isdigit(static_cast<unsigned char>(c)) || c == '-' || c == '+') {
                    std::string_view numValue = parseNumber(json);
                    if (!numValue.empty()) {
                        return json::Value(numValue);
                    }
                }
        }

        return std::nullopt; // No valid JSON value found
    }
}

std::optional<json::Value> json::Parser::parseNext()
{
    skipWhitespace(json);
    if (json.empty()) {
        return std::nullopt; // No more JSON to parse
    }

    auto value = parseValue(json);
    if (!value) {
        return std::nullopt; // Error in parsing
    }

    return value; // Return the parsed value
}

bool json::isPrimitive(const Value& value)
{
    return std::holds_alternative<Primitive>(value);
}

bool json::isArray(const Value& value)
{
    return std::holds_alternative<Array>(value);
}

bool json::isObject(const Value& value)
{
    return std::holds_alternative<Object>(value);
}

bool json::isString(const Value& value)
{
    if (!std::holds_alternative<Primitive>(value))
        return false;

    auto &primitive = std::get<Primitive>(value);
    if (primitive.empty())
        return false;
    return primitive.front() == '"' && primitive.back() == '"';
}

bool json::isNumber(const Value& value)
{
    if (!std::holds_alternative<Primitive>(value))
        return false;

    auto &primitive = std::get<Primitive>(value);
    if (primitive.empty())
        return false;

    auto c = static_cast<unsigned char>(primitive.front());

    // Check if the primitive starts with a digit or a sign
    if (!std::isdigit(c) && c != '-' && c != '+') {
        return false;
    }

    return true;
}

bool json::isBoolean(const Value& value)
{
    if (!std::holds_alternative<Primitive>(value))
        return false;

    auto &primitive = std::get<Primitive>(value);
    if (primitive == "true"sv || primitive == "false"sv) {
        return true;
    }
    return false;
}

bool json::isNull(const Value& value)
{
    if (!std::holds_alternative<Primitive>(value))
        return false;

    auto &primitive = std::get<Primitive>(value);
    return primitive == "null"sv;
}

std::string_view json::getString(const Value& value)
{
    if (!std::holds_alternative<Primitive>(value)) {
        throw std::runtime_error("Value is not a string");
    }

    auto& primitive = std::get<Primitive>(value);
    if (primitive.empty() || primitive.front() != '"' || primitive.back() != '"') {
        throw std::runtime_error("Value is not a valid string");
    }

    // Remove the surrounding quotes
    return primitive.substr(1, primitive.size() - 2);
}

double json::getNumber(const Value& value)
{
    if (!std::holds_alternative<Primitive>(value)) {
        throw std::runtime_error("Value is not a number");
    }

    auto& primitive = std::get<Primitive>(value);
    if (primitive.empty() || (primitive.front() != '-' && primitive.front() != '+' && !std::isdigit(static_cast<unsigned char>(primitive.front())))) {
        throw std::runtime_error("Value is not a valid number");
    }

    try {
        return std::stod(std::string(primitive));
    } catch (const std::invalid_argument&) {
        throw std::runtime_error("Value is not a valid number");
    } catch (const std::out_of_range&) {
        throw std::runtime_error("Number out of range");
    }
}

bool json::getBoolean(const Value& value)
{
    if (!std::holds_alternative<Primitive>(value)) {
        throw std::runtime_error("Value is not a boolean");
    }

    auto& primitive = std::get<Primitive>(value);
    if (primitive == "true"sv) {
        return true;
    } else if (primitive == "false"sv) {
        return false;
    } else {
        throw std::runtime_error("Value is not a valid boolean");
    }
}

void json::getNull(const Value& value)
{
    if (!std::holds_alternative<Primitive>(value)) {
        throw std::runtime_error("Value is not null");
    }

    auto& primitive = std::get<Primitive>(value);
    if (primitive != "null"sv) {
        throw std::runtime_error("Value is not a valid null");
    }
}

json::ValueType json::getValueType(const json::Value& value)
{
    if (std::holds_alternative<Primitive>(value)) {
        auto& primitive = std::get<Primitive>(value);
        if (primitive == "true"sv)
            return ValueType::trueValue;

        if (primitive == "false"sv)
            return ValueType::trueValue;

        if (primitive == "null"sv)
            return ValueType::nullValue;

        if (primitive.front() == '"' && primitive.back() == '"')
            return ValueType::stringValue;

        return ValueType::numberValue;

    } else if (std::holds_alternative<Array>(value)) {
        return ValueType::arrayValue;
    } else if (std::holds_alternative<Object>(value)) {
        return ValueType::objectValue;
    }
    throw std::runtime_error("Unknown value type");
}
