/*  This file is part of the KDE project

    Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef KORESOURCESERVERPOLICIES_H
#define KORESOURCESERVERPOLICIES_H

#include "kritawidgets_export.h"

class KoResource;

template <class T> struct PointerStoragePolicy
{
    typedef T* PointerType;
    static inline void deleteResource(PointerType resource) {
        delete resource;
    }
    static inline KoResource* toResourcePointer(PointerType resource) {
        return resource;
    }
};

template <class SharedPointer> struct SharedPointerStoragePolicy
{
    typedef SharedPointer PointerType;
    static inline void deleteResource(PointerType resource) {
        Q_UNUSED(resource);
    }
    static inline KoResource* toResourcePointer(PointerType resource) {
        return resource.data();
    }
};

#endif // KORESOURCESERVERPOLICIES_H
