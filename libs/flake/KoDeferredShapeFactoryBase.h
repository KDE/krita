/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Boudewijn Rempt (boud@valdyas.org)
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KODEFERREDSHAPEFACTORYBASE_H
#define KODEFERREDSHAPEFACTORYBASE_H

#include "kritaflake_export.h"

#include <QObject>

class KoShape;
class KoDocumentResourceManager;
class KoProperties;

class QString;

/**
 * A factory for KoShape objects. This factory differs from the public KoShapeFactorBase
 * class that this class really creates the shape; it's the plugin entry point for the
 * actually shape plugins.
 */
class KRITAFLAKE_EXPORT KoDeferredShapeFactoryBase : public QObject
{
    Q_OBJECT
public:

    explicit KoDeferredShapeFactoryBase(QObject *parent);

    ~KoDeferredShapeFactoryBase() override;

    virtual QString deferredPluginName() = 0;

    /**
     * This method should be implemented by factories to create a shape that the user
     * gets when doing a base insert. For example from a script.  The created shape
     * should have its values set to good defaults that the user can then adjust further if
     * needed.  Including the KoShape:setShapeId(), with the Id from this factory
     * The default shape position is not relevant, it will be moved by the caller.
     * @param documentResources the resources manager that has all the document wide
     *      resources which can be used to create the object.
     * @return a new shape
     * @see createShape() newDocumentResourceManager()
     */
    virtual KoShape *createDefaultShape(KoDocumentResourceManager *documentResources = 0) const = 0;

    /**
     * This method should be implemented by factories to create a shape based on a set of
     * properties that are specifically made for this shape-type.
     * This method should also set this factories shapeId on the shape using KoShape::setShapeId()
     * The default implementation just ignores 'params' and calls createDefaultShape()
     * @return a new shape
     * @param params the properties object is the same as added in the addTemplate() call
     * @param documentResources the resources manager that has all the document wide
     *      resources which can be used to create the object.
     * @see createDefaultShape() newDocumentResourceManager()
     * @see KoShapeTemplate::properties
     */
    virtual KoShape *createShape(const KoProperties *params, KoDocumentResourceManager *documentResources = 0) const;

};

#endif
