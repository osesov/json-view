#pragma once

#include "TreeViewColumn.h"

#include <QTreeView>
#include <QModelIndex>

void openEditorsRecursive(QTreeView* view, const QModelIndex& parent = QModelIndex());
