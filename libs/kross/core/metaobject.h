/***************************************************************************
 * metaobject.h
 * This file is part of the KDE project
 * copyright (C)2004-2006 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KROSS_METAOBJECT_H
#define KROSS_METAOBJECT_H

//#include <QString>
//#include <QStringList>
//#include <QMap>
//#include <QVariant>
#include <QMetaType>
//#include <QObject>

#include "krossconfig.h"

namespace Kross {

    class Object;

    struct MetaObject : public QMetaObject
    {
        public:
            MetaObject(const Object* object);
            ~MetaObject();

            //void addSlot(QObject* sender, QByteArray slot);
            //void removeSlot(QObject* sender, QByteArray slot);

            void attachObject(QObject* object);
            void rebuild();

        private:
            class Private;
            Private* const dptr;
    };
}

#endif

