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
#ifndef KO_CANVASRESOURCEMANAGER_H
#define KO_CANVASRESOURCEMANAGER_H

#include <QObject>
#include <QList>
#include <QVariant>
#include <QSizeF>

#include "flake_export.h"

#include <KoColor.h>
#include <KoUnit.h>

class KoShape;
class KoShapeStroke;
class KUndo2Stack;
class KoImageCollection;
class KoOdfDocument;

/**
 * The KoCanvasResourceManager contains a set of per-canvas
 * properties, like current foreground color, current background
 * color and more. All tools belonging to the current canvas are
 * notified when a Resource changes (is set).
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
class FLAKE_EXPORT KoCanvasResourceManager : public QObject
{
    Q_OBJECT

public:

    /**
     * This enum holds identifiers to the resources that can be stored in here.
     */
    enum CanvasResource {
        ForegroundColor,    ///< The active forground color selected for this canvas.
        BackgroundColor,    ///< The active background color selected for this canvas.
        ActiveStroke,       ///< The active stroke selected for this canvas
        PageSize,           ///< The size of the (current) page in postscript points.
        Unit,               ///< The unit of this canvas
        CurrentPage,        ///< The current page number
        ActiveStyleType,    ///< the actual active style type see KoFlake::StyleType for valid values
        ActiveRange,        ///< The area where the rulers should show white
        ShowTextShapeOutlines,     ///< Paint of text shape outlines ?
        ShowFormattingCharacters,  ///< Paint of formatting characters ?
        ShowTableBorders,  ///< Paint of table borders (when not really there) ?
        KarbonStart = 1000,      ///< Base number for Karbon specific values.
        KexiStart = 2000,        ///< Base number for Kexi specific values.
        FlowStart = 3000,        ///< Base number for Flow specific values.
        PlanStart = 4000,        ///< Base number for Plan specific values.
        StageStart = 5000,       ///< Base number for Stage specific values.
        KritaStart = 6000,       ///< Base number for Krita specific values.
        SheetsStart = 7000,      ///< Base number for Sheets specific values.
        WordsStart = 8000,       ///< Base number for Words specific values.
        KoPageAppStart = 9000    ///< Base number for KoPageApp specific values.
    };

    /**
     * Constructor.
     * @param parent the parent QObject, used for memory management.
     */
    explicit KoCanvasResourceManager(QObject *parent = 0);
    virtual ~KoCanvasResourceManager();

public slots:
    /**
     * Set a resource of any type.
     * @param key the integer key
     * @param value the new value for the key.
     * @see KoCanvasResourceManager::CanvasResource
     */
    void setResource(int key, const QVariant &value);

    /**
     * Set a resource of type KoColor.
     * @param key the integer key
     * @param color the new value for the key.
     * @see KoCanvasResourceManager::CanvasResource
     */
    void setResource(int key, const KoColor &color);

    /**
     * Set a resource of type KoShape*.
     * @param key the integer key
     * @param id the new value for the key.
     * @see KoCanvasResourceManager::CanvasResource
     */
    void setResource(int key, KoShape *shape);

    /**
     * Set a resource of type KoUnit
     * @param key the integer key
     * @param id the new value for the key.
     * @see KoCanvasResourceManager::CanvasResource
     */
    void setResource(int key, const KoUnit &unit);

public:
    /**
     * Returns a qvariant containing the specified resource or a standard one if the
     * specified resource does not exist.
     * @param key the key
     * @see KoCanvasResourceManager::CanvasResource
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

    /// Sets the stroke resource
    void setActiveStroke(const KoShapeStroke &stroke);

    /// Returns the stroke resource
    KoShapeStroke activeStroke() const;

    /**
     * Return the resource determined by param key as a boolean.
     * @param key the indentifying key for the resource
     * @see KoCanvasResourceManager::CanvasResource
     */
    bool boolResource(int key) const;

    /**
     * Return the resource determined by param key as an integer.
     * @param key the indentifying key for the resource
     * @see KoCanvasResourceManager::CanvasResource
     */
    int intResource(int key) const;

    /**
     * Return the resource determined by param key as a KoColor.
     * @param key the indentifying key for the resource
     * @see KoCanvasResourceManager::CanvasResource
     */
    KoColor koColorResource(int key) const;

    /**
     * Return the resource determined by param key as a pointer to a KoShape.
     * @param key the indentifying key for the resource
     * @see KoCanvasResourceManager::CanvasResource
     */
    KoShape *koShapeResource(int key) const;

    /**
     * Return the resource determined by param key as a QString .
     * @param key the indentifying key for the resource
     * @see KoCanvasResourceManager::CanvasResource
     */
    QString stringResource(int key) const;

    /**
     * Return the resource determined by param key as a QSizeF.
     * @param key the indentifying key for the resource
     * @see KoCanvasResourceManager::CanvasResource
     */
    QSizeF sizeResource(int key) const;

    /**
     * Return the resource determined by param key as a KoUnit.
     * @param key the indentifying key for the resource
     * @see KoCanvasResourceManager::CanvasResource
     */
    KoUnit unitResource(int key) const;

    /**
     * Returns true if there is a resource set with the requested key.
     * @param key the indentifying key for the resource
     * @see KoCanvasResourceManager::CanvasResource
     */
    bool hasResource(int key) const;

    /**
     * Remove the resource with @p key from the provider.
     * @param key the key that will be used to remove the resource
     * There will be a signal emitted with a variable that will return true on QVariable::isNull();
     * @see KoCanvasResourceManager::CanvasResource
     */
    void clearResource(int key);

    KUndo2Stack *undoStack() const;
    void setUndoStack(KUndo2Stack *undoStack);


signals:
    /**
     * This signal is emitted every time a resource is set that is either
     * new or different from the previous set value.
     * @param key the indentifying key for the resource
     * @param value the variants new value.
     * @see KoCanvasResourceManager::CanvasResource
     */
    void resourceChanged(int key, const QVariant &value);

private:
    KoCanvasResourceManager(const KoCanvasResourceManager&);
    KoCanvasResourceManager& operator=(const KoCanvasResourceManager&);

    class Private;
    Private *const d;
};

#endif
