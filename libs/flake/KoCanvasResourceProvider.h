/*
   SPDX-FileCopyrightText: 2006, 2011 Boudewijn Rempt (boud@valdyas.org)
   SPDX-FileCopyrightText: 2007, 2009, 2010 Thomas Zander <zander@kde.org>
   SPDX-FileCopyrightText: 2008 Carlos Licea <carlos.licea@kdemail.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KO_CANVASRESOURCEMANAGER_H
#define KO_CANVASRESOURCEMANAGER_H

#include <QObject>

#include "kritaflake_export.h"
#include "KoDerivedResourceConverter.h"
#include "KoResourceUpdateMediator.h"
#include "KoActiveCanvasResourceDependency.h"
#include <KoCanvasResourcesIds.h>

template<class T> class QSharedPointer;
class KoCanvasResourcesInterface;
using KoCanvasResourcesInterfaceSP = QSharedPointer<KoCanvasResourcesInterface>;

class KoShape;
class KoShapeStroke;
class KoColor;
class KoUnit;

class QVariant;
class QSizeF;

/**
 * The KoCanvasResourceProvider contains a set of per-canvas
 * properties, like current foreground color, current background
 * color and more. All tools belonging to the current canvas are
 * notified when a Resource changes (is set).
 *
 * The manager can contain all sorts of variable types and there are accessors
 * for the most common ones.  All variables are always stored inside a QVariant
 * instance internally and you can always just use the resource() method to get
 * that directly.
 * The way to store arbitrary data objects that are stored as pointers you can use
 * the following code snippets;
 * @code
 *  QVariant variant;
 *  variant.setValue<void*>(textShapeData->document());
 *  resourceManager->setResource(KoText::CurrentTextDocument, variant);
 *  // and get it out again.
 *  QVariant var = resourceManager->resource(KoText::CurrentTextDocument);
 *  document = static_cast<QTextDocument*>(var.value<void*>());
 * @endcode
 */
class KRITAFLAKE_EXPORT KoCanvasResourceProvider : public QObject
{
    Q_OBJECT

public:
    enum ApplicationSpecial {
        NoSpecial = 0,
        NoAdvancedText = 1
    };

    /**
     * Constructor.
     * @param parent the parent QObject, used for memory management.
     */
    explicit KoCanvasResourceProvider(QObject *parent = 0);
    ~KoCanvasResourceProvider() override;

public Q_SLOTS:
    /**
     * Set a resource of any type.
     * @param key the integer key
     * @param value the new value for the key.
     * @see KoCanvasResource::CanvasResourceId
     */
    void setResource(int key, const QVariant &value);

    /**
     * Set a resource of type KoColor.
     * @param key the integer key
     * @param color the new value for the key.
     * @see KoCanvasResource::CanvasResourceId
     */
    void setResource(int key, const KoColor &color);

    /**
     * Set a resource of type KoShape*.
     * @param key the integer key
     * @param id the new value for the key.
     * @see KoCanvasResource::CanvasResourceId
     */
    void setResource(int key, KoShape *shape);

    /**
     * Set a resource of type KoUnit
     * @param key the integer key
     * @param id the new value for the key.
     * @see KoCanvasResource::CanvasResourceId
     */
    void setResource(int key, const KoUnit &unit);

public:
    /**
     * Returns a qvariant containing the specified resource or a standard one if the
     * specified resource does not exist.
     * @param key the key
     * @see KoCanvasResource::CanvasResourceId
     */
    QVariant resource(int key) const;

    /**
     * Set the foregroundColor resource.
     * @param color the new foreground color
     */
    void setForegroundColor(const KoColor &color);

    /**
     * Return the foregroundColor
     */
    KoColor foregroundColor() const;

    /**
     * Set the backgroundColor resource.
     * @param color the new background color
     */
    void setBackgroundColor(const KoColor &color);
    /**
     * Return the backgroundColor
     */
    KoColor backgroundColor() const;

    /**
     * Return the resource determined by param key as a boolean.
     * @param key the identifying key for the resource
     * @see KoCanvasResource::CanvasResourceId
     */
    bool boolResource(int key) const;

