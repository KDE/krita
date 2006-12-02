/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoTextSelectionHandler.h"
#include "KoFontDia.h"

#include <kdebug.h>
#include <QTextCharFormat>
#include <QFont>
#include <QTextCursor>
#include <QTextBlock>

void KoTextSelectionHandler::bold(bool bold) {
    Q_ASSERT(m_caret);
    QTextCharFormat cf = m_caret->charFormat();
    cf.setFontWeight( bold ? QFont::Bold : QFont::Normal );
    m_caret->mergeCharFormat(cf);
}

void KoTextSelectionHandler::italic(bool italic) {
    Q_ASSERT(m_caret);
    QTextCharFormat cf = m_caret->charFormat();
    cf.setFontItalic(italic);
    m_caret->mergeCharFormat(cf);
}

void KoTextSelectionHandler::underline(bool underline) {
    Q_ASSERT(m_caret);
    QTextCharFormat cf = m_caret->charFormat();
    cf.setFontUnderline(underline);
    m_caret->mergeCharFormat(cf);
}

void KoTextSelectionHandler::strikeOut(bool strikeout) {
    Q_ASSERT(m_caret);
    QTextCharFormat cf = m_caret->charFormat();
    cf.setFontStrikeOut(strikeout);
    m_caret->mergeCharFormat(cf);
}

void KoTextSelectionHandler::insertFrameBreak() {
    QTextBlock block = m_caret->block();
/*
    if(m_caret->position() == block.position() && block.length() > 0) { // start of parag
        QTextBlockFormat bf = m_caret->blockFormat();
        bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysAfter);
        m_caret->setBlockFormat(bf);
    } else { */
        QTextBlockFormat bf = m_caret->blockFormat();
//       if(m_caret->position() != block.position() + block.length() -1 ||
//               bf.pageBreakPolicy() != QTextFormat::PageBreak_Auto) // end of parag or already a pagebreak
            m_caret->insertText("\n");
        bf = m_caret->blockFormat();
        bf.setPageBreakPolicy(QTextFormat::QTextFormat::PageBreak_AlwaysBefore);
        m_caret->setBlockFormat(bf);
    //}
}

void KoTextSelectionHandler::setFontSize(int size) {
    // TODO
}

void KoTextSelectionHandler::increaseFontSize() {
    // TODO
}

void KoTextSelectionHandler::decreaseFontSize() {
    // TODO
}

void KoTextSelectionHandler::setHorizontalTextAlignment(Qt::Alignment align) {
    // TODO
    // left,right,center,justified
}

void KoTextSelectionHandler::setVerticalTextAlignment(Qt::Alignment align) {
    // TODO
    // superscript, subscript, normal
}

void KoTextSelectionHandler::setTextColor(const QColor &color) {
    // TODO
}

void KoTextSelectionHandler::setTextBackgroundColor(const QColor &color) {
    // TODO
}

QString KoTextSelectionHandler::selectedText() const {
    // TODO
    return "";
}

void KoTextSelectionHandler::insert(const QString &text) {
    m_caret->insertText(text);
}

void KoTextSelectionHandler::selectFont(QWidget *parent) {
    KoFontDia *fontDlg = new KoFontDia( m_caret->charFormat()); // , 0, parent);
    fontDlg->exec();
QTextCharFormat cf = fontDlg->format();
kDebug() << "font now: " << cf.font().family() << " " << cf.font().pointSize() << endl;
    m_caret->mergeCharFormat(fontDlg->format());
    delete fontDlg;
}
