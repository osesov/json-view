#pragma once

#include "JsonFile.h"

#include <QAbstractTableModel>
#include <QStringList>
#include <QRegularExpression>
#include <QTableView>
#include <QFuture>
#include <QFutureWatcher>
#include <QStatusBar>

class JsonTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    JsonTableModel(JsonFile* jsonFile, QObject* parent = nullptr);

    int rowCount(const QModelIndex&) const override;
    int columnCount(const QModelIndex&) const override;

    QVariant data(const QModelIndex& index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void reload();
    void search(bool forward, const QString& query, QTableView* tableView, QStatusBar *);
    void cancelSearch();

signals:
    void updateColumns() const;

public slots:
    void doUpdateColumns();

private:
    JsonFile* m_jsonFile;
    std::vector<QString> m_keys;
    std::optional<QString> m_query;
    std::optional<QModelIndex> m_currentSearchIndex;

    mutable std::map<std::string, QVariant> m_cache;

    QFuture<void> searchFuture;
    QFutureWatcher<void> searchWatcher;

    void searchCore(bool forward, const QString& query, QTableView* tableView, QStatusBar *);

};
