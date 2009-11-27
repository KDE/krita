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

#ifndef KOEVENTACTIONFACTORY_H
#define KOEVENTACTIONFACTORY_H

#include <QString>

#include "flake_export.h"

class KoEventAction;
class KoEventActionWidget;

/// API docs go here
class FLAKE_EXPORT KoEventActionFactory
{
public:
    /**
     * Factory to create events
     *
     * @param id The id of the event action
     * @param action Only presentation event actions need to set the action. It is not used
     *               for script event actions.
     */
    explicit KoEventActionFactory(const QString &id, const QString & action = QString(""));
    virtual ~KoEventActionFactory();

    /**
     * Create the event action.
     */
    virtual KoEventAction * createEventAction() = 0;

    /**
     * Create the widget to configure the action
     */
    virtual KoEventActionWidget * createOptionWidget() = 0;

    /**
     * The action is used to differentiate presentation effects.
     */
    const QString & action();

    /**
     * return the id for the variable this factory creates.
     * @return the id for the variable this factory creates.
     */
    const QString & id() const;

private:
    class Private;
    Private * const d;
};

#endif /* KOEVENTACTIONFACTORY_H */
