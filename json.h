#pragma once
#include <rapidjson/document.h>
#include <string>
#include <string_view>
#include <optional>
#include <functional>

struct Range
{
    const char* start;
    const char* end;
};

std::string toJsonString(const rapidjson::Value& value);
std::string toJsonString(const rapidjson::Value& value, size_t limit);
std::optional<Range> matchJsonValue(const char* input, size_t length);

void parseSequentialJson(std::string_view data, std::function<void(size_t, std::string_view)> consumer);
