#pragma once

#include <cstddef>

enum TreeViewColumn : int
{
    KeyColumn = 0,
    SizeColumn,
    BytesColumn,
    ValueColumn,
    MaxColumn,
};

const std::size_t MAX_JSON_STRING_LENGTH = 1024; // 1 KB
