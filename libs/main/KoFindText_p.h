/* This file is part of the KDE project
 *
 * Copyright (c) 2010 Arjen Hiemstra <ahiemstra@heimr.nl>
 * Copyright (C) 2011 Thorsten Zachmann <zachmann@kde.org>
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
#ifndef KoFindText_p_h
#define KoFindText_p_h

#include "KoFindText.h"

#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextLayout>
#include <QPalette>
#include <QStyle>
#include <QApplication>
#include <QAbstractTextDocumentLayout>

#include <kdebug.h>
#include <klocalizedstring.h>

#include <KoText.h>
#include <KoTextDocument.h>
#include <KoShape.h>
#include <KoShapeContainer.h>
#include <KoTextShapeData.h>

#include "KoFindOptionSet.h"
#include "KoFindOption.h"
#include "KoDocument.h"

class KoFindText::Private
{
public:
    Private(KoFindText* qq) : q(qq), selectionStart(-1), selectionEnd(-1) { }

    void updateSelections();
    void updateDocumentList();
    void documentDestroyed(QObject *document);
    void updateCurrentMatch(int position);
    static void initializeFormats();

    KoFindText *q;

    QList<QTextDocument*> documents;

    QTextCursor currentCursor;
    QTextCursor selection;
    QHash<QTextDocument*, QVector<QAbstractTextDocumentLayout::Selection> > selections;

    int selectionStart;
    int selectionEnd;

    static QTextCharFormat highlightFormat;
    static QTextCharFormat currentMatchFormat;
    static QTextCharFormat currentSelectionFormat;
    static QTextCharFormat replacedFormat;
    static bool formatsInitialized;

    QPair<QTextDocument*, int> currentMatch;
};

#endif