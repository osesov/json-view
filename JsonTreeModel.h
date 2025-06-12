#pragma once

#include <QAbstractItemModel>
#include <QTreeView>
#include <QStatusBar>

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

    void reload();
    void search(bool forward, const QString& query, QTreeView* tableView, QStatusBar*);
    void cancelSearch();

private:
    JsonTreeItem* m_root;
    std::optional<QModelIndex> m_currentSearchIndex;

    void revealMatchInTree(QTreeView* treeView, const QModelIndex& index);
    void getPath(QModelIndex index, QStringList& path) const;

};
