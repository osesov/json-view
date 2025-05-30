#pragma once

#include "constants.h"
#include "json.h"
#include "Locale.h"

#include <rapidjson/document.h>

#include <QModelIndex>
#include <QLocale>
#include <QVariant>
#include <QString>
#include <vector>

class JsonTreeItem {
public:
    JsonTreeItem(const rapidjson::Value* value, QString key);
    JsonTreeItem(const rapidjson::Value* value, QString key, JsonTreeItem* parent, size_t index, bool lineExtension);
    ~JsonTreeItem();

    void ensureChildren();

    JsonTreeItem* parent() const;
    JsonTreeItem* child(int row);
    int childCount();
    int row() const;

    QVariant data(int column) const;

    bool isMultiline() const { return m_isMultiline && m_lineExtension; }

    static JsonTreeItem * fromIndex(const QModelIndex& index) {
        return static_cast<JsonTreeItem*>(index.internalPointer());
    }

    bool match(const QString& query) const;

private:
    const rapidjson::Value* m_value;
    QString m_key;
    JsonTreeItem* m_parent;
    size_t m_index;
    bool m_isMultiline;
    bool m_lineExtension;
    std::vector<JsonTreeItem*> m_children;
    size_t m_byteSize;
};
