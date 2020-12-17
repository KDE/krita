/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_TEXT_BRUSH_FACTORY
#define KIS_TEXT_BRUSH_FACTORY

#include <QString>
#include <QDomElement>
#include "kis_brush_factory.h"
#include "kis_brush.h"

/**
 * A brush factory can create a new brush instance based
 * on a properties object that contains a serialized representation
 * of the object.
 */
class BRUSH_EXPORT KisTextBrushFactory : public KisBrushFactory
{

public:

    KisTextBrushFactory() {}
    ~KisTextBrushFactory() override {}

    QString id() const override {
        return "kis_text_brush";
    }


    /**
     * Create a new brush from the given data or return an existing KisBrush
     * object. If this call leads to the creation of a resource, it should be
     * added to the resource provider, too.
     */
    KisBrushSP createBrush(const QDomElement& brushDefinition, KisResourcesInterfaceSP resourcesInterface) override;


};

#endif
