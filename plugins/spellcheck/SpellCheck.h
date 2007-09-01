/* This file is part of the KDE project
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
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

#ifndef SPELLCHECK_H
#define SPELLCHECK_H

#include <QTextCursor>
#include <QQueue>

#include <KoTextEditingPlugin.h>

#include <sonnet/speller.h>

class BgSpellCheck;

class SpellCheck : public KoTextEditingPlugin
{
    Q_OBJECT
public:
    SpellCheck();

    void finishedWord(QTextDocument *document, int cursorPosition);
    void finishedParagraph(QTextDocument *document, int cursorPosition);
    void checkSection(QTextDocument *document, int startPosition, int endPosition);

private slots:
    void highlightMisspelled(const QString &word, int startPosition);
    void dequeueDocument();

private:
    Sonnet::Speller m_speller;
    QTextCursor m_cursor;
    QString m_word;
    BgSpellCheck *m_bgSpellCheck;
    QQueue<QTextDocument *> m_documentsQueue;
    bool m_enableSpellCheck;
};

#endif
