#pragma once

#include "constants.h"

#include <QTreeView>
#include <QModelIndex>

void openEditorsRecursive(QTreeView* view, const QModelIndex& parent = QModelIndex());
