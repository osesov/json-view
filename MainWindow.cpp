#include "MainWindow.h"
#include "JsonCellEditorDelegate.h"
#include "treeViewUtil.h"

#include <QFileDialog>
#include <QStatusBar>
#include <QToolBar>
#include <QMenuBar>
#include <QMessageBox>
#include <QDir>
#include <QListView>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setupUI();
    setupMenu();
    setupConnections();
}

void MainWindow::setupUI() {
    auto* mainSplitter = new QSplitter(Qt::Horizontal);

    auto* tabWidget = new QTabWidget;
    tabWidget->addTab(new QListView, "Tab 1");
    tabWidget->addTab(new QListView, "Tab 2");

    tableView = new QTableView;
    treeView = new QTreeView;

    tableModel = new JsonTableModel(&jsonFile, this);
    tableView->setModel(tableModel);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    auto* rightSplitter = new QSplitter(Qt::Vertical);
    rightSplitter->addWidget(tableView);
    rightSplitter->addWidget(treeView);

    mainSplitter->addWidget(tabWidget);
    mainSplitter->addWidget(rightSplitter);
    mainSplitter->setStretchFactor(1, 1);

    treeView->setUniformRowHeights(false);
    treeView->setWordWrap(true);
    treeView->setAutoScroll(true);
    treeView->setSelectionBehavior(QAbstractItemView::SelectItems);
    treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    treeView->setItemDelegateForColumn(TreeViewColumn::ValueColumn, new JsonCellEditorDelegate(treeView));
    treeView->setEditTriggers(QAbstractItemView::CurrentChanged | QAbstractItemView::SelectedClicked);

    setCentralWidget(mainSplitter);
    statusBar()->showMessage("Ready");
}

void MainWindow::setupMenu() {
    QMenu* fileMenu = menuBar()->addMenu("&File");
    QAction* openAction = fileMenu->addAction("&Open");
    QAction* exitAction = fileMenu->addAction("E&xit");

    QMenu* editMenu = menuBar()->addMenu("&Edit");
    QAction* refreshAction = editMenu->addAction("&Refresh");

    QToolBar* toolbar = addToolBar("Main Toolbar");
    toolbar->addAction(openAction);
    toolbar->addAction(refreshAction);

    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(refreshAction, &QAction::triggered, this, &MainWindow::onRefresh);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::setupConnections() {
    connect(tableView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::onTableRowSelected);
}

void MainWindow::onOpenFile() {
    QString path = QFileDialog::getOpenFileName(
        this,
        "Open JSONL File",
        QDir::currentPath(),
        "Json Files (*.json *.jsonl);;All Files (*)"
    );
    if (!path.isEmpty()) {
        loadJson(path);
    }
}

void MainWindow::onRefresh() {
    tableModel->reload();
    // tableModel->beginResetModel();
    // jsonFile.reload();  // assumes such method exists
    // tableModel->endResetModel();
    // statusBar()->showMessage("Refreshed", 2000);
}

void MainWindow::onTableRowSelected(const QModelIndex& current, const QModelIndex&) {
    if (!current.isValid())
        return;

    int row = current.row();
    const JsonFile::LineInfo& line = jsonFile.line(row);
    const rapidjson::Value& val = line.doc;

    auto* model = new JsonTreeModel(&val, treeView);
    auto currentModel = treeView->model();

    treeView->setModel(model);
    treeView->expand(QModelIndex());

    openEditorsRecursive(treeView);

    delete currentModel;
}

void MainWindow::loadJson(const QString& filePath)
{
    setCursor(Qt::WaitCursor);
    auto status = jsonFile.open(filePath);
    unsetCursor();
    tableModel->reload();
    if (!status) {
        QMessageBox::critical(this, "Error", "Failed to open file.");
        return;
    }

    statusBar()->showMessage("Loaded " + filePath, 2000);
}
