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
//protected:
        friend class KisResourcesInterface;
        virtual QVector<KoResourceSP> resourcesForFilename(const QString& filename) const = 0;
        virtual QVector<KoResourceSP> resourcesForName(const QString& name) const = 0;
        virtual QVector<KoResourceSP> resourcesForMD5(const QString& md5) const = 0;
public:
        /**
         * @brief resource retrieves a resource, prefarably by md5, but with filename and name
         * as fallback for older files that do not store the md5sum. Note that if the resource is
         * not found by md5 if the md5 isn't empty, we do NOT then look by filename.
         * @return a resource, or 0 of the resource doesn't exist.
         */
        QVector<KoResourceSP> resources(const QString md5, const QString filename, const QString name) {

            QVector<KoResourceSP> resourceSet;

            if (!md5.isEmpty()) {
                resourceSet += resourcesForMD5(md5);
            }

            if (!filename.isEmpty()) {
                if (md5.isEmpty()) {
                    resourceSet += resourcesForFilename(filename);
                }
                else {
                    Q_FOREACH(KoResourceSP resource, resourcesForFilename(filename)) {
                        if (resource->md5Sum() == md5) {
                            resourceSet << resource;
                        }
                    }
                }
            }

            if (!name.isEmpty()) {
                if (md5.isEmpty()) {
                    resourceSet += resourcesForName(name);
                }
                else {
                    Q_FOREACH(KoResourceSP resource, resourcesForName(name)) {
                        if (resource->md5Sum() == md5) {
                            resourceSet << resource;
                        }
                    }
                }
            }

            std::sort(resourceSet.begin(), resourceSet.end());
            resourceSet.erase(std::unique(resourceSet.begin(), resourceSet.end()), resourceSet.end());

            return resourceSet;
        }
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
private:
        QVector<QSharedPointer<T>> resourcesForFilename(const QString& filename) const
        {
            QVector<QSharedPointer<T>> r;
            Q_FOREACH(KoResourceSP resource, m_source->resourcesForFilename(filename)) {
                r << resource.dynamicCast<T>();
            }
            return r;
        }

        QVector<QSharedPointer<T>> resourcesForName(const QString& name) const
        {
            QVector<QSharedPointer<T>> r;
            Q_FOREACH(KoResourceSP resource, m_source->resourcesForName(name)) {
                r << resource.dynamicCast<T>();
            }
            return r;
        }

        QVector<QSharedPointer<T>> resourcesForMD5(const QString& md5) const
        {
            QVector<QSharedPointer<T>> r;
            Q_FOREACH(KoResourceSP resource, m_source->resourcesForMD5(md5)) {
                r << resource.dynamicCast<T>();
            }
            return r;
        }
public:
        /**
         * @brief resource retrieves resources, prefarably by md5, but with filename and name
         * as fallback for older files that do not store the md5sum.
         * @return resources, or an empty list if none are found
         */
        QVector<QSharedPointer<T>> resources(const QString md5, const QString filename, const QString name)
        {
            QVector<QSharedPointer<T>> resourceSet;

            if (!md5.isEmpty()) {
                resourceSet << resourcesForMD5(md5);
            }

            if (!filename.isEmpty()) {
                resourceSet << resourcesForFilename(filename);
            }

            if (!name.isEmpty()) {
                resourceSet << resourcesForName(name);
            }

            std::sort(resourceSet.begin(), resourceSet.end());
            resourceSet.erase(std::unique(resourceSet.begin(), resourceSet.end()), resourceSet.end());

            return resourceSet;

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
