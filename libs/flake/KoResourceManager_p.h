/*
   SPDX-FileCopyrightText: 2006, 2011 Boudewijn Rempt (boud@valdyas.org)
   SPDX-FileCopyrightText: 2007, 2009, 2010 Thomas Zander <zander@kde.org>
   SPDX-FileCopyrightText: 2008 Carlos Licea <carlos.licea@kdemail.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
#include "KoActiveCanvasResourceDependency.h"


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
     * @see KoCanvasResource::CanvasResourceId KoDocumentResourceManager::DocumentResource
     */
    void setResource(int key, const QVariant &value);

    /**
     * Set a resource of type KoColor.
     * @param key the integer key
     * @param color the new value for the key.
     * @see KoCanvasResource::CanvasResourceId KoDocumentResourceManager::DocumentResource
     */
    void setResource(int key, const KoColor &color);

    /**
     * Set a resource of type KoShape*.
     * @param key the integer key
     * @param shape the new shape for the key.
     * @see KoCanvasResource::CanvasResourceId KoDocumentResourceManager::DocumentResource
     */
    void setResource(int key, KoShape *shape);

    /**
     * Set a resource of type KoUnit
     * @param key the integer key
     * @param unit the unit for the key.
     * @see KoCanvasResource::CanvasResourceId KoDocumentResourceManager::DocumentResource
     */
    void setResource(int key, const KoUnit &unit);

    /**
     * Returns a qvariant containing the specified resource or a standard one if the
     * specified resource does not exist.
     * @param key the key
     * @see KoCanvasResource::CanvasResourceId KoDocumentResourceManager::DocumentResource
     */
    QVariant resource(int key) const;

    /**
     * Return the resource determined by param key as a boolean.
     * @param key the identifying key for the resource
     * @see KoCanvasResource::CanvasResourceId KoDocumentResourceManager::DocumentResource
     */
    bool boolResource(int key) const;

    /**
     * Return the resource determined by param key as an integer.
     * @param key the identifying key for the resource
     * @see KoCanvasResource::CanvasResourceId KoDocumentResourceManager::DocumentResource
     */
    int intResource(int key) const;

    /**
     * Return the resource determined by param key as a KoColor.
     * @param key the identifying key for the resource
     * @see KoCanvasResource::CanvasResourceId KoDocumentResourceManager::DocumentResource
     */
    KoColor koColorResource(int key) const;

    /**
     * Return the resource determined by param key as a pointer to a KoShape.
     * @param key the identifying key for the resource
     * @see KoCanvasResource::CanvasResourceId KoDocumentResourceManager::DocumentResource
     */
    KoShape *koShapeResource(int key) const;

    /**
     * Return the resource determined by param key as a QString .
     * @param key the identifying key for the resource
     * @see KoCanvasResource::CanvasResourceId KoDocumentResourceManager::DocumentResource
     */
    QString stringResource(int key) const;

    /**
     * Return the resource determined by param key as a QSizeF.
     * @param key the identifying key for the resource
     * @see KoCanvasResource::CanvasResourceId KoDocumentResourceManager::DocumentResource
     */
    QSizeF sizeResource(int key) const;

    /**
     * Return the resource determined by param key as a KoUnit.
     * @param key the identifying key for the resource
     * @see KoCanvasResource::CanvasResourceId KoDocumentResourceManager::DocumentResource
     */
    KoUnit unitResource(int key) const;

    /**
     * Returns true if there is a resource set with the requested key.
     * @param key the identifying key for the resource
     * @see KoCanvasResource::CanvasResourceId KoDocumentResourceManager::DocumentResource
     */
    bool hasResource(int key) const;

    /**
     * Remove the resource with @p key from the provider.
     * @param key the key that will be used to remove the resource
     * @see KoCanvasResource::CanvasResourceId KoDocumentResourceManager::DocumentResource
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

    /**
     * Add a dependency object that will link two active resources. When the
     * target of the dependency changes, the source will get a notification
     * about the change.
     */
    void addActiveCanvasResourceDependency(KoActiveCanvasResourceDependencySP dep);

    /**
     * @return true if specified dependency exists
     *
     * \see addActiveCanvasResourceDependency
     */
    bool hasActiveCanvasResourceDependency(int sourceKey, int targetKey) const;

    /**
     * Remove specified dependency
     *
     * \see addActiveCanvasResourceDependency
     */
    void removeActiveCanvasResourceDependency(int sourceKey, int targetKey);

Q_SIGNALS:
    void resourceChanged(int key, const QVariant &value);
    void resourceChangeAttempted(int key, const QVariant &value);

private:
    void notifyResourceChanged(int key, const QVariant &value);
    void notifyDerivedResourcesChanged(int key, const QVariant &value);

    void notifyResourceChangeAttempted(int key, const QVariant &value);
    void notifyDerivedResourcesChangeAttempted(int key, const QVariant &value);

    void notifyDependenciesAboutTargetChange(int targetKey, const QVariant &value);

private Q_SLOTS:
    void slotResourceInternalsChanged(int key);

private:
    KoResourceManager(const KoResourceManager&);
    KoResourceManager& operator=(const KoResourceManager&);

    QHash<int, QVariant> m_resources;

    QHash<int, KoDerivedResourceConverterSP> m_derivedResources;
    QMultiHash<int, KoDerivedResourceConverterSP> m_derivedFromSource;

    QMultiHash<int, KoActiveCanvasResourceDependencySP> m_dependencyFromSource;
    QMultiHash<int, KoActiveCanvasResourceDependencySP> m_dependencyFromTarget;

    QHash<int, KoResourceUpdateMediatorSP> m_updateMediators;
};

#endif

