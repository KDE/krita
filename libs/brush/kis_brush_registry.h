/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_BRUSH_REGISTRY_H_
#define KIS_BRUSH_REGISTRY_H_

#include <QObject>

#include "kis_types.h"
#include "KoGenericRegistry.h"

#include <kritabrush_export.h>

#include "kis_brush.h"
#include "kis_brush_factory.h"

class QDomElement;

class BRUSH_EXPORT KisBrushRegistry : public QObject, public KoGenericRegistry<KisBrushFactory*>
{

    Q_OBJECT

public:
    KisBrushRegistry();
    ~KisBrushRegistry() override;

    static KisBrushRegistry* instance();

    KisBrushSP createBrush(const QDomElement& element, KisResourcesInterfaceSP resourcesInterface);

private:
    KisBrushRegistry(const KisBrushRegistry&);
    KisBrushRegistry operator=(const KisBrushRegistry&);
};

#endif // KIS_GENERATOR_REGISTRY_H_
