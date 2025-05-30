// SearchBarWidget.h
#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QToolButton>
#include <QCheckBox>

class SearchBarWidget : public QWidget {
    Q_OBJECT
public:
    explicit SearchBarWidget(QWidget* parent = nullptr);

signals:
    void searchRequested(const QString& text, bool forward);

protected:
    void showEvent(QShowEvent* event) override;

private:
    QLineEdit* searchEdit;
    QToolButton* forwardBtn;
    QToolButton* backwardBtn;
};
