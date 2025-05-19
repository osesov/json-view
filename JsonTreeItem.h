#pragma once

#include "constants.h"
#include "json.h"
#include "Locale.h"

#include <rapidjson/document.h>

#include <QLocale>
#include <QVariant>
#include <QString>
#include <vector>

class JsonTreeItem {
public:
    JsonTreeItem(const rapidjson::Value* value, QString key = {}, JsonTreeItem* parent = nullptr);
    ~JsonTreeItem();

    void ensureChildren();

    JsonTreeItem* child(int row);
    int childCount();
    int row() const;

    QVariant data(int column) const;

    JsonTreeItem* parent() const;

private:
    const rapidjson::Value* m_value;
    QString m_key;
    JsonTreeItem* m_parent;
    std::vector<JsonTreeItem*> m_children;
};
