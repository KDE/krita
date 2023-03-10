/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISBRUSHPROPERTIESMODEL_H
#define KISBRUSHPROPERTIESMODEL_H

#include <KisBrushModel.h>
#include <lager/cursor.hpp>

class KisBrushPropertiesModel
{
public:
    KisBrushPropertiesModel(lager::reader<KisBrushModel::BrushData> _brushData,
                            KisResourcesInterfaceSP _resourcesInterface);
private:
    KisResourcesInterfaceSP resourcesInterface;

public:
    lager::reader<KisBrushModel::BrushData> brushData;
    lager::reader<bool> isBrushPierced;
    lager::reader<enumBrushApplication> brushApplication;

private:
    bool calcBrushPierced(const KisBrushModel::BrushData &data);

    enumBrushApplication calcBrushApplication(const KisBrushModel::BrushData &data);
};
#endif // KISBRUSHPROPERTIESMODEL_H
