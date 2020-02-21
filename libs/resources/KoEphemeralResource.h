/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KOEPHEMERALRESOURCE_H
#define KOEPHEMERALRESOURCE_H

#include <KoResource.h>


/**
 * KoEphemeralResource is a type of resource that has no physical
 * representation on disk. Therefore, its load()/save() calls do
 * nothing.
 *
 * This type of resources is created directly by the factory.
 */
template<class ParentClass>
class KoEphemeralResource : public ParentClass
{
public:
    KoEphemeralResource()
        : ParentClass()
    {
    }

    KoEphemeralResource(const QString &arg)
        : ParentClass(arg)
    {
    }

    KoEphemeralResource(const KoEphemeralResource &rhs)
        : ParentClass(rhs)
    {
    }

    bool load(KisResourcesInterfaceSP resourcesInterface) override
    {
        Q_UNUSED(resourcesInterface);
        return false;
    }

    bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) override
    {
        Q_UNUSED(dev);
        Q_UNUSED(resourcesInterface);
        return false;
    }

    bool save() override
    {
        return false;
    }

    bool saveToDevice(QIODevice *dev) const override
    {
        Q_UNUSED(dev);
        return false;
    }
};

#endif // KOEPHEMERALRESOURCE_H
