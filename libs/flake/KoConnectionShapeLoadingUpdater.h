/* This file is part of the KDE project
   Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOCONNECTIONSHAPELOADINGUPDATER_H
#define KOCONNECTIONSHAPELOADINGUPDATER_H

#include <KoLoadingShapeUpdater.h>

class KoConnectionShape;
class KoShape;

class KoConnectionShapeLoadingUpdater : public KoLoadingShapeUpdater
{
public:
    enum ConnectionPosition
    {
        First,
        Second
    };

    KoConnectionShapeLoadingUpdater(KoConnectionShape *connectionShape, ConnectionPosition position);
    virtual ~KoConnectionShapeLoadingUpdater();

    virtual void update(KoShape *shape);

private:
    KoConnectionShape *m_connectionShape;
    ConnectionPosition m_position;
};

#endif /* KOCONNECTIONSHAPELOADINGUPDATER_H */
