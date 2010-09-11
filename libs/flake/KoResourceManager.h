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
#ifndef KO_RESOURCEMANAGER_H
#define KO_RESOURCEMANAGER_H

#include <QObject>
#include <QList>
#include <QVariant>
#include <QSizeF>

#include "flake_export.h"

#include <KoColor.h>
#include <KoUnit.h>

class KoShape;
class KoLineBorder;
class KUndoStack;
class KoImageCollection;
class KoOdfDocument;

/**
 * The KoCanvasResource contains a set of per-canvas
 * properties, like current foreground color, current background
 * color and more.
 * \sa KoResourceManager KoCanvasBase::resourceManager()
 */
namespace KoCanvasResource
{

/**
 * This enum holds identifiers to the resources that can be stored in here.
 */
enum CanvasResource {
    ForegroundColor,    ///< The active forground color selected for this canvas.
    BackgroundColor,    ///< The active background color selected for this canvas.
    ActiveBorder,       ///< The active border selected for this canvas
    HandleRadius,       ///< The handle radius used for drawing handles of any kind
    GrabSensitivity,    ///< The grab sensitivity used for grabbing handles of any kind
    PageSize,           ///< The size of the (current) page in postscript points.
    Unit,               ///< The unit of this canvas
    CurrentPage,        ///< The current page number
    ActiveStyleType,    ///< the actual active style type see KoFlake::StyleType for valid values
    ActiveRange,        ///< The area where the rulers should show white
    KarbonStart = 1000,      ///< Base number for karbon specific values.
    KexiStart = 2000,        ///< Base number for kexi specific values.
    KivioStart = 3000,       ///< Base number for kivio specific values.
    KPlatoStart = 4000,      ///< Base number for kplato specific values.
    KPresenterStart = 5000,  ///< Base number for kpresenter specific values.
    KritaStart = 6000,       ///< Base number for krita specific values.
    KSpreadStart = 7000,     ///< Base number for kspread specific values.
    KWordStart = 8000,        ///< Base number for kword specific values.
    KoPageAppStart = 9000    ///< Base number for KoPageApp specific values.
};

}

/**
 * The KoDocumentResource contains a set of per-document
 * properties.
 * \sa KoResourceManager KoShapeCollection::resourceManager()
 */
namespace KoDocumentResource
{
/**
 * This enum holds identifiers to the resources that can be stored in here.
 */
enum DocumentResource {
    UndoStack,              ///< The document-wide undo stack (KUndoStack)
    ImageCollection,        ///< The KoImageCollection for the document
    OdfDocument,            ///< The document this canvas shows (KoOdfDocument)

    KarbonStart = 1000,      ///< Base number for karbon specific values.
    KexiStart = 2000,        ///< Base number for kexi specific values.
    KivioStart = 3000,       ///< Base number for kivio specific values.
    KPlatoStart = 4000,      ///< Base number for kplato specific values.
    KPresenterStart = 5000,  ///< Base number for kpresenter specific values.
    KritaStart = 6000,       ///< Base number for krita specific values.
    KSpreadStart = 7000,     ///< Base number for kspread specific values.
    KWordStart = 8000,       ///< Base number for kword specific values.
    KoPageAppStart = 9000,   ///< Base number for KoPageApp specific values.
    KoTextStart = 10000      ///< Base number for KoText specific values.
};
}

/**
 * The KoResourceManager contains a set of per-canvas
 * properties, like current foreground color, current background
 * color and more. All tools belonging to the current canvas are
 * notified when a Resource changes (is set).
 * The properties come from the KoCanvasResource::CanvasResource enum or the
 * KoDocumentResource::DocumentResource depending on which manager you got.
 * See KoCanvasBase::resourceManager KoShapeController::resourceManager
 *
 * The manager can contain all sorts of variable types and there are accessors
 * for the most common ones.  All variables are always stored inside a QVariant
 * instance internally and you can always just use the resource() method to get
 * that directly.
 * The way to store arbitairy data objects that are stored as pointers you can use
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
class FLAKE_EXPORT KoResourceManager : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor.
     * @param parent the parent QObject, used for memory management.
     */
    explicit KoResourceManager(QObject *parent = 0);
    ~KoResourceManager();

    /**
     * Set a resource of any type.
     * @param key the integer key
     * @param value the new value for the key.
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    void setResource(int key, const QVariant &value);

    /**
     * Set a resource of type KoColor.
     * @param key the integer key
     * @param color the new value for the key.
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    void setResource(int key, const KoColor &color);

    /**
     * Set a resource of type KoShape*.
     * @param key the integer key
     * @param id the new value for the key.
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    void setResource(int key, KoShape *shape);

    /**
     * Set a resource of type KoUnit
     * @param key the integer key
     * @param id the new value for the key.
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    void setResource(int key, const KoUnit &unit);

    /**
     * Returns a qvariant containing the specified resource or a standard one if the
     * specified resource does not exist.
     * @param key the key
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
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

    /// Sets the border resource
    void setActiveBorder(const KoLineBorder &border);

    /// Returns the border resource
    KoLineBorder activeBorder() const;

    /**
     * Return the resource determined by param key as a boolean.
     * @param key the indentifying key for the resource
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    bool boolResource(int key) const;

    /**
     * Return the resource determined by param key as an integer.
     * @param key the indentifying key for the resource
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    int intResource(int key) const;

    /**
     * Return the resource determined by param key as a KoColor.
     * @param key the indentifying key for the resource
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    KoColor koColorResource(int key) const;

    /**
     * Return the resource determined by param key as a pointer to a KoShape.
     * @param key the indentifying key for the resource
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    KoShape *koShapeResource(int key) const;

    /**
     * Return the resource determined by param key as a QString .
     * @param key the indentifying key for the resource
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    QString stringResource(int key) const;

    /**
     * Return the resource determined by param key as a QSizeF.
     * @param key the indentifying key for the resource
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    QSizeF sizeResource(int key) const;

    /**
     * Return the resource determined by param key as a KoUnit.
     * @param key the indentifying key for the resource
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    KoUnit unitResource(int key) const;

    /**
     * Returns true if there is a resource set with the requested key.
     * @param key the indentifying key for the resource
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    bool hasResource(int key) const;

    /**
     * Remove the resource with @p key from the provider.
     * @param key the key that will be used to remove the resource
     * There will be a signal emitted with a variable that will return true on QVariable::isNull();
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    void clearResource(int key);

    KUndoStack *undoStack() const;
    void setUndoStack(KUndoStack *undoStack);

    KoImageCollection *imageCollection() const;
    void setImageCollection(KoImageCollection *ic);

    KoOdfDocument *odfDocument() const;
    void setOdfDocument(KoOdfDocument *currentDocument);

signals:
    /**
     * This signal is emitted every time a resource is set that is either
     * new or different from the previous set value.
     * @param key the indentifying key for the resource
     * @param value the variants new value.
     * @see KoCanvasResource::CanvasResource KoDocumentResource::DocumentResource
     */
    void resourceChanged(int key, const QVariant &value);

private:
    KoResourceManager(const KoResourceManager&);
    KoResourceManager& operator=(const KoResourceManager&);

    class Private;
    Private *const d;
};

#endif
