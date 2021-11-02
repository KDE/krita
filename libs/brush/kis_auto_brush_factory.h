/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_AUTO_BRUSH_FACTORY
#define KIS_AUTO_BRUSH_FACTORY

#include <QString>
#include <QDomElement>
#include <QHash>

#include <KoID.h>

#include "kis_brush.h"
#include "kis_brush_factory.h"
#include "kis_fixed_paint_device.h"

/**
 * A brush factory can create a new brush instance based
 * on a properties object that contains a serialized representation
 * of the object.
 */
class BRUSH_EXPORT KisAutoBrushFactory : public KisBrushFactory
{

public:

    KisAutoBrushFactory() {}
    ~KisAutoBrushFactory() override {}

    QString id() const override {
        return "auto_brush";
    }

    /**
     * Create a new brush from the given data or return an existing KisBrush
     * object. If this call leads to the creation of a resource, it should be
     * added to the resource provider, too.
     */
    KoResourceLoadResult createBrush(const QDomElement& brushDefinition, KisResourcesInterfaceSP resourcesInterface) override;

};

#endif

