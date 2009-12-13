/* This file is part of the KDE project
   Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOEVENTACTIONDATA_H
#define KOEVENTACTIONDATA_H

#include "flake_export.h"

class KoShape;
class KoEventAction;

/**
 * Data passed to an event action in setData.
 *
 * Applications can extend that to pass additional data to the event action.
 */
class FLAKE_EXPORT KoEventActionData
{
public:
    KoEventActionData(KoShape *shape, KoEventAction *eventAction);
    virtual ~KoEventActionData();

    /**
     * Get the shape
     *
     * @return the shape or 0 if there is none.
     */
    KoShape *shape();

    /**
     * Get the event action
     *
     * @return the event action or 0 if there is none
     */
    KoEventAction *eventAction();

private:
    class Private;
    Private * const d;
};

#endif /* KOEVENTACTIONDATA_H */
