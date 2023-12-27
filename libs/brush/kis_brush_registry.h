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
#include "KisBrushModel.h"

class QDomElement;

class BRUSH_EXPORT KisBrushRegistry : public QObject, public KoGenericRegistry<KisBrushFactory*>
{

    Q_OBJECT

public:
    KisBrushRegistry();
    ~KisBrushRegistry() override;

    static KisBrushRegistry* instance();

    KoResourceLoadResult createBrush(const QDomElement& element, KisResourcesInterfaceSP resourcesInterface);
    KoResourceLoadResult createBrush(const KisBrushModel::BrushData &data, KisResourcesInterfaceSP resourcesInterface);
    std::optional<KisBrushModel::BrushData> createBrushModel(const QDomElement& element, KisResourcesInterfaceSP resourcesInterface);
    void toXML(QDomDocument &doc, QDomElement& element, const KisBrushModel::BrushData &model);

private:
    KisBrushRegistry(const KisBrushRegistry&);
    KisBrushRegistry operator=(const KisBrushRegistry&);
};

#endif // KIS_GENERATOR_REGISTRY_H_
