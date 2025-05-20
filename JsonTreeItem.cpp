#include "JsonTreeItem.h"

#include "constants.h"

JsonTreeItem::JsonTreeItem(const rapidjson::Value* value, QString key, JsonTreeItem* parent, size_t index)
    : m_value(value), m_key(std::move(key)), m_parent(parent), m_index(index)
{
    if (value && value->IsString()) {
        const char* str = value->GetString();
        size_t len = value->GetStringLength();
        m_isMultiline = !!memchr(str, '\n', len);
        // printf("isMultiline: %d, %s\n", m_isMultiline, key.toUtf8().constData());
    }
    else {
        m_isMultiline = false;
    }
}

JsonTreeItem::~JsonTreeItem()
{
    qDeleteAll(m_children);
}

void JsonTreeItem::ensureChildren()
{
    if (!m_children.empty() || !m_value->IsArray() && !m_value->IsObject())
        return;

    if (m_value->IsObject()) {
        size_t index = 0;
        for (auto it = m_value->MemberBegin(); it != m_value->MemberEnd(); ++it) {
            QString key = QString::fromUtf8(it->name.GetString(), it->name.GetStringLength());
            m_children.push_back(new JsonTreeItem(&it->value, key, this, index++));
        }
    } else if (m_value->IsArray()) {
        size_t index = 0;
        m_children.reserve(m_value->Size());
        for (rapidjson::SizeType i = 0; i < m_value->Size(); ++i) {
            m_children.push_back(new JsonTreeItem(&(*m_value)[i], QString("[%1]").arg(i), this, index++));
        }
    }
}

JsonTreeItem* JsonTreeItem::child(int row)
{
    ensureChildren();
    return (row >= 0 && row < m_children.size()) ? m_children[row] : nullptr;
}

int JsonTreeItem::childCount()
{
    ensureChildren();
    return static_cast<int>(m_children.size());
}

int JsonTreeItem::row() const
{
    return m_index;
    // return m_parent ? std::find(m_parent->m_children.begin(), m_parent->m_children.end(), this) - m_parent->m_children.begin() : 0;
}

QVariant JsonTreeItem::data(int column) const
{

    if (column == TreeViewColumn::KeyColumn)
        return m_key;

    if (!m_value)
        return QVariant();

    if (column == TreeViewColumn::SizeColumn) {
        if (m_value->IsObject())
            return locale.toString(m_value->MemberCount());
        if (m_value->IsArray())
            return locale.toString(m_value->Size());

        return 0;
    }

    if (column == TreeViewColumn::BytesColumn) {
        return locale.toString(qint64(toJsonString(*m_value).size()));
        // return m_value ? m_value->GetStringLength() : 0;
    }

    if (column == TreeViewColumn::ValueColumn) {
        if (m_value->IsString()) return QString::fromUtf8(m_value->GetString());
        if (m_value->IsBool()) return m_value->GetBool() ? "true" : "false";
        if (m_value->IsInt64()) return locale.toString(m_value->GetInt64()); // qint64(m_value->GetInt64());
        if (m_value->IsUint64()) return locale.toString(m_value->GetUint64()); // qint64(m_value->GetInt64());
        if (m_value->IsDouble()) return locale.toString(m_value->GetDouble()); // m_value->GetDouble();
        if (m_value->IsNull()) return "null";
        if (m_value->IsArray()) return QString::fromUtf8(toJsonString(*m_value, MAX_JSON_STRING_LENGTH));
        if (m_value->IsObject()) return QString::fromUtf8(toJsonString(*m_value, MAX_JSON_STRING_LENGTH));
    }

    return {};
}

JsonTreeItem* JsonTreeItem::parent() const
{
    return m_parent;
}
