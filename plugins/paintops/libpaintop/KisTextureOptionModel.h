/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISTEXTUREOPTIONMODEL_H
#define KISTEXTUREOPTIONMODEL_H


#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisTextureOptionData.h"
#include "KoResourceLoadResult.h"


class KisTextureOptionModel : public QObject
{
    Q_OBJECT
public:
    KisTextureOptionModel(lager::cursor<KisTextureOptionData> optionData, KisResourcesInterfaceSP resourcesInterface);
    lager::cursor<KisTextureOptionData> optionData;

    LAGER_QT_CURSOR(bool, isEnabled);
    LAGER_QT_CURSOR(KoResourceSP, textureResource);
    LAGER_QT_CURSOR(qreal, scale);
    LAGER_QT_CURSOR(qreal, brightness);
    LAGER_QT_CURSOR(qreal, contrast);
    LAGER_QT_CURSOR(qreal, neutralPoint);
    LAGER_QT_CURSOR(int, offsetX);
    LAGER_QT_CURSOR(int, offsetY);
    LAGER_QT_CURSOR(int, maximumOffsetX);
    LAGER_QT_CURSOR(int, maximumOffsetY);
    LAGER_QT_CURSOR(bool, isRandomOffsetX);
    LAGER_QT_CURSOR(bool, isRandomOffsetY);
    LAGER_QT_CURSOR(int, texturingMode);
    LAGER_QT_CURSOR(int, cutOffPolicy);
    LAGER_QT_CURSOR(qreal, cutOffLeftNormalized);
    LAGER_QT_CURSOR(qreal, cutOffRightNormalized);
    LAGER_QT_CURSOR(bool, invert);

    KisTextureOptionData bakedOptionData() const;

private:
    void updateOffsetLimits(KoResourceSP resource);
};

#endif // KISTEXTUREOPTIONMODEL_H
