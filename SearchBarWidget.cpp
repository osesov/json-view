// SearchBarWidget.cpp
#include "SearchBarWidget.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QTimer>

SearchBarWidget::SearchBarWidget(QWidget* parent)
    : QWidget(parent)
{
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Search...");
    searchEdit->setClearButtonEnabled(true);
    searchEdit->setFocusPolicy(Qt::ClickFocus);

    forwardBtn = new QToolButton(this);
    backwardBtn = new QToolButton(this);

    forwardBtn->setIcon(QIcon::fromTheme("go-down"));   // or use right arrow
    backwardBtn->setIcon(QIcon::fromTheme("go-up"));    // or use left arrow

    forwardBtn->setToolTip("Find Next (F3)");
    backwardBtn->setToolTip("Find Previous (Shift+F3)");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(searchEdit);
    layout->addWidget(backwardBtn);
    layout->addWidget(forwardBtn);
    setLayout(layout);

    connect(searchEdit, &QLineEdit::returnPressed, this, [this]() {
        emit searchRequested(searchEdit->text(), true);
    });
    connect(forwardBtn, &QToolButton::clicked, this, [this]() {
        emit searchRequested(searchEdit->text(), true);
    });
    connect(backwardBtn, &QToolButton::clicked, this, [this]() {
        emit searchRequested(searchEdit->text(), false);
    });
}

// void SearchBarWidget::activate()
// {
//     // TODO: need a better way to activate the search bar
//     searchEdit->setFocus();
// }

void SearchBarWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);  // call base class

    if (searchEdit) {
        QTimer::singleShot(0, this, [this]() {
            searchEdit->setFocus();
            searchEdit->selectAll();
        });
    }
}
