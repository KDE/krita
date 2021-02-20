/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_BRUSH_FACTORY
#define KIS_BRUSH_FACTORY

#include "kis_brush.h"

class QDomElement;

/**
 * A brush factory can create a new brush instance based
 * on a properties object that contains a serialized representation
 * of the object.
 */
class BRUSH_EXPORT KisBrushFactory
{

public:

    KisBrushFactory() {}
    virtual ~KisBrushFactory() {}


    virtual QString id() const = 0;

    virtual QString name() const {
        return QString();
    }

    /**
     * Create a new brush from the given data or return an existing KisBrush
     * object. If this call leads to the creation of a resource, it should be
     * added to the resource provider, too.
     */
    virtual KisBrushSP createBrush(const QDomElement& element, KisResourcesInterfaceSP resourcesInterface) = 0;

};

#endif
