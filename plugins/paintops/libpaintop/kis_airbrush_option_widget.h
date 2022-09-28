/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_AIRBRUSH_OPTION_H
#define KIS_AIRBRUSH_OPTION_H

#include <kis_paintop_option.h>
#include <kritapaintop_export.h>

class PAINTOP_EXPORT KisAirbrushOptionProperties : public KisPaintopPropertiesBase
{
public:
    bool enabled {false};
    qreal airbrushInterval {1000.0 / 20.0};
    bool ignoreSpacing {false};
protected:
    void readOptionSettingImpl(const KisPropertiesConfiguration *settings) override;
    void writeOptionSettingImpl(KisPropertiesConfiguration *settings) const override;
};


#endif
