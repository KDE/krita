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

#ifndef CHANGECASE_H
#define CHANGECASE_H

#include <QTextCursor>

#include <KoTextEditingPlugin.h>

class QTextDocument;
class QRadioButton;

class Changecase : public KoTextEditingPlugin
{
    Q_OBJECT
public:
    Changecase();

    void finishedWord(QTextDocument *document, int cursorPosition);
    void finishedParagraph(QTextDocument *document, int cursorPosition);
    void checkSection(QTextDocument *document, int startPosition, int endPosition);

private slots:
    void process();

private:
    void sentenceCase();
    void lowerCase();
    void upperCase();
    void initialCaps();
    void toggleCase();

    QRadioButton *m_sentenceCaseRadio;
    QRadioButton *m_lowerCaseRadio;
    QRadioButton *m_upperCaseRadio;
    QRadioButton *m_initialCapsRadio;
    QRadioButton *m_toggleCaseRadio;

    QTextCursor m_cursor;
    QTextDocument *m_document;
    int m_startPosition;
    int m_endPosition;
};

#endif
