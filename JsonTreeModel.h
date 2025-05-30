#pragma once

#include <QAbstractItemModel>
#include <QTreeView>

#include "JsonTreeItem.h"

class JsonTreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    JsonTreeModel(const rapidjson::Value* rootValue, QObject* parent = nullptr);
    ~JsonTreeModel() override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex&) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const;

    void search(bool restartSearch, bool forward, const QString& query, QTreeView* tableView);
    void cancelSearch();

private:
    JsonTreeItem* m_root;
};
