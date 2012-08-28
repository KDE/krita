/* This file is part of the KDE project
 * Copyright (c) 2010 Boudewijn Rempt (boud@valdyas.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KODEFERREDSHAPEFACTORYBASE_H
#define KODEFERREDSHAPEFACTORYBASE_H

#include <QStringList>
#include <QString>
#include <QWidget>
#include <QList>

#include "flake_export.h"

#include <KoShapeFactoryBase.h>

class KoShape;
class KoDocumentResourceManager;
class KoProperties;

/**
 * A factory for KoShape objects. This factory differs from the public KoShapeFactorBase
 * class that this class really creates the shape; it's the plugin entry point for the
 * actualy shape plugins.
 */
class FLAKE_EXPORT KoDeferredShapeFactoryBase : public QObject
{
    Q_OBJECT
public:

    KoDeferredShapeFactoryBase(QObject *parent);

    virtual ~KoDeferredShapeFactoryBase();

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
