#include "JsonTableModel.h"
#include "constants.h"

#include "json.h"
#include "Locale.h"

#include <regex>

#include <QTimer>
#include <QtConcurrent/QtConcurrent>
#include <QApplication>

JsonTableModel::JsonTableModel(JsonFile *jsonFile, QObject *parent)
    : QAbstractTableModel(parent), m_jsonFile(jsonFile)
{
    m_keys = jsonFile->topLevelKeys();

    connect(this, &JsonTableModel::updateColumns, this, &JsonTableModel::doUpdateColumns);
}

int JsonTableModel::rowCount(const QModelIndex &) const
{
    return static_cast<int>(m_jsonFile->size());
}

int JsonTableModel::columnCount(const QModelIndex &) const
{
    return static_cast<int>(m_keys.size() + 2);
}

QVariant JsonTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    int rowIndex = index.row();
    int colIndex = index.column();

    const auto &line = m_jsonFile->line(rowIndex);

    if (line.keysUpdated) {
        QTimer::singleShot(0, this, [this]() mutable {
            Q_EMIT updateColumns();
            // beginInsertColumns(QModelIndex(), m_keys.size(), m_keys.size());
            // endInsertColumns();

            // beginResetModel();
            // m_keys = m_jsonFile->topLevelKeys();
            // endResetModel();
        });
    }

    if (colIndex-- == 0) {
        return QString::number(line.index);
    }

    if (colIndex-- == 0) {
        return locale.toString(line.size);
    }

    if (line.doc.IsObject())
    {
        const auto &json = line.doc;
        const std::string key = m_keys[colIndex].toStdString();
        auto itr = json.FindMember(key.c_str());
        if (itr != json.MemberEnd())
        {
            std::string cacheKey = key + ":" + std::to_string(line.index);
            // check cache first
            const auto it = m_cache.find(cacheKey);

            if (it != m_cache.end())
                return it->second;

            // cache miss
            const auto &val = itr->value;
            QVariant result;
            if (val.IsNull())
                result = QString("null");
            else if (val.IsString())
                result = QString::fromUtf8(val.GetString());
            else if (val.IsInt64())
                result = locale.toString(val.GetInt64());
            else if (val.IsUint64())
                result = locale.toString(val.GetUint64());
            else if (val.IsBool())
                result = val.GetBool() ? "true" : "false";
            else if (val.IsDouble())
                result = locale.toString(val.GetDouble());
            else {
                result = QString::fromUtf8(toJsonString(val, MAX_JSON_STRING_LENGTH));
            }

            m_cache[cacheKey] = result;
            return result;
        }
    }

    return QVariant();
}

QVariant JsonTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section -- == 0)
            return "#";

        if (section -- == 0)
            return "Size";

        return m_keys[section];
    }
    return QVariant();
}

void JsonTableModel::doUpdateColumns() {
    int knownCol = m_keys.size();
    m_keys = m_jsonFile->topLevelKeys();
    int newCol = m_keys.size();

    if (newCol > knownCol) {
        beginInsertColumns(QModelIndex(), knownCol, newCol - 1);
        endInsertColumns();
    } else if (newCol < knownCol) {
        beginRemoveColumns(QModelIndex(), newCol, knownCol - 1);
        endRemoveColumns();
    }
}

void JsonTableModel::reload() {
    beginResetModel();
    m_keys = m_jsonFile->topLevelKeys();
    // e.g., m_jsonFile->reload() if needed
    endResetModel();
}

void JsonTableModel::search(bool forward, const QString& query, QTableView* tableView, QStatusBar * statusBar)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (searchFuture.isRunning()) {
        searchFuture.cancel();
        searchWatcher.waitForFinished();
    }

    searchFuture = QtConcurrent::run([this, forward, query, tableView, statusBar]() {
        searchCore(forward, query, tableView, statusBar);
    });

    searchWatcher.setFuture(searchFuture);
    connect(&searchWatcher, &QFutureWatcher<void>::finished, this, [this]() {
        QApplication::restoreOverrideCursor();
    });
}

void JsonTableModel::cancelSearch()
{
    if (searchFuture.isRunning()) {
        searchFuture.cancel();
        searchWatcher.waitForFinished();
    }
}

void JsonTableModel::searchCore(bool forward, const QString& query, QTableView* tableView, QStatusBar * statusBar)
{
    const int rowCount = m_jsonFile->size();
    if (rowCount == 0)
        return;

    const int nextRow = forward ? +1 : -1;

    QModelIndex index = tableView->currentIndex();
    bool skipCurrent = false;

    if (!index.isValid()) {
        index = this->index(0, 0, QModelIndex());
    }

    else if (m_currentSearchIndex && *m_currentSearchIndex == index) {
        skipCurrent = true; // don't search the current index again
    }

    else {
        skipCurrent = true;
    }

    const auto searchString = query.toStdString();

    while (index.isValid()) {
        if (skipCurrent) {
            skipCurrent = false;
        } else {

            int row = index.row();
            const auto line = m_jsonFile->lineText(row);

            if (line.find(searchString) != std::string_view::npos) {
                m_currentSearchIndex = index; // store the current search index

                // QModelIndex index = this->index(row, 0, QModelIndex());
                tableView->scrollTo(index, QAbstractItemView::PositionAtCenter);
                tableView->selectionModel()->select(
                    index,
                    QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows
                );

                tableView->setCurrentIndex(index);

                statusBar->showMessage(tr("Found match for '%1' at row %2").arg(query, QString::number(row + 1)), 2000);

                return; // stop at first match
            }
        }

        index = index.sibling(index.row() + nextRow, 0); // move to next row
    }

    m_currentSearchIndex.reset(); // no match found
    statusBar->showMessage(tr("No matches found for '%1'").arg(query), 2000);
}
