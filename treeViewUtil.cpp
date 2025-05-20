#include "treeViewUtil.h"
#include "JsonTreeItem.h"

void openEditorsRecursive(QTreeView* view, const QModelIndex& parent) {
    int rows = view->model()->rowCount(parent);
    for (int i = 0; i < rows; ++i) {
        QModelIndex index0 = view->model()->index(i, TreeViewColumn::KeyColumn, parent);
        QModelIndex index1 = view->model()->index(i, TreeViewColumn::ValueColumn, parent);

        JsonTreeItem* item = JsonTreeItem::fromIndex(index1);

        if (item->isMultiline()) {
            view->openPersistentEditor(index1);
        }

        if (view->model()->hasChildren(index0)) {
            openEditorsRecursive(view, index0);
        }
    }
}
