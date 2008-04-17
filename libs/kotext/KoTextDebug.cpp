/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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

#include "KoTextDebug.h"

#include <QTextDocument>
#include <QTextFrame>
#include <QTextBlock>
#include <QTextTable>
#include <QTextFragment>
#include <QTextList>
#include <QDebug>

int KoTextDebug::depth = 0;
const int KoTextDebug::INDENT = 2;

void KoTextDebug::dumpDocument(QTextDocument *document)
{
    qDebug() << "<document>";
    dumpFrame(document->rootFrame());
    qDebug() << "</document>";
}

void KoTextDebug::dumpFrame(QTextFrame *frame)
{
    depth += INDENT;
    qDebug("%*s%s", depth, " ", "<frame>");

    QTextFrame::iterator iterator = frame->begin();

    for (; !iterator.atEnd() && !iterator.atEnd(); ++iterator) {
        QTextFrame *childFrame = iterator.currentFrame();
        QTextBlock textBlock = iterator.currentBlock();

        if (childFrame) {
            QTextTable *table = qobject_cast<QTextTable *>(childFrame);
            if (table) {
                dumpTable(table);
            } else {
                dumpFrame(frame);
            }
        } else if (textBlock.isValid()) {
            dumpBlock(textBlock);
        }
    }

    qDebug("%*s%s", depth, " ", "</frame>");
    depth -= INDENT;
}

void KoTextDebug::dumpBlock(const QTextBlock &block)
{
    depth += INDENT;

    QTextList *list = block.textList();

    QString startTag;
    if (list) {
        startTag.sprintf("<block listitem=\"%d/%d\">", list->itemNumber(block)+1, list->count());
    } else {
        startTag = "<block>";
    }

    qDebug("%*s%s", depth, " ", qPrintable(startTag));

    QTextBlock::Iterator iterator = block.begin();
    for(; !iterator.atEnd() && !iterator.atEnd(); ++iterator) {
        QTextFragment fragment = iterator.fragment();
        if (fragment.isValid()) {
            dumpFragment(fragment);
        }
    }
    qDebug("%*s%s", depth, " ", "</block>");
    depth -= INDENT;
    if (block.next().isValid())
        qDebug(" ");
}

void KoTextDebug::dumpTable(QTextTable *)
{
    depth += INDENT;
    qDebug("%*s%s", depth, " ", "<table>");
    // FIXME
    qDebug("%*s%s", depth, " ", "</table>");
    depth -= INDENT;
}

void KoTextDebug::dumpFragment(const QTextFragment &fragment)
{
    depth += INDENT;

    QTextCharFormat textFormat = fragment.charFormat();
    QTextImageFormat imageFormat = textFormat.toImageFormat();

    QString startTag;
    if (imageFormat.isValid()) {
        qDebug("%*s%s", depth, " ", "<fragment type=\"image\">");
    } else {
        QString formatString = QString(" font=\"%1\"").arg(textFormat.font().toString());
        if (textFormat.isAnchor()) {
            formatString.append(QString(" achorHref=\"%1\"").arg(textFormat.anchorHref()));
            formatString.append(QString(" achorName=\"%1\"").arg(textFormat.anchorName()));
        }
        startTag = QString("<fragment type=\"char\"%1>").arg(formatString);
    }

    qDebug("%*s%s", depth, " ", qPrintable(startTag));

    qDebug("%*s|%s|", depth+INDENT, " ", qPrintable(fragment.text()));

    qDebug("%*s%s", depth, " ", "</fragment>");
    depth -= INDENT;
}

