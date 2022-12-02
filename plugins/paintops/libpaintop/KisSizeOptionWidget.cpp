/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSizeOptionWidget.h"

#include <KisZug.h>
#include <kis_paintop_lod_limitations.h>

struct KisSizeOptionWidget::Private
{
    Private(lager::cursor<KisSizeOptionData> optionData)
        : lodLimitations(optionData.map(&KisSizeOptionData::lodLimitations))
    {}

    lager::reader<KisPaintopLodLimitations> lodLimitations;
};

KisSizeOptionWidget::KisSizeOptionWidget(lager::cursor<KisSizeOptionData> optionData)
    : KisSizeOptionWidget(optionData, KisPaintOpOption::GENERAL)
{
}

KisSizeOptionWidget::KisSizeOptionWidget(lager::cursor<KisSizeOptionData> optionData, PaintopCategory categoryOverride)
    : KisCurveOptionWidget2(optionData.zoom(kiszug::lenses::to_base<KisCurveOptionDataCommon>), categoryOverride)
    , m_d(new Private(optionData))
{
}

KisSizeOptionWidget::~KisSizeOptionWidget()
{
}

lager::reader<KisPaintopLodLimitations> KisSizeOptionWidget::lodLimitationsReader() const
{
    return m_d->lodLimitations;
}
