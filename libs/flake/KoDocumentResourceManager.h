/*
   Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
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
#ifndef KO_DOCUMENTRESOURCEMANAGER_H
#define KO_DOCUMENTRESOURCEMANAGER_H

#include <QObject>

#include "kritaflake_export.h"

class KoShape;
class KUndo2Stack;
class KoImageCollection;
class KoDocumentBase;
class KoShapeController;
class KoColor;
class KoUnit;

class QVariant;
class QSizeF;

/**
 * The KoResourceManager contains a set of per-canvas <i>or</i> per-document
 * properties, like current foreground color, current background
 * color and more. All tools belonging to the current canvas are
 * notified when a Resource changes (is set).
 *
 * The properties come from the KoDocumentResourceManager::DocumentResource
 * See KoShapeController::resourceManager
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
class KRITAFLAKE_EXPORT KoDocumentResourceManager : public QObject
{
    Q_OBJECT

public:

    /**
     * This enum holds identifiers to the resources that can be stored in here.
     */
enum DocumentResource {
    UndoStack,              ///< The document-wide undo stack (KUndo2Stack)
    ImageCollection,        ///< The KoImageCollection for the document
    OdfDocument,            ///< The document this canvas shows (KoDocumentBase)
    HandleRadius,           ///< The handle radius used for drawing handles of any kind
    GrabSensitivity,        ///< The grab sensitivity used for grabbing handles of any kind
    MarkerCollection,       ///< The collection holding all markers
    GlobalShapeController,  ///< The KoShapeController for the document
    DocumentResolution,     ///< Pixels-per-inch resoluton of the document
    DocumentRectInPixels,   ///< Bounds of the document in pixels

    KarbonStart = 1000,      ///< Base number for Karbon specific values.
    KexiStart = 2000,        ///< Base number for Kexi specific values.
    FlowStart = 3000,        ///< Base number for Flow specific values.
    PlanStart = 4000,        ///< Base number for Plan specific values.
    StageStart = 5000,       ///< Base number for Stage specific values.
    KritaStart = 6000,       ///< Base number for Krita specific values.
    SheetsStart = 7000,      ///< Base number for Sheets specific values.
    WordsStart = 8000,       ///< Base number for Words specific values.
    KoPageAppStart = 9000,   ///< Base number for KoPageApp specific values.
    KoTextStart = 10000      ///< Base number for KoText specific values.
};


    /**
     * Constructor.
     * @param parent the parent QObject, used for memory management.
     */
    explicit KoDocumentResourceManager(QObject *parent = 0);
    ~KoDocumentResourceManager() override;

    /**
     * Set a resource of any type.
     * @param key the integer key
     * @param value the new value for the key.
     * @see  KoDocumentResourceManager::DocumentResource
     */
    void setResource(int key, const QVariant &value);

    /**
     * Set a resource of type KoColor.
     * @param key the integer key
     * @param color the new value for the key.
     * @see  KoDocumentResourceManager::DocumentResource
     */
    void setResource(int key, const KoColor &color);

    /**
     * Set a resource of type KoShape*.
     * @param key the integer key
     * @param id the new value for the key.
     * @see  KoDocumentResourceManager::DocumentResource
     */
    void setResource(int key, KoShape *shape);

    /**
     * Set a resource of type KoUnit
     * @param key the integer key
     * @param id the new value for the key.
     * @see  KoDocumentResourceManager::DocumentResource
     */
    void setResource(int key, const KoUnit &unit);

    /**
     * Returns a qvariant containing the specified resource or a standard one if the
     * specified resource does not exist.
     * @param key the key
     * @see  KoDocumentResourceManager::DocumentResource
     */
    QVariant resource(int key) const;

    /**
     * Return the resource determined by param key as a boolean.
     * @param key the identifying key for the resource
     * @see  KoDocumentResourceManager::DocumentResource
     */
    bool boolResource(int key) const;

    /**
     * Return the resource determined by param key as an integer.
     * @param key the identifying key for the resource
     * @see  KoDocumentResourceManager::DocumentResource
     */
    int intResource(int key) const;

    /**
     * Return the resource determined by param key as a KoColor.
     * @param key the identifying key for the resource
     * @see  KoDocumentResourceManager::DocumentResource
     */
    KoColor koColorResource(int key) const;

    /**
     * Return the resource determined by param key as a pointer to a KoShape.
     * @param key the identifying key for the resource
     * @see  KoDocumentResourceManager::DocumentResource
     */
    KoShape *koShapeResource(int key) const;

    /**
     * Return the resource determined by param key as a QString .
     * @param key the identifying key for the resource
     * @see  KoDocumentResourceManager::DocumentResource
     */
    QString stringResource(int key) const;

    /**
     * Return the resource determined by param key as a QSizeF.
     * @param key the identifying key for the resource
     * @see  KoDocumentResourceManager::DocumentResource
     */
    QSizeF sizeResource(int key) const;

    /**
     * Return the resource determined by param key as a KoUnit.
     * @param key the identifying key for the resource
     * @see  KoDocumentResourceManager::DocumentResource
     */
    KoUnit unitResource(int key) const;

    /**
     * Returns true if there is a resource set with the requested key.
     * @param key the identifying key for the resource
     * @see  KoDocumentResourceManager::DocumentResource
     */
    bool hasResource(int key) const;

    /**
     * Remove the resource with @p key from the provider.
     * @param key the key that will be used to remove the resource
     * There will be a signal emitted with a variable that will return true on QVariable::isNull();
     * @see  KoDocumentResourceManager::DocumentResource
     */
    void clearResource(int key);

        /**
     * Tools that provide a handle for controlling the content that the tool can edit can
     * use this property to alter the radius that a circular handle should have on screen.
     * @param handleSize the radius in pixels.
     */
    void setHandleRadius(int handleSize);
    /// Returns the actual handle radius
    int handleRadius() const;

    /**
     * Tools that are used to grab handles or similar with the mouse
     * should use this value to determine if the mouse is near enough
     * @param grabSensitivity the grab sensitivity in pixels
     */
    void setGrabSensitivity(int grabSensitivity);
    /// Returns the actual grab sensitivity
    int grabSensitivity() const;

    KUndo2Stack *undoStack() const;
    void setUndoStack(KUndo2Stack *undoStack);

    KoImageCollection *imageCollection() const;
    void setImageCollection(KoImageCollection *ic);

    KoDocumentBase *odfDocument() const;
    void setOdfDocument(KoDocumentBase *currentDocument);

    qreal documentResolution() const;
    QRectF documentRectInPixels() const;

    /**
     * TODO: remove these methods after legacy ODF text shape is removed.
     * New code must use documentResolution() and documentRectInPixels()
     * instead.
     */
    Q_DECL_DEPRECATED KoShapeController *globalShapeController() const;
    Q_DECL_DEPRECATED void setGlobalShapeController(KoShapeController *globalShapeController);

Q_SIGNALS:
    /**
     * This signal is emitted every time a resource is set that is either
     * new or different from the previous set value.
     * @param key the identifying key for the resource
     * @param value the variants new value.
     * @see KoDocumentResourceManager::DocumentResource
     */
    void resourceChanged(int key, const QVariant &value);

private:
    KoDocumentResourceManager(const KoDocumentResourceManager&);
    KoDocumentResourceManager& operator=(const KoDocumentResourceManager&);

    class Private;
    Private *const d;
};

#endif
