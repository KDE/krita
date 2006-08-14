/***************************************************************************
 * childreninterface.h
 * This file is part of the KDE project
 * copyright (C)2004-2005 by Sebastian Sauer (mail@dipe.org)
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

#ifndef KROSS_CHILDRENINTERFACE_H
#define KROSS_CHILDRENINTERFACE_H

#include <QString>
#include <QHash>
#include <QObject>
#include <koffice_export.h>

#include "krossconfig.h"
//#include "object.h"

namespace Kross {

    /**
     * Interface for managing \a Object collections.
     */
    class KROSS_EXPORT ChildrenInterface
    {
        public:

            inline void addObject(QObject* object, const QString& name = QString::null) {
                //Object* obj = new Object(object, name);
                //m_objects.insert(obj->objectName(), Object::Ptr(obj));
                m_objects.insert(name.isNull() ? object->objectName() : name, object);
            }

            inline bool hasObject(const QString& name) const {
                return m_objects.contains(name);
            }

            inline QObject* object(const QString& name) const {
                return m_objects.value(name);
            }

            inline QHash< QString, QObject* > objects() const {
                return m_objects;
            }

        private:
            QHash< QString, QObject* > m_objects;
    };

}

#endif

