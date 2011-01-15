/*
 * Copyright (c) 2011 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_COLOR_SOURCE_OPTION_WIDGET_H
#define KIS_COLOR_SOURCE_OPTION_WIDGET_H

#include "kis_paintop_option.h"

/**
 * The brush option allows the user to select a particular brush
 * footprint for suitable paintops
 */
class PAINTOP_EXPORT KisColorSourceOptionWidget : public KisPaintOpOption
{
    Q_OBJECT
public:
    KisColorSourceOptionWidget();
    ~KisColorSourceOptionWidget();

    void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    void readOptionSetting(const KisPropertiesConfiguration* setting);
private slots:
    void sourceChanged();
private:
    struct Private;
    Private* const d;    
};

#endif
