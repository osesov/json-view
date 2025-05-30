#include "JsonTreeModel.h"

#include <QTreeView>

JsonTreeModel::JsonTreeModel(const rapidjson::Value* rootValue, QObject* parent)
    : QAbstractItemModel(parent), m_root(new JsonTreeItem(rootValue, "root"))
{

}

JsonTreeModel::~JsonTreeModel()
{
    delete m_root;
}

QModelIndex JsonTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    JsonTreeItem* parentItem = parent.isValid() ? static_cast<JsonTreeItem*>(parent.internalPointer()) : m_root;
    JsonTreeItem* childItem = parentItem->child(row);
    return childItem ? createIndex(row, column, childItem) : QModelIndex();
}

QModelIndex JsonTreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid()) return QModelIndex();
    JsonTreeItem* childItem = static_cast<JsonTreeItem*>(index.internalPointer());
    JsonTreeItem* parentItem = childItem->parent();
    if (parentItem == m_root || !parentItem) return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}

int JsonTreeModel::rowCount(const QModelIndex& parent) const
{
    JsonTreeItem* parentItem = parent.isValid() ? static_cast<JsonTreeItem*>(parent.internalPointer()) : m_root;
    return parentItem->childCount();
}

int JsonTreeModel::columnCount(const QModelIndex&) const
{
    return TreeViewColumn::MaxColumn;
}

QVariant JsonTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return {};

    if (role == Qt::DisplayRole) {
        auto* item = static_cast<JsonTreeItem*>(index.internalPointer());
        return item->data(index.column());
    }

    if (role == Qt::TextAlignmentRole) {
        return static_cast<int>(Qt::AlignTop | Qt::AlignLeft);
    }

    return {};
}

QVariant JsonTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) return {};
    switch (section) {
        case TreeViewColumn::KeyColumn: return "Key";
        case TreeViewColumn::SizeColumn: return "Size";
        case TreeViewColumn::BytesColumn: return "Bytes";
        case TreeViewColumn::ValueColumn: return "Value";
    }
    return section == 0 ? "Key" : "Value";
}

Qt::ItemFlags JsonTreeModel::flags(const QModelIndex& index) const {
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    JsonTreeItem* item = JsonTreeItem::fromIndex(index);

    // if (item->isMultiline()) {
    //     return defaultFlags | Qt::ItemIsEditable;
    // }
    return defaultFlags;
}

void JsonTreeModel::search(bool restartSearch, bool forward, const QString& query, QTreeView* tableView)
{

}

void JsonTreeModel::cancelSearch()
{

}
