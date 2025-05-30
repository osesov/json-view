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
    restartCheck = new QCheckBox("Restart", this);


    forwardBtn->setIcon(QIcon::fromTheme("go-down"));   // or use right arrow
    backwardBtn->setIcon(QIcon::fromTheme("go-up"));    // or use left arrow

    forwardBtn->setToolTip("Find Next (F3)");
    backwardBtn->setToolTip("Find Previous (Shift+F3)");
    restartCheck->setToolTip("Start from beginning");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(searchEdit);
    layout->addWidget(backwardBtn);
    layout->addWidget(forwardBtn);
    layout->addWidget(restartCheck);
    setLayout(layout);

    connect(searchEdit, &QLineEdit::returnPressed, this, [this]() {
        emit searchRequested(searchEdit->text(), true, restartCheck->isChecked());
        restartCheck->setChecked(false);
    });
    connect(forwardBtn, &QToolButton::clicked, this, [this]() {
        emit searchRequested(searchEdit->text(), true, restartCheck->isChecked());
        restartCheck->setChecked(false);
    });
    connect(backwardBtn, &QToolButton::clicked, this, [this]() {
        emit searchRequested(searchEdit->text(), false, restartCheck->isChecked());
        restartCheck->setChecked(false);
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
