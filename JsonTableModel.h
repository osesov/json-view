#pragma once

#include <QAbstractTableModel>
#include <QStringList>

#include "JsonFile.h"

class JsonTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    JsonTableModel(JsonFile* jsonFile, QObject* parent = nullptr);

    int rowCount(const QModelIndex&) const override;
    int columnCount(const QModelIndex&) const override;

    QVariant data(const QModelIndex& index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void reload();

signals:
    void updateColumns() const;

public slots:
    void doUpdateColumns();

private:
    JsonFile* m_jsonFile;
    std::vector<QString> m_keys;
};
