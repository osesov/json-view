#include "MainWindow.h"
#include "JsonCellEditorDelegate.h"
#include "treeViewUtil.h"
#include "WheelSignalEmitter.h"
#include "HoverEditorHandler.h"

#include <QFileDialog>
#include <QStatusBar>
#include <QToolBar>
#include <QMenuBar>
#include <QMessageBox>
#include <QDir>
#include <QListView>
#include <QScrollBar>
#include <QShortcut>
#include <QVBoxLayout>

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

    tableSearchBar = new SearchBarWidget(this);
    auto * tableWidget = new QWidget;
    auto * tablePanel = new QVBoxLayout(tableWidget);
    tablePanel->setContentsMargins(0, 0, 0, 0);
    tablePanel->setSpacing(2);
    tablePanel->addWidget(tableSearchBar);
    tablePanel->addWidget(tableView);

    treeSearchBar = new SearchBarWidget(this);
    auto * treeWidget = new QWidget;
    auto * treePanel = new QVBoxLayout(treeWidget);
    treePanel->setContentsMargins(0, 0, 0, 0);
    treePanel->setSpacing(2);
    treePanel->addWidget(treeSearchBar);
    treePanel->addWidget(treeView);

    auto* rightSplitter = new QSplitter(Qt::Vertical);
    rightSplitter->addWidget(tableWidget);
    rightSplitter->addWidget(treeWidget);

    mainSplitter->addWidget(tabWidget);
    mainSplitter->addWidget(rightSplitter);
    mainSplitter->setStretchFactor(1, 1);

    // layout complete, do some set up...
    // ...search box
    tableSearchBar->hide();
    treeSearchBar->hide();

    // setup table view
    tableModel = new JsonTableModel(&jsonFile, this);
    tableView->setModel(tableModel);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    // setup tree view
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
    connect(treeView, &QTreeView::clicked, this, &MainWindow::openEditor);
}

void MainWindow::setupConnections() {
    connect(tableView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::onTableRowSelected);

    // connect(treeView, &QTreeView::clicked, this, [this](const QModelIndex& index) {
    //     if (index.column() == 1) {
    //         auto* editor = treeView->indexWidget(index);
    //         if (editor)
    //             editor->setFocus(Qt::MouseFocusReason);
    //     }
    // });
    // ---

#if 0
    connect(treeView, &QTreeView::clicked, this, &MainWindow::openEditorsForVisibleRows);
    connect(treeView->verticalScrollBar(), &QScrollBar::valueChanged, this, &MainWindow::openEditorsForVisibleRows);
    connect(treeView, &QTreeView::expanded, this, &MainWindow::openEditorsForVisibleRows);
    connect(treeView, &QTreeView::collapsed, this, &MainWindow::openEditorsForVisibleRows);

    auto* wheelEmitter = new WheelSignalEmitter();
    treeView->viewport()->installEventFilter(wheelEmitter);

    connect(wheelEmitter, &WheelSignalEmitter::wheelScrolled, this, &MainWindow::openEditorsForVisibleRows);
#else

    // make text editable on hover
    treeView->viewport()->setMouseTracking(true);
    auto* hoverHandler = new HoverEditorHandler(treeView, treeView);
    treeView->viewport()->installEventFilter(hoverHandler);
#endif

    // setup table search bar
    QShortcut* tableSearchShortcut = new QShortcut(QKeySequence("Ctrl+F"), tableView);
    tableSearchShortcut->setContext(Qt::ApplicationShortcut);

    connect(tableSearchShortcut, &QShortcut::activated, this, [this]() {
        tableSearchBar->show();
        tableSearchBar->setFocus();
    });

    connect(tableSearchBar, &SearchBarWidget::searchRequested, this, [this](const QString& text, bool forward) {
        tableModel->search(forward, text, tableView, statusBar());
    });

    // setup tree search bar
    QShortcut* treeSearchShortcut = new QShortcut(QKeySequence("Ctrl+Shift+F"), treeView);
    treeSearchShortcut->setContext(Qt::ApplicationShortcut);
    connect(treeSearchShortcut, &QShortcut::activated, this, [this]() {
        treeSearchBar->show();
        treeSearchBar->setFocus();
    });

    connect(treeSearchBar, &SearchBarWidget::searchRequested, this, [this](const QString& text, bool forward) {
        auto * treeModel = getTreeModel();
        if (!treeModel) {
            QMessageBox::warning(this, "Error", "Tree model is not set.");
            return;
        }
        treeModel->search(forward, text, treeView, statusBar());
    });

    // hide bars on escape
    QShortcut* escapeShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), tableView);
    connect(escapeShortcut, &QShortcut::activated, this, [this]() {
        tableModel->cancelSearch();
        tableSearchBar->hide();

        auto * treeModel = getTreeModel();
        if (treeModel) {
            treeModel->cancelSearch();
            treeSearchBar->hide();
        }
    });
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

