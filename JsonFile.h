#pragma once

#include <QSet>
#include <QFile>
#include <QString>

#include <rapidjson/document.h>
#include <string_view>
#include <optional>
#include <set>

class JsonFile
{
public:
    using StringView = std::basic_string_view<char>;

    struct LineInfo
    {
        size_t index;
        size_t size;
        StringView text;
        const rapidjson::Document& doc;
        bool keysUpdated;
    };

    struct Line
    {
        size_t index;
        StringView range;
        std::optional<rapidjson::Document> value;
    };

    const std::vector<QString>& topLevelKeys() const { return discoveredKeys; }

    JsonFile();
    ~JsonFile();

    JsonFile(const JsonFile&) = delete;
    JsonFile& operator=(const JsonFile&) = delete;
    JsonFile(JsonFile&&) = delete;
    JsonFile& operator=(JsonFile&&) = delete;

    bool open(const QString& filename);
    void close();


    size_t size() const
    {
        return lines.size();
    }

    LineInfo line(size_t index);
    StringView lineText(size_t index) const
    {
        if (index >= lines.size())
            return StringView();

        return lines[index].range;
    }

private:
    QFile file;

    StringView dataView;
    uchar * mappedData = nullptr;
    std::vector<Line> lines;

    // Lazily discovered keys
    std::vector<QString> discoveredKeys;
    QSet<QString> keySet;

    // static std::vector<Line> parseSequentialJson(StringView data);
};
