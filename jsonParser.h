#pragma once

#include <string_view>
#include <stdint.h>
#include <variant>
#include <list>
#include <map>
#include <optional>

// non-copying json parser
namespace json
{
    struct Value;

    enum class ValueType
    {
        arrayValue,
        objectValue,
        stringValue,
        numberValue,
        trueValue,
        falseValue,
        nullValue
    };

    typedef std::string_view Primitive;
    typedef std::list<Value> Array;
    typedef std::map<std::string_view, Value> Object;

    struct Value : public std::variant<Primitive, Array, Object> {
        using std::variant<Primitive, Array, Object>::variant;
    };

    class Parser
    {

    public:
        Parser(std::string_view json)
            : json(json)
        {
        }

        Parser(const Parser&) = delete;
        Parser& operator=(const Parser&) = delete;
        Parser(Parser&&) = delete;
        Parser& operator=(Parser&&) = delete;
        ~Parser() = default;

        std::optional<Value> parseNext();

    private:
        std::string_view json;
    };

    bool isPrimitive(const Value& value);
    bool isArray(const Value& value);
    bool isObject(const Value& value);

    bool isString(const Value& value);
    bool isNumber(const Value& value);
    bool isBoolean(const Value& value);
    bool isNull(const Value& value);

    std::string_view getString(const Value& value);
    double getNumber(const Value& value);
    bool getBoolean(const Value& value);
    void getNull(const Value& value);

    ValueType getValueType(const Value&);
}
