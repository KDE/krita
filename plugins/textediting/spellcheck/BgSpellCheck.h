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

#ifndef BGSPELLCHECK_H
#define BGSPELLCHECK_H

#include <sonnet/backgroundchecker.h>
#include <sonnet/speller.h>

#include <QTextCursor>

using namespace Sonnet;

class QTextDocument;

class BgSpellCheck : public BackgroundChecker
{
    Q_OBJECT
public:
    explicit BgSpellCheck(const Speller &speller, QObject *parent = 0);
    void startRun(QTextDocument *document, int startPosition, int endPosition);

protected:
    /// reimplemented
    virtual QString fetchMoreText();

private slots:
    void foundMisspelling(const QString &word, int start);

signals:
    void misspelledWord(const QString &word, int startPosition, bool misspelled);

private:
    QTextDocument *m_document;
    QTextCursor m_cursor;

    int m_currentPosition;
    int m_endPosition;
};

#endif
