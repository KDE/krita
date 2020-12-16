/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOEPHEMERALRESOURCE_H
#define KOEPHEMERALRESOURCE_H

#include <KoResource.h>


/**
 * KoEphemeralResource is a type of resource that has no physical
 * representation on disk. Therefore, its load()/save() calls do
 * nothing.
 *
 * This type of resources is created directly by the corresponding
 * factory or other object (e.g. KisAutoBrushFactory).
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
