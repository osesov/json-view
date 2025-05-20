#include "JsonCellEditorDelegate.h"

QWidget *JsonCellEditorDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    QPlainTextEdit *editor = new QPlainTextEdit(parent);
    editor->setReadOnly(true);
    editor->setFrameStyle(QFrame::NoFrame);
    editor->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    editor->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    editor->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    editor->setFrameShape(QFrame::Box);

    editor->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    editor->setFocusPolicy(Qt::StrongFocus);
    // editor->installEventFilter(this);  // if you want to forward focus or key events


    // // Use palette to match theme
    // QPalette pal = editor->palette();
    // pal.setColor(QPalette::Base, pal.color(QPalette::Base));  // background
    // pal.setColor(QPalette::Text, pal.color(QPalette::Text));  // text
    // pal.setColor(QPalette::Highlight, pal.color(QPalette::Highlight));
    // pal.setColor(QPalette::HighlightedText, pal.color(QPalette::HighlightedText));
    // editor->setPalette(pal);

    return editor;
}

void JsonCellEditorDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString text = index.data(Qt::DisplayRole).toString();
    QPlainTextEdit *edit = qobject_cast<QPlainTextEdit *>(editor);
    if (edit) {
        edit->setPlainText(text);
        // QMetaObject::invokeMethod(edit, "setFocus", Qt::QueuedConnection);
    }
}

void JsonCellEditorDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}

QSize JsonCellEditorDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QString text = index.data(Qt::DisplayRole).toString();
    QFontMetrics fm(option.font);

    int lineHeight = fm.lineSpacing();
    int maxLines = 20;  // limit to N lines

    int lines = text.count('\n') + 1;
    int height = qMin(lines, maxLines) * lineHeight + 4;

    return QSize(option.rect.width(), height);
}
