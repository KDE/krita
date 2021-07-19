/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISRESOURCESINTERFACE_H
#define KISRESOURCESINTERFACE_H

#include "kritaresources_export.h"

#include <QScopedPointer>
#include <KoResource.h>
#include <QHash>

class QString;
class QByteArray;
class KisResourcesInterfacePrivate;

/**
 * @brief a provider-like interface class for accessing resource sources in Krita.
 *
 * Main differences to KoResourceServer and KisResourceModel:
 *
 *  1) It is a polymorphic class. Therefore, we are not obliged to pass
       a pointer to the global gui-only resource storage everywhere. Instead,
       we can create temporary storages and pass them to the strokes, when needed.

    2) The class doesn't depend on any specific resource types. Its baseline
       implementation of resourceInterface->source(type) returns a source
       working with KoResourceSP only. But when needed, the caller may request
       a typed version via resourcesInterface->source<KisBrush>(type). It
       will instantiate a templated wrapper **in the caller's** object file,
       not in kritaresources library. It solves linking problem:
       we have a source for KisBrush objects in kritaresources library,
       even though this library doesn't link kritabrush.

    3) Since strokes may have local storages for the resources, we operate
       with resources sources using shared pointers, not raw pointers.
 */
class KRITARESOURCES_EXPORT KisResourcesInterface
{
public:
    class KRITARESOURCES_EXPORT ResourceSourceAdapter
    {
    public:
        ResourceSourceAdapter();
        virtual ~ResourceSourceAdapter();

        virtual KoResourceSP resourceForFilename(const QString& filename) const = 0;
        virtual KoResourceSP resourceForName(const QString& name) const = 0;
        virtual KoResourceSP resourceForMD5(const QByteArray& md5) const = 0;
        virtual KoResourceSP fallbackResource() const = 0;

    private:
        Q_DISABLE_COPY(ResourceSourceAdapter);
    };

    template <typename T>
    class TypedResourceSourceAdapter
    {
    public:
        TypedResourceSourceAdapter(ResourceSourceAdapter *adapter)
            : m_source(adapter)
        {
        }

        QSharedPointer<T> resourceForFilename(const QString& filename) const
        {
            return m_source->resourceForFilename(filename).dynamicCast<T>();
        }

        QSharedPointer<T> resourceForName(const QString& name) const
        {
            return m_source->resourceForName(name).dynamicCast<T>();
        }

        QSharedPointer<T> resourceForMD5(const QByteArray& md5) const
        {
            return m_source->resourceForMD5(md5).dynamicCast<T>();
        }

        QSharedPointer<T> fallbackResource() const
        {
            return m_source->fallbackResource().dynamicCast<T>();
        }

    protected:
        ResourceSourceAdapter *m_source;
    };

public:
    KisResourcesInterface();
    virtual ~KisResourcesInterface();

    /**
     * A basic implementation that returns a source for a specific type
     * of the resource. Please take into account that this source object will
     * return un-casted resources of type KoResourceSP. If you want to have a
     * proper resource (in most of the cases), use a `server<T>(type)` instead.
     */
    ResourceSourceAdapter& source(const QString &type) const;

    /**
     * The main fetcher of resource source for resources of a specific type.
     *
     * Usage:
     *
     * \code{.cpp}
     *
     * auto source = resourceInterface->source<KisBrush>(ResourceType::Brushes);
     * KisBrushSP brush = source.resourceByMd5(md5)
     *
     * \endcode
     *
     */
    template<typename T>
    TypedResourceSourceAdapter<T> source(const QString &type) const {
        return TypedResourceSourceAdapter<T>(&this->source(type));
    }

protected:
    KisResourcesInterface(KisResourcesInterfacePrivate *dd);
    virtual ResourceSourceAdapter* createSourceImpl(const QString &type) const = 0;

protected:
    QScopedPointer<KisResourcesInterfacePrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(KisResourcesInterface)
};

using KisResourcesInterfaceSP = QSharedPointer<KisResourcesInterface>;

#endif // KISRESOURCESINTERFACE_H
