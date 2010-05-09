/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "FindDirection_p.h"

#include <QTextCursor>
#include <KoResourceManager.h>

#include "KoText.h"
#include "KoFind_p.h"

FindDirection::FindDirection(KoResourceManager *provider)
        : m_provider(provider)
{
}

FindDirection::~FindDirection()
{
}

FindForward::FindForward(KoResourceManager *provider)
        : FindDirection(provider)
{
}

FindForward::~FindForward()
{
}

bool FindForward::positionReached(const QTextCursor &currentPos, const QTextCursor &endPos)
{
    return currentPos > endPos;
}

void FindForward::positionCursor(QTextCursor &currentPos)
{
    currentPos.movePosition(QTextCursor::Start);
}

void FindForward::select(const QTextCursor &cursor)
{
    m_provider->setResource(KoText::CurrentTextPosition, cursor.position());
    m_provider->setResource(KoText::CurrentTextAnchor, cursor.anchor());
}

void FindForward::nextDocument(QTextDocument *document, KoFindPrivate *findPrivate)
{
    findPrivate->findDocumentSetNext(document);
}

FindBackward::FindBackward(KoResourceManager *provider)
        : FindDirection(provider)
{
}

FindBackward::~FindBackward()
{
}

bool FindBackward::positionReached(const QTextCursor &currentPos, const QTextCursor &endPos)
{
    return currentPos < endPos;
}

void FindBackward::positionCursor(QTextCursor &currentPos)
{
    currentPos.movePosition(QTextCursor::End);
}

void FindBackward::select(const QTextCursor &cursor)
{
    m_provider->setResource(KoText::CurrentTextPosition, cursor.anchor());
    m_provider->setResource(KoText::CurrentTextAnchor, cursor.position());
}

void FindBackward::nextDocument(QTextDocument *document, KoFindPrivate *findPrivate)
{
    findPrivate->findDocumentSetPrevious(document);
}

