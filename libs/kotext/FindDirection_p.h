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

#ifndef FINDDIRECTION_P_H
#define FINDDIRECTION_P_H

class KoResourceManager;
class KoFindPrivate;
class QTextCursor;
class QTextDocument;

class FindDirection
{
public:
    FindDirection(KoResourceManager *provider);
    virtual ~FindDirection();

    virtual bool positionReached(const QTextCursor &currentPos, const QTextCursor &endPos) = 0;

    virtual void positionCursor(QTextCursor &currentPos) = 0;

    virtual void select(const QTextCursor &cursor) = 0;

    virtual void nextDocument(QTextDocument *document, KoFindPrivate *findPrivate) = 0;

protected:
    KoResourceManager *m_provider;
};

class FindForward : public FindDirection
{
public:
    FindForward(KoResourceManager *provider);
    virtual ~FindForward();

    virtual bool positionReached(const QTextCursor &currentPos, const QTextCursor &endPos);

    virtual void positionCursor(QTextCursor &currentPos);

    virtual void select(const QTextCursor &cursor);

    virtual void nextDocument(QTextDocument *document, KoFindPrivate *findPrivate);
};

class FindBackward : public FindDirection
{
public:
    FindBackward(KoResourceManager *provider);
    virtual ~FindBackward();

    virtual bool positionReached(const QTextCursor &currentPos, const QTextCursor &endPos);

    virtual void positionCursor(QTextCursor &currentPos);

    virtual void select(const QTextCursor &cursor);

    virtual void nextDocument(QTextDocument *document, KoFindPrivate *findPrivate);
};

#endif // FINDDIRECTION_P_H