    /**
     * Return the resource determined by param key as an integer.
     * @param key the identifying key for the resource
     * @see KoCanvasResource::CanvasResourceId
     */
    int intResource(int key) const;

    /**
     * Return the resource determined by param key as a KoColor.
     * @param key the identifying key for the resource
     * @see KoCanvasResource::CanvasResourceId
     */
    KoColor koColorResource(int key) const;

    /**
     * Return the resource determined by param key as a pointer to a KoShape.
     * @param key the identifying key for the resource
     * @see KoCanvasResource::CanvasResourceId
     */
    KoShape *koShapeResource(int key) const;

    /**
     * Return the resource determined by param key as a QString .
     * @param key the identifying key for the resource
     * @see KoCanvasResource::CanvasResourceId
     */
    QString stringResource(int key) const;

    /**
     * Return the resource determined by param key as a QSizeF.
     * @param key the identifying key for the resource
     * @see KoCanvasResource::CanvasResourceId
     */
    QSizeF sizeResource(int key) const;

    /**
     * Return the resource determined by param key as a KoUnit.
     * @param key the identifying key for the resource
     * @see KoCanvasResource::CanvasResourceId
     */
    KoUnit unitResource(int key) const;

    /**
     * Returns true if there is a resource set with the requested key.
     * @param key the identifying key for the resource
     * @see KoCanvasResource::CanvasResourceId
     */
    bool hasResource(int key) const;

    /**
     * Remove the resource with @p key from the provider.
     * @param key the key that will be used to remove the resource
     * There will be a signal emitted with a variable that will return true on QVariable::isNull();
     * @see KoCanvasResource::CanvasResourceId
     */
    void clearResource(int key);

    /**
     * @see KoResourceManager::addDerivedResourceConverter()
     */
    void addDerivedResourceConverter(KoDerivedResourceConverterSP converter);

    /**
     * @see KoResourceManager::hasDerivedResourceConverter()
     */
    bool hasDerivedResourceConverter(int key);

    /**
     * @see KoResourceManager::removeDerivedResourceConverter()
     */
    void removeDerivedResourceConverter(int key);

    /**
     * @see KoResourceManager::addResourceUpdateMediator
     */
    void addResourceUpdateMediator(KoResourceUpdateMediatorSP mediator);

    /**
     * @see KoResourceManager::hasResourceUpdateMediator
     */
    bool hasResourceUpdateMediator(int key);

    /**
     * @see KoResourceManager::removeResourceUpdateMediator
     */
    void removeResourceUpdateMediator(int key);

    /**
     * @see KoResourceManager::addActiveCanvasResourceDependency
     */
    void addActiveCanvasResourceDependency(KoActiveCanvasResourceDependencySP dep);

    /**
     * @see KoResourceManager::hasActiveCanvasResourceDependency
     */
    bool hasActiveCanvasResourceDependency(int sourceKey, int targetKey) const;

    /**
     * @see KoResourceManager::removeActiveCanvasResourceDependency
     */
    void removeActiveCanvasResourceDependency(int sourceKey, int targetKey);

    /**
     * An interface for accessing this KoCanvasResourceProvider via
     * KoCanvasResourcesInterface.
     */
    KoCanvasResourcesInterfaceSP canvasResourcesInterface() const;

Q_SIGNALS:
    /**
     * This signal is emitted every time a resource is set that is either
     * new or different from the previous set value.
     * @param key the identifying key for the resource
     * @param value the variants new value.
     * @see KoCanvasResource::CanvasResourceId
     */
    void canvasResourceChanged(int key, const QVariant &value);

    /**
     * This signal is emitted every time a resource is attempted to be
     * changed. The this signal is emitted even when the new value of
     * the resource is the same as the current value. This method is called
     * **before** the actual change has happended at the resource manager.
     * @param key the identifying key for the resource
     * @param value the variants new value.
     * @see KoCanvasResource::CanvasResourceId
     */
    void canvasResourceChangeAttempted(int key, const QVariant &value);

private:
    KoCanvasResourceProvider(const KoCanvasResourceProvider&);
    KoCanvasResourceProvider& operator=(const KoCanvasResourceProvider&);

    class Private;
    Private *const d;
};

#endif
