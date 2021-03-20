/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2018 Mehmet Salih Çalışkan <msalihcaliskan@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

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
