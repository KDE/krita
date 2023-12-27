/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDeformOptionData.h"

#include "kis_properties_configuration.h"
#include <kis_paintop_lod_limitations.h>


const QString DEFORM_AMOUNT = "Deform/deformAmount";
const QString DEFORM_ACTION = "Deform/deformAction";
const QString DEFORM_USE_BILINEAR = "Deform/bilinear";
const QString DEFORM_USE_MOVEMENT_PAINT = "Deform/useMovementPaint";
const QString DEFORM_USE_COUNTER = "Deform/useCounter";
const QString DEFORM_USE_OLD_DATA = "Deform/useOldData";


bool KisDeformOptionData::read(const KisPropertiesConfiguration *setting)
{
    deformAmount = setting->getDouble(DEFORM_AMOUNT, 0.2);
    deformUseBilinear = setting->getBool(DEFORM_USE_BILINEAR, false);
    deformUseCounter = setting->getBool(DEFORM_USE_COUNTER, false);
    deformUseOldData = setting->getBool(DEFORM_USE_OLD_DATA, false);
    deformAction = (DeformModes)setting->getInt(DEFORM_ACTION, 1);

    return true;
}

void KisDeformOptionData::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty(DEFORM_AMOUNT, deformAmount);
    setting->setProperty(DEFORM_ACTION, deformAction);
    setting->setProperty(DEFORM_USE_BILINEAR, deformUseBilinear);
    setting->setProperty(DEFORM_USE_COUNTER, deformUseCounter);
    setting->setProperty(DEFORM_USE_OLD_DATA, deformUseOldData);
}

KisPaintopLodLimitations KisDeformOptionData::lodLimitations() const
{
    KisPaintopLodLimitations l;
    l.blockers << KoID("deform-brush", i18nc("PaintOp instant preview limitation", "Deform Brush (unsupported)"));
    return l;
}
