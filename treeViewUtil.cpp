#include "treeViewUtil.h"

void openEditorsRecursive(QTreeView* view, const QModelIndex& parent) {
    int rows = view->model()->rowCount(parent);
    for (int i = 0; i < rows; ++i) {
        QModelIndex index0 = view->model()->index(i, TreeViewColumn::KeyColumn, parent);
        QModelIndex index1 = view->model()->index(i, TreeViewColumn::ValueColumn, parent);

        QString value = index1.data(Qt::DisplayRole).toString();
        if (value.contains('\n') || value.length() > 300) {  // Heuristic: multiline or long
            view->openPersistentEditor(index1);
        }

        if (view->model()->hasChildren(index0)) {
            openEditorsRecursive(view, index0);
        }
    }
}
