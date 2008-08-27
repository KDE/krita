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

#ifndef KOFINDSTRATEGYBASE_H
#define KOFINDSTRATEGYBASE_H

class KFindDialog;
class QTextCursor;
class FindDirection;

/**
 * Abstract base class for the different strategies
 * find and replace in KoFind.
 */
class KoFindStrategyBase
{
public:
    KoFindStrategyBase() {}
    virtual ~KoFindStrategyBase() {}

    /**
     * Get the find dialog
     */
    virtual KFindDialog * dialog() = 0;

    /**
     * Reset internal status
     *
     * E.g. set number of matches found to 0
     */
    virtual void reset() = 0;

    /**
     * Displays the final dialog
     */
    virtual void displayFinalDialog() = 0;

    /**
     * This get called when a match was found
     *
     * @param findDirection The find direction helper that can be used for highlighting
     */
    virtual bool foundMatch(QTextCursor & cursor, FindDirection * findDirection) = 0;
};

#endif /* KOFINDSTRATEGYBASE_H */
