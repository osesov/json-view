#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QTableView>
#include <QTreeView>
#include <QTabWidget>
#include <QAction>

#include "JsonFile.h"
#include "JsonTableModel.h"
#include "JsonTreeModel.h"
#include "SearchBarWidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    void processArguments(const QStringList& args);

private slots:
    void onOpenFile();
    void onRefresh();
    void onTableRowSelected(const QModelIndex& current, const QModelIndex&);

private:
    JsonFile jsonFile;
    JsonTableModel* tableModel = nullptr;
    QTableView* tableView = nullptr;
    QTreeView* treeView = nullptr;
    SearchBarWidget* tableSearchBar = nullptr;
    SearchBarWidget* treeSearchBar = nullptr;

    void setupUI();
    void setupMenu();
    void setupConnections();
    void openEditor(const QModelIndex& index);

    void loadJson(const QString& filePath);
    void openEditorsForVisibleRows();
    JsonTreeModel * getTreeModel();

    // void openEditorsForVisibleChildren(const QModelIndex& parent);
};
