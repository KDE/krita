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

#ifndef KIS_SUMIPAINTOP_SETTINGS_H_
#define KIS_SUMIPAINTOP_SETTINGS_H_

#include <kis_paintop_settings.h>
#include <kis_types.h>
#include "kis_sumi_paintop_settings_widget.h"

class QWidget;
class QDomElement;
class QDomDocument;

class KisSumiPaintOpSettings : public KisPaintOpSettings
{

public:
    KisSumiPaintOpSettings();
    virtual ~KisSumiPaintOpSettings() {}

    bool paintIncremental();
    
    using KisPropertiesConfiguration::fromXML;
    using KisPropertiesConfiguration::toXML;

    virtual void fromXML(const QDomElement&);
    virtual void toXML(QDomDocument&, QDomElement&) const;

    virtual QRectF paintOutlineRect(const QPointF& pos, KisImageWSP image, OutlineMode _mode) const;
    virtual void paintOutline(const QPointF& pos, KisImageWSP image, QPainter &painter, const KoViewConverter &converter, OutlineMode _mode) const;

    virtual void changePaintOpSize(qreal x, qreal y) const;

    
    
    KisPaintOpSettingsSP clone() const;

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

    // XXX: Hack!
    void setOptionsWidget(KisPaintOpSettingsWidget* widget) {
        if (m_options != 0 && m_options->property("owned by settings").toBool()) {
            delete m_options;
        }
        if (!widget) {
            m_options = 0;
        } else {
            m_options = qobject_cast<KisSumiPaintOpSettingsWidget*>(widget);
            m_options->writeConfiguration(this);
        }
    }

private:

    KisSumiPaintOpSettingsWidget* m_options;
};

#endif