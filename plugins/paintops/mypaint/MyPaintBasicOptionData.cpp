/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "MyPaintBasicOptionData.h"

#include <kis_properties_configuration.h>

bool MyPaintBasicOptionData::read(const KisPropertiesConfiguration *setting)
{
    eraserMode = setting->getBool("EraserMode", false);

    return true;
}

void MyPaintBasicOptionData::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty("EraserMode", eraserMode);
}
