#include "JsonFile.h"

#include "json.h"

#include <rapidjson/reader.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <iostream>
#include <vector>
#include <string_view>
#include <cctype>
#include <optional>

JsonFile::JsonFile()
{
}

JsonFile::~JsonFile()
{
    close();
}

bool JsonFile::open(const QString& filename)
{
    file.setFileName(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    auto size = file.size();
    uchar *data = file.map(0, file.size());

    if (!data) {
        file.close();
        return false;
    }
    this->dataView = StringView(reinterpret_cast<const char*>(data), size);
    this->mappedData = data;

    lines.clear();
    lines.reserve(1000);

    parseSequentialJson(this->dataView, [&](size_t index, StringView range) {
        lines.push_back(Line{
            .index = index,
            .range = range,
            .value = std::nullopt
        });
    });

    // preload first lines
    for (int i = 0; i < 10; ++i) {
        if (i == this->lines.size())
            break;
        line(i);
    }

    return true;
}

void JsonFile::close()
{
    if (file.isOpen()) {
        file.unmap(mappedData);
        file.close();
    }
}



JsonFile::LineInfo JsonFile::line(size_t index)
{
    auto& line = this->lines[index];
    if (line.value) {
        return LineInfo{
            .index = index,
            .size = line.range.size(),
            .text = line.range,
            .doc = *line.value,
            .keysUpdated = false
        };
    }

    rapidjson::Document doc;
    doc.Parse(line.range.data(), line.range.size());
    bool keysUpdated = false;

    // update names
    if (doc.IsObject()) {
        for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it) {
            QString key = QString::fromUtf8(it->name.GetString(), it->name.GetStringLength());
            if (!keySet.contains(key)) {
                keySet.insert(key);
                discoveredKeys.push_back(key);
                keysUpdated = true;
            }
        }
    }

    line.value = std::move(doc);
    return LineInfo{
        .index = index,
        .size = line.range.size(),
        .text = line.range,
        .doc = *line.value,
        .keysUpdated = keysUpdated
    };
}
