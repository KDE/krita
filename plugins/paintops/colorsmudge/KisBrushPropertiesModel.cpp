/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisBrushPropertiesModel.h"

#include <KoResourceLoadResult.h>
#include "kis_brush_registry.h"


KisBrushPropertiesModel::KisBrushPropertiesModel(lager::reader<KisBrushModel::BrushData> _brushData, KisResourcesInterfaceSP _resourcesInterface)

    : resourcesInterface(_resourcesInterface)
    , brushData(_brushData)
    , isBrushPierced(brushData.map(std::bind(&KisBrushPropertiesModel::calcBrushPierced, this, std::placeholders::_1)))
    , brushApplication(brushData.map(std::bind(&KisBrushPropertiesModel::calcBrushApplication, this, std::placeholders::_1)))
{
}

bool KisBrushPropertiesModel::calcBrushPierced(const KisBrushModel::BrushData &data)
{
    KisBrushSP brush = KisBrushRegistry::instance()->createBrush(data, resourcesInterface).resource<KisBrush>();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(brush, false);

    return brush->isPiercedApprox();
}

enumBrushApplication KisBrushPropertiesModel::calcBrushApplication(const KisBrushModel::BrushData &data)
{
    return data.type == KisBrushModel::Predefined ?
                data.predefinedBrush.application :
                ALPHAMASK;
}
