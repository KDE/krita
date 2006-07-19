/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KO_SHAPE_CONFIG_FACTORY_
#define _KO_SHAPE_CONFIG_FACTORY_

#include <koffice_export.h>

#include <QString>

class KoShape;
class KoShapeConfigWidgetBase;

/**
 * A factory that creates config panels (widgets) for just a created shape.
 * The KoCreateShapesTool is able to show a number of configuration panels after
 * it created a shape via user interaction.  Each shape configuration panel type
 * has its own factory, which will inherit from this class.
 * @see KoShapeFactory::panelFactories()
 * @see KoShapeConfigWidgetBase
 */
class FLAKE_EXPORT KoShapeConfigFactory {
public:
    /// default constructor
    KoShapeConfigFactory() {}
    virtual ~KoShapeConfigFactory() {}

    /**
     * create a new config widget, intialized with the param shape
     * @param shape the shape that will be configured in the config widget.
     * @see KoShapeConfigWidgetBase::open()
     */
    virtual KoShapeConfigWidgetBase *createConfigWidget(KoShape *shape) = 0;
    /// return the (translated) name of this configuration
    virtual QString name() const = 0;

    /**
     * Return a sorting ordering to specify where in the list of config widgets this
     * one will be shown.
     * Higher sorting numbers will be shown first. The default is 1.
     */
    virtual int sortingOrder() const { return 1; }

    /**
     * Return true if the createConfigWidget() should be called at all for a shape of
     * the specified type.
     * @param id an ID like the KoShapeFactory::shapeId()
     */
    virtual bool showForShapeId(const QString &id) const { Q_UNUSED(id); return true; }

    /// \internal a compare for sorting.
    static bool compare(KoShapeConfigFactory *f1, KoShapeConfigFactory *f2) {
        return f1->sortingOrder() - f2->sortingOrder() > 0;
    }
};

#endif
