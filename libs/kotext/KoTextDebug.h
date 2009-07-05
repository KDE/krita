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

#ifndef KOTEXTDEBUG_H
#define KOTEXTDEBUG_H

#include "kotext_export.h"

class QTextDocument;
class QTextFrame;
class QTextBlock;
class QTextTable;
class QTextFragment;
class QTextCharFormat;
class QTextListFormat;
class QTextTableFormat;
class QTextBlockFormat;
class KoParagraphStyle;
class KoCharacterStyle;

#include <QMap>
#include <QVariant>

class KOTEXT_EXPORT KoTextDebug
{
public:
    static void dumpDocument(const QTextDocument *document);
    static void dumpFrame(const QTextFrame *frame);
    static void dumpBlock(const QTextBlock &block);
    static void dumpTable(const QTextTable *table);
    static void dumpFragment(const QTextFragment &fragment);
    static QString textAttributes(const QTextCharFormat &format);
    static QString textAttributes(const KoCharacterStyle &style);
    static QString paraAttributes(const QTextBlockFormat &format);
    static QString paraAttributes(const KoParagraphStyle &style);
    static QString listAttributes(const QTextListFormat &format);
    static QString tableAttributes(const QTextTableFormat &tableformat);
    static QString inlineObjectAttributes(const QTextCharFormat &textFormat);

private:
    KoTextDebug();
    KoTextDebug(const KoTextDebug&);
    KoTextDebug operator=(const KoTextDebug&);

    static const QTextDocument *document;
    static int depth;
    static const int INDENT;
};

#endif /* KOTEXTDEBUG_H */
