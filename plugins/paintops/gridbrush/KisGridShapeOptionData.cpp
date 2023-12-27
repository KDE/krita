/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisGridShapeOptionData.h"

#include "kis_properties_configuration.h"


bool KisGridShapeOptionData::read(const KisPropertiesConfiguration *setting)
{
	shape = setting->getInt(GRIDSHAPE_SHAPE, 0);
    return true;
}

void KisGridShapeOptionData::write(KisPropertiesConfiguration *setting) const
{
	setting->setProperty(GRIDSHAPE_SHAPE, shape);
}
