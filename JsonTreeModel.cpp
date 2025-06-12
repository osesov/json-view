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

void JsonTreeModel::reload()
{
    beginResetModel();
    delete m_root;
    m_root = new JsonTreeItem(nullptr, "root");
    endResetModel();
}

QModelIndex JsonTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    JsonTreeItem* parentItem = parent.isValid() ? JsonTreeItem::fromIndex(parent) : m_root;
    JsonTreeItem* childItem = parentItem->child(row);
    return childItem ? createIndex(row, column, childItem) : QModelIndex();
}

QModelIndex JsonTreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid()) return QModelIndex();
    JsonTreeItem* childItem = JsonTreeItem::fromIndex(index);
    JsonTreeItem* parentItem = childItem->parent();
    if (parentItem == m_root || !parentItem) return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}

int JsonTreeModel::rowCount(const QModelIndex& parent) const
{
    JsonTreeItem* parentItem = parent.isValid() ? JsonTreeItem::fromIndex(parent) : m_root;
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
        auto* item = JsonTreeItem::fromIndex(index);
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

void JsonTreeModel::getPath(QModelIndex index, QStringList& path) const
{
    if (!index.isValid()) return;

    // Get the parent index
    while (index.isValid()) {
        path.prepend(JsonTreeItem::fromIndex(index)->data(TreeViewColumn::KeyColumn).toString());
        index = index.parent();
    }
}

void JsonTreeModel::search(bool forward, const QString& query, QTreeView* tableView, QStatusBar * statusBar)
{
    QModelIndex index = tableView->currentIndex();
    bool skipCurrent = false;

    if (!index.isValid()) {// nothing is selected, use root
        index = this->index(0, 0, QModelIndex());
    }
    else if (m_currentSearchIndex && *m_currentSearchIndex == index) { // the same index is selected
        skipCurrent = true;  // Skip current index if it's already the search index
    }
    else {
        // something else is selected, use it as the starting point
        skipCurrent = true;
    }

    while (index.isValid()) {
        // Process current node
        // ...

        if (skipCurrent) {
            skipCurrent = false;
        }
        else {
            auto * item = JsonTreeItem::fromIndex(index);
            if (item->match(query)) {
                // If a match is found, reveal it in the tree view
                revealMatchInTree(tableView, index);
                m_currentSearchIndex = index;  // Store the current search index

                QStringList path;
                getPath(index, path);

                statusBar->showMessage(tr("Found at '%1'").arg(path.join(" > ")));

                return;  // Exit after finding the first match
            }
        }

        // advance to the next node
        // Prefer child if exists
        if (this->rowCount(index) > 0) {
            if (forward) {
                index = this->index(0, 0, index);  // go to first child
            }
            else {
                index = this->index(this->rowCount(index) - 1, 0, index);  // go to last child
            }
        }

        // Otherwise go to next sibling or climb up until we find one
        else {
            while (index.isValid()) {
                QModelIndex sibling = this->index(index.row() + (forward ? +1 : -1), 0, index.parent());
                if (sibling.isValid()) {
                    index = sibling;
                    break;
                }
                index = index.parent();
            }
        }
    }

    m_currentSearchIndex.reset();  // Reset current search index if no match is found
    statusBar->showMessage(tr("No matches found for '%1'").arg(query), 2000);
}

void JsonTreeModel::cancelSearch()
{

}

void JsonTreeModel::revealMatchInTree(QTreeView* treeView, const QModelIndex& index) {
    if (!index.isValid())
        return;

    // Expand all ancestors
    QModelIndex p = index.parent();
    while (p.isValid()) {
        treeView->expand(p);
        p = p.parent();
    }

    treeView->scrollTo(index, QAbstractItemView::PositionAtCenter);
    treeView->setCurrentIndex(index);
    treeView->selectionModel()->select(
        index,
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows
    );
}
