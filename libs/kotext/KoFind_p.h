/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Fredy Yanardi <fyanardi@gmail.com>
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

#ifndef KOFIND_P_H
#define KOFIND_P_H

#include <QTextCursor>

#include "KoFindStrategy.h"
#include "KoReplaceStrategy.h"
#include "FindDirection_p.h"

class KoFind;
class KoResourceManager;
class QAction;
class QTextDocument;
class QVariant;
class QWidget;

class KoFindPrivate
{
public:
    KoFindPrivate(KoFind *find, KoResourceManager *crp, QWidget *w);

    void resourceChanged(int key, const QVariant &variant);

    void findActivated();

    void findNextActivated();

    void findPreviousActivated();

    void replaceActivated();

    // executed when the user presses the 'find' button.
    void startFind();

    // executed when the user presses the 'replace' button.
    void startReplace();

    QAction *findNext;
    QAction *findPrev;

    void findDocumentSetNext(QTextDocument * document);
    void findDocumentSetPrevious(QTextDocument * document);

protected:
    void parseSettingsAndFind();

private:
    KoFind * q;
    KoResourceManager *provider;
    KoFindStrategy findStrategy; /// strategy used for find
    KoReplaceStrategy replaceStrategy; /// strategy used for replace
    KoFindStrategyBase * strategy; /// the current strategy used

    QTextDocument *document;
    QTextDocument *startDocument;
    QTextCursor lastKnownPosition;
    bool restarted;
    bool start; /// if true find/replace is restarted
    bool inFind; /// if true find/replace is running (not showing the dialog)
    QTextCursor startPosition;
    QTextCursor endPosition;
    FindDirection * findDirection; /// the current direction used
    FindForward findForward;
    FindBackward findBackward;
};

#endif /* KOFIND_P_H */