JsonTreeModel * MainWindow::getTreeModel()
{
    return dynamic_cast<JsonTreeModel *>(treeView->model());
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

    connect(treeView->model(), &QAbstractItemModel::rowsInserted, this, &MainWindow::openEditorsForVisibleRows);

    // openEditorsRecursive(treeView);

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

void MainWindow::openEditor(const QModelIndex& index)
{
    if (index.column() == TreeViewColumn::ValueColumn) {
        auto* editor = treeView->indexWidget(index);
        if (editor)
            editor->setFocus(Qt::MouseFocusReason);
    }
}

void MainWindow::processArguments(const QStringList& args) {
    // Handle args as needed (ignore args[0] which is the executable path)
    if (args.isEmpty()) {
        return;
    }

    for (int i = 0; i < args.size(); ++i) {
        QString arg = args[i];
        loadJson(arg);
    }
}

#if 0
void MainWindow::openEditorsForVisibleRows() {
    auto* model = treeView->model();
    auto* viewport = treeView->viewport();
    QRect visibleRect = viewport->rect();

    for (int row = 0; row < model->rowCount(); ++row) {
        QModelIndex index = model->index(row, TreeViewColumn::ValueColumn);
        QRect indexRect = treeView->visualRect(index);
        if (visibleRect.intersects(indexRect)) {
            JsonTreeItem* item = JsonTreeItem::fromIndex(index);
            if (item->isMultiline()) {
                treeView->openPersistentEditor(index);
            }
        }

        // Recursively handle children if expanded
        QModelIndex child = model->index(row, 0);
        if (treeView->isExpanded(child)) {
            openEditorsForVisibleChildren(child);
        }
    }
}

void MainWindow::openEditorsForVisibleChildren(const QModelIndex& parent) {
    auto* model = treeView->model();
    for (int row = 0; row < model->rowCount(parent); ++row) {
        QModelIndex index = model->index(row, TreeViewColumn::ValueColumn, parent);
        QRect indexRect = treeView->visualRect(index);
        if (treeView->viewport()->rect().intersects(indexRect)) {
            JsonTreeItem* item = JsonTreeItem::fromIndex(index);
            if (item->isMultiline()) {
                treeView->openPersistentEditor(index);
            }
        }

        QModelIndex child = model->index(row, 0, parent);
        if (treeView->isExpanded(child)) {
            openEditorsForVisibleChildren(child);
        }
    }
}

#else

void MainWindow::openEditorsForVisibleRows()
{
    QTreeView* view = treeView;
    if (!view || !view->model()) return;

    const QRect visibleRect = view->viewport()->rect();
    const int rowHeight = view->sizeHintForRow(0);
    const int step = qMax(1, rowHeight / 2);

    QSet<QModelIndex> seen;

    for (int y = visibleRect.top(); y < visibleRect.bottom(); y += step) {
        QModelIndex index = view->indexAt(QPoint(0, y));
        if (!index.isValid()) continue;

        QModelIndex valueIndex = index.sibling(index.row(), TreeViewColumn::ValueColumn);
        if (seen.contains(valueIndex)) continue;
        seen.insert(valueIndex);

        JsonTreeItem* item = JsonTreeItem::fromIndex(valueIndex);

        if (item->isMultiline()) {
            view->openPersistentEditor(valueIndex);
        }
    }
}


#endif
