#include "HoverEditorHandler.h"
#include "constants.h"
#include "JsonTreeItem.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPlainTextEdit>

bool HoverEditorHandler::eventFilter(QObject* obj, QEvent* event)
{
    if (!treeView || !treeView->model())
        return false;

    if (event->type() == QEvent::MouseMove) {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        QModelIndex index = treeView->indexAt(mouseEvent->pos());

        if (index.column() == TreeViewColumn::ValueColumn && index.isValid()) {
            if (index != currentIndex) {
                if (currentIndex.isValid()) {
                    QPlainTextEdit* editor = qobject_cast<QPlainTextEdit*>(treeView->indexWidget(currentIndex));

                    if (editor) editor->setTextInteractionFlags(Qt::NoTextInteraction);
            //         treeView->closePersistentEditor(currentIndex);
                }

                JsonTreeItem* item = JsonTreeItem::fromIndex(index);

                if (item->isMultiline()) {
                    treeView->openPersistentEditor(index);
                    currentIndex = index;

                    QPlainTextEdit* editor = qobject_cast<QPlainTextEdit*>(treeView->indexWidget(index));
                    if (editor) {
                        editor->setReadOnly(true);
                        editor->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

                        editor->setFocus(Qt::MouseFocusReason);

                        // highlight the editor
                        QPalette pal = editor->palette();

                        // Use system's highlight color with low opacity blended into base
                        QColor highlight = pal.color(QPalette::Highlight);
                        QColor base = pal.color(QPalette::Base);

                        // Blend 15% of highlight over base (semi-transparent effect)
                        QColor blended = QColor(
                            (base.red()   * 85 + highlight.red()   * 15) / 100,
                            (base.green() * 85 + highlight.green() * 15) / 100,
                            (base.blue()  * 85 + highlight.blue()  * 15) / 100
                        );

                        QPalette p = editor->palette();
                        p.setColor(QPalette::Base, blended);
                        editor->setPalette(p);

                    }
                }
            }
        } else {
            if (currentIndex.isValid()) {
                QPlainTextEdit* editor = qobject_cast<QPlainTextEdit*>(treeView->indexWidget(currentIndex));

            //     treeView->closePersistentEditor(currentIndex);
                currentIndex = QModelIndex();

                if (editor) {
                    editor->setTextInteractionFlags(Qt::NoTextInteraction);
                    editor->clearFocus();
                    editor = nullptr;
                }
            }
        }
    }
    return false;
}
