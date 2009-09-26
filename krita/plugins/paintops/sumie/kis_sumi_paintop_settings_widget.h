/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_SUMIPAINTOP_SETTINGS_WIDGET_H_
#define KIS_SUMIPAINTOP_SETTINGS_WIDGET_H_

#include <kis_paintop_settings_widget.h>

#include "ui_wdgsumieoptions.h"
#include "widgets/kis_popup_button.h"

class KisSumiPaintOpSettingsWidget : public KisPaintOpSettingsWidget
{
    Q_OBJECT

public:
    KisSumiPaintOpSettingsWidget(QWidget* parent = 0);

    virtual ~KisSumiPaintOpSettingsWidget();

    void setConfiguration( const KisPropertiesConfiguration * config);
    KisPropertiesConfiguration* configuration() const;
    void writeConfiguration( KisPropertiesConfiguration *config ) const;

    QList<float> curve() const;
    int radius() const;
    double sigma() const;
    int brushDimension() const;
    int inkAmount() const;
    bool mousePressure() const;

    bool useSaturation() const;
    bool useOpacity() const;
    bool useWeights() const;

    int pressureWeight() const;
    int bristleLengthWeight() const;
    int bristleInkAmountWeight() const;
    int inkDepletionWeight() const;

    double shearFactor() const;
    double randomFactor() const;
    double scaleFactor() const;

private:
    Ui::WdgSumieOptions* m_options;
    int m_curveSamples;
    QList<QPointF> m_curveData;
};

#endif
