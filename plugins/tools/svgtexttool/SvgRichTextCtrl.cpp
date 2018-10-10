#include "SvgRichTextCtrl.h"
#include <QMimeData>

SvgRichTextCtrl::SvgRichTextCtrl(QWidget* parent /*= nullptr*/)
: QTextEdit(parent)
{
}

void SvgRichTextCtrl::insertFromMimeData(const QMimeData *source)
{
    if (!source->hasHtml() && source->hasText()) {
        QTextCursor cursor = textCursor();
        cursor.insertText(source->text());
    } else {
        QTextEdit::insertFromMimeData(source);
    }
}
