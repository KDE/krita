/*
   Copyright (c) 2006, 2011 Boudewijn Rempt (boud@valdyas.org)
   Copyright (C) 2007, 2009, 2010 Thomas Zander <zander@kde.org>
   Copyright (c) 2008 Carlos Licea <carlos.licea@kdemail.net>

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
   Boston, MA 02110-1301, USA.
 */
#ifndef KO_RESOURCEMANAGER_P_H
#define KO_RESOURCEMANAGER_P_H

#include <QObject>
#include <QSizeF>
#include <QHash>

#include "kritaflake_export.h"
#include <KoColor.h>
#include <KoUnit.h>
#include "KoDerivedResourceConverter.h"
#include "KoResourceUpdateMediator.h"


class KoShape;
class QVariant;

/**
 * @brief The KoResourceManager class provides access to the currently
 * active resources for a given canvas. It has nearly zilch to do with
 * the system that provides resources like brushes or palettes to the
 * application.
 */
class KRITAFLAKE_EXPORT KoResourceManager : public QObject
{
    Q_OBJECT
public:

    KoResourceManager() {}

    /**
     * Set a resource of any type.
     * @param key the integer key
     * @param value the new value for the key.
     * @see KoCanvasResourceProvider::CanvasResource KoDocumentResourceManager::DocumentResource
     */
    void setResource(int key, const QVariant &value);

    /**
     * Set a resource of type KoColor.
     * @param key the integer key
     * @param color the new value for the key.
     * @see KoCanvasResourceProvider::CanvasResource KoDocumentResourceManager::DocumentResource
     */
    void setResource(int key, const KoColor &color);

    /**
     * Set a resource of type KoShape*.
     * @param key the integer key
     * @param shape the new shape for the key.
     * @see KoCanvasResourceProvider::CanvasResource KoDocumentResourceManager::DocumentResource
     */
    void setResource(int key, KoShape *shape);

    /**
     * Set a resource of type KoUnit
     * @param key the integer key
     * @param unit the unit for the key.
     * @see KoCanvasResourceProvider::CanvasResource KoDocumentResourceManager::DocumentResource
     */
    void setResource(int key, const KoUnit &unit);

    /**
     * Returns a qvariant containing the specified resource or a standard one if the
     * specified resource does not exist.
     * @param key the key
     * @see KoCanvasResourceProvider::CanvasResource KoDocumentResourceManager::DocumentResource
     */
    QVariant resource(int key) const;

    /**
     * Return the resource determined by param key as a boolean.
     * @param key the identifying key for the resource
     * @see KoCanvasResourceProvider::CanvasResource KoDocumentResourceManager::DocumentResource
     */
    bool boolResource(int key) const;

    /**
     * Return the resource determined by param key as an integer.
     * @param key the identifying key for the resource
     * @see KoCanvasResourceProvider::CanvasResource KoDocumentResourceManager::DocumentResource
     */
    int intResource(int key) const;

    /**
     * Return the resource determined by param key as a KoColor.
     * @param key the identifying key for the resource
     * @see KoCanvasResourceProvider::CanvasResource KoDocumentResourceManager::DocumentResource
     */
    KoColor koColorResource(int key) const;

    /**
     * Return the resource determined by param key as a pointer to a KoShape.
     * @param key the identifying key for the resource
     * @see KoCanvasResourceProvider::CanvasResource KoDocumentResourceManager::DocumentResource
     */
    KoShape *koShapeResource(int key) const;

    /**
     * Return the resource determined by param key as a QString .
     * @param key the identifying key for the resource
     * @see KoCanvasResourceProvider::CanvasResource KoDocumentResourceManager::DocumentResource
     */
    QString stringResource(int key) const;

    /**
     * Return the resource determined by param key as a QSizeF.
     * @param key the identifying key for the resource
     * @see KoCanvasResourceProvider::CanvasResource KoDocumentResourceManager::DocumentResource
     */
    QSizeF sizeResource(int key) const;

    /**
     * Return the resource determined by param key as a KoUnit.
     * @param key the identifying key for the resource
     * @see KoCanvasResourceProvider::CanvasResource KoDocumentResourceManager::DocumentResource
     */
    KoUnit unitResource(int key) const;

    /**
     * Returns true if there is a resource set with the requested key.
     * @param key the identifying key for the resource
     * @see KoCanvasResourceProvider::CanvasResource KoDocumentResourceManager::DocumentResource
     */
    bool hasResource(int key) const;

    /**
     * Remove the resource with @p key from the provider.
     * @param key the key that will be used to remove the resource
     * @see KoCanvasResourceProvider::CanvasResource KoDocumentResourceManager::DocumentResource
     */
    void clearResource(int key);

    /**
     * Some of the resources may be "derived" from the others. For
     * example opacity, composite op and erase mode properties are
     * contained inside a paintop preset, so we need not create a
     * separate resource for them. Instead we created a derived resource,
     * that loads/saves values from/to another resource, but has its own
     * "resource changed" signal (via a different key).
     *
     * When a parent resource changes, the resource manager emits
     * update signals for all its derived resources.
     */
    void addDerivedResourceConverter(KoDerivedResourceConverterSP converter);

    /**
     * @return true if the resource with \p key is a derived resource
     *         (has a converter installed)
     *
     * @see addDerivedResourceConverter()
     */
    bool hasDerivedResourceConverter(int key);

    /**
     * Removes a derived resource converter. If you rty to add a
     * resource with \p key it will be treated as a usual resource.
     *
     * @see addDerivedResourceConverter()
     */
    void removeDerivedResourceConverter(int key);

    /**
     * Some resources can "mutate", that is their value doesn't change
     * (a pointer), whereas the contents changes. But we should still
     * notify all the derived resources subscribers that the resource
     * has changed. For that purpose we use a special mediator class
     * that connects the resource (which is not a QObject at all) and
     * the resource manager controls that connection.
     */
    void addResourceUpdateMediator(KoResourceUpdateMediatorSP mediator);

    /**
     * \see addResourceUpdateMediator()
     */
    bool hasResourceUpdateMediator(int key);

    /**
     * \see addResourceUpdateMediator()
     */
    void removeResourceUpdateMediator(int key);

Q_SIGNALS:
    void resourceChanged(int key, const QVariant &value);

private:
    void notifyResourceChanged(int key, const QVariant &value);
    void notifyDerivedResourcesChanged(int key, const QVariant &value);

private Q_SLOTS:
    void slotResourceInternalsChanged(int key);

private:
    KoResourceManager(const KoResourceManager&);
    KoResourceManager& operator=(const KoResourceManager&);

    QHash<int, QVariant> m_resources;

    QHash<int, KoDerivedResourceConverterSP> m_derivedResources;
    QMultiHash<int, KoDerivedResourceConverterSP> m_derivedFromSource;

    QHash<int, KoResourceUpdateMediatorSP> m_updateMediators;
};

#endif

