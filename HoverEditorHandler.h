#pragma once

#include <QObject>
#include <QTreeView>
#include <QModelIndex>
#include <QPlainTextEdit>

class HoverEditorHandler : public QObject
{
    Q_OBJECT
public:
    HoverEditorHandler(QTreeView* view, QObject* parent = nullptr)
        : QObject(parent), treeView(view) {}

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    QTreeView* treeView;
    QModelIndex currentIndex;
};
