#include "json.h"

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

class TruncatingStream {
public:
    typedef char Ch; // Character type

    TruncatingStream(size_t limit) : _limit(limit), _buffer()
    {
        _buffer.reserve(limit);
    }

    // void PutUnsafe(char c) {
    //     if (_buffer.size() < _limit)
    //         _buffer.push_back(c);
    // }

    void PutUnsafe(char c)
    {
        if (_buffer.size() < _limit)
            _buffer.push_back(c);
    }

    void Put(char c)
    {
        if (_buffer.size() < _limit)
            _buffer.push_back(c);
    }

    void Flush() {} // No-op

    const std::string& str() const { return _buffer; }

private:
    size_t _limit;
    std::string _buffer;
};

class CountingStream
{
public:
    typedef char Ch; // Character type

    CountingStream() : _size()
    {
    }

    void PutUnsafe(char c)
    {
        _size++;
    }

    void Put(char c)
    {
        _size++;
    }

    void Flush() {} // No-op

    size_t size() const { return _size; }

private:
    size_t _size;
};

size_t toJsonStringSize(const rapidjson::Value& value, size_t limit) {

    CountingStream truncatingStream;
    rapidjson::Writer<CountingStream> writer(truncatingStream);
    value.Accept(writer);
    return truncatingStream.size();
}

std::string toJsonString(const rapidjson::Value& value, size_t limit) {

    TruncatingStream truncatingStream(limit);
    rapidjson::Writer<TruncatingStream> writer(truncatingStream);
    value.Accept(writer);
    return truncatingStream.str();
}

std::string toJsonString(const rapidjson::Value& value) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    buffer.Reserve(4096);
    value.Accept(writer);
    return std::string(buffer.GetString(), buffer.GetSize());
}

std::optional<Range> matchJsonValue(const char* input, size_t length) {
    const char* p = input;
    const char* end = input + length;

    // Skip leading whitespace
    while (p < end && std::isspace(static_cast<unsigned char>(*p))) ++p;

    if (p >= end)
        return std::nullopt;

    switch (*p) {
        case '{': case '[': {
            // Likely object or array – use bracket matching
            char open = *p;
            char close = (open == '{') ? '}' : ']';
            int depth = 0;
            const char* start = p;

            while (p < end) {
                if (*p == open) depth++;
                else if (*p == close) {
                    depth--;
                    if (depth == 0) {
                        return Range{start, p + 1};
                        // outConsumed = (p - input) + 1;
                        // return true;
                    }
                } else if (*p == '"') {
                    // Skip string content
                    ++p;
                    while (p < end && *p != '"') {
                        if (*p == '\\' && p + 1 < end) ++p;
                        ++p;
                    }
                }
                ++p;
            }
            return std::nullopt;
        }
        case '"': {
            ++p;
            while (p < end && *p != '"') {
                if (*p == '\\' && p + 1 < end) ++p;
                ++p;
            }
            if (p < end && *p == '"') {
                return Range{input, p + 1};
                // outConsumed = (p - input) + 1;
                // return true;
            }
            return std::nullopt;
        }
        case 't': return (end - p >= 4 && std::strncmp(p, "true", 4) == 0) ? std::make_optional<Range>({p, p+4}) : std::nullopt;
        case 'f': return (end - p >= 5 && std::strncmp(p, "false", 5) == 0) ? std::make_optional<Range>({p, p+5}) : std::nullopt;
        case 'n': return (end - p >= 4 && std::strncmp(p, "null", 4) == 0) ? std::make_optional<Range>({p, p+4}) : std::nullopt;
        default: {
            // Likely number: match using a regex-style FSM
            const char* start = p;
            if (*p == '-' || std::isdigit(*p)) {
                ++p;
                while (p < end && (std::isdigit(*p) || *p == '.' || *p == 'e' || *p == 'E' || *p == '+' || *p == '-')) {
                    ++p;
                }
                return Range{start, p};
            }
        }
    }

    return std::nullopt;
}

// std::vector<JsonFile::StringView> parseSequentialJson(StringView data) {
//     std::vector<StringView> results;
//     rapidjson::MemoryStream ms(reinterpret_cast<const char*>(data.data()), data.size());

//     while (true) {
//         rapidjson::Document doc;

//         while (ms.Tell() < ms.size_ && isspace(static_cast<unsigned char>(ms.Peek()))) {
//             ms.Take();
//         }

//         if (ms.Tell() == ms.size_ ) {
//             break; // No more data to process
//         }

//         size_t offset = ms.Tell();
//         rapidjson::ParseResult result = doc.ParseStream<rapidjson::kParseStopWhenDoneFlag | rapidjson::kParseCommentsFlag /*| rapidjson::kParseTrailingCommasFlag*/>(ms);

//         if (!result) {
//             std::cerr << "Parse error: " << rapidjson::GetParseError_En(result.Code())
//                       << " at offset " << result.Offset() << std::endl;
//             break;
//         }

//         size_t consumed = ms.Tell();
//         if (consumed == 0) {
//             break; // No more valid JSON found
//         }
//         results.emplace_back(data.data() + offset, consumed);
//     }

//     return results;
// }

void parseSequentialJson(std::string_view data, std::function<void(size_t, std::string_view)> consumer)
{
    // std::vector<Line> results;
    const char* p = data.data();
    const char* end = p + data.size();
    size_t index = 0;

    while (p < end) {
        size_t length = end - p;
        auto range = matchJsonValue(p, length);
        if (!range) {
            break; // No more valid JSON found
        }

        consumer(index, std::string_view(range->start, range->end - range->start));
        // Line line {index, std::string_view(range->start, range->end - range->start), std::nullopt};
        // results.emplace_back(std::move(line));
        p = range->end;
        index++;
    }

    // return results;
}
