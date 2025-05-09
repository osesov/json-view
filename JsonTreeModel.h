#pragma once

#include <QAbstractItemModel>

#include "JsonTreeItem.h"

class JsonTreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    JsonTreeModel(const rapidjson::Value* rootValue, QObject* parent = nullptr)
        : QAbstractItemModel(parent), m_root(new JsonTreeItem(rootValue, "root")) {}

    ~JsonTreeModel() override { delete m_root; }

    QModelIndex index(int row, int column, const QModelIndex& parent) const override {
        JsonTreeItem* parentItem = parent.isValid() ? static_cast<JsonTreeItem*>(parent.internalPointer()) : m_root;
        JsonTreeItem* childItem = parentItem->child(row);
        return childItem ? createIndex(row, column, childItem) : QModelIndex();
    }

    QModelIndex parent(const QModelIndex& index) const override {
        if (!index.isValid()) return QModelIndex();
        JsonTreeItem* childItem = static_cast<JsonTreeItem*>(index.internalPointer());
        JsonTreeItem* parentItem = childItem->parent();
        if (parentItem == m_root || !parentItem) return QModelIndex();
        return createIndex(parentItem->row(), 0, parentItem);
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        JsonTreeItem* parentItem = parent.isValid() ? static_cast<JsonTreeItem*>(parent.internalPointer()) : m_root;
        return parentItem->childCount();
    }

    int columnCount(const QModelIndex&) const override { return TreeViewColumn::MaxColumn; }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
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

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (role != Qt::DisplayRole || orientation != Qt::Horizontal) return {};
        switch (section) {
            case TreeViewColumn::KeyColumn: return "Key";
            case TreeViewColumn::SizeColumn: return "Size";
            case TreeViewColumn::BytesColumn: return "Bytes";
            case TreeViewColumn::ValueColumn: return "Value";
        }
        return section == 0 ? "Key" : "Value";
    }

private:
    JsonTreeItem* m_root;
};
