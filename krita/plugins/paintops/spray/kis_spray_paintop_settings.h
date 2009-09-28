/*
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

#ifndef KIS_SPRAY_PAINTOP_SETTINGS_H_
#define KIS_SPRAY_PAINTOP_SETTINGS_H_

#include <kis_paintop_settings.h>
#include <kis_types.h>

#include "kis_spray_paintop_settings_widget.h"

class QWidget;
class QDomElement;
class QDomDocument;


class KisSprayPaintOpSettings : public KisPaintOpSettings
{

public:

    KisSprayPaintOpSettings();
    virtual ~KisSprayPaintOpSettings() {}

    virtual QRectF paintOutlineRect(const QPointF& pos, KisImageSP image, OutlineMode _mode ) const;
    virtual void paintOutline(const QPointF& pos, KisImageSP image, QPainter &painter, const KoViewConverter &converter, OutlineMode _mode) const;

    bool paintIncremental();

    using KisPropertiesConfiguration::fromXML;
    using KisPropertiesConfiguration::toXML;

    virtual void fromXML(const QDomElement&);
    virtual void toXML(QDomDocument&, QDomElement&) const;

    QRectF paintOutlineRect(const QPointF& pos, KisImageSP image) const;
    void paintOutline(const QPointF& pos, KisImageSP image, QPainter &painter, const KoViewConverter &converter) const;

    KisPaintOpSettingsSP clone() const;

    int diameter() const;

    qreal coverage() const;
    qreal amount() const;
    qreal spacing() const;
    qreal scale() const;

    int object() const;
    int shape() const;
    int width() const;
    int height() const;
    bool jitterShapeSize() const;


    // metaballs
    qreal maxTresh() const;
    qreal minTresh() const;

    // color options
    bool useRandomOpacity() const;
    bool useRandomHSV() const;

    // TODO: these should be intervals like 20..180
    int hue() const;
    int saturation() const;
    int value() const;

    bool highRendering() const;
    bool proportional() const;
    qreal widthPerc() const;
    qreal heightPerc() const;

    bool jitterMovement() const;
    bool jitterSize() const;

    bool useDensity() const;
    int particleCount() const;

    bool gaussian() const;

    // XXX: Hack!
    void setOptionsWidget(KisPaintOpSettingsWidget* widget)
    {
        if (m_options != 0  && m_options->property("owned by settings").toBool()) {
            delete m_options;
        }
        if (!widget) {
            m_options = 0;
        }
        else {
            m_options = qobject_cast<KisSprayPaintOpSettingsWidget*>( widget );
            m_options->writeConfiguration( this );
        }
    }


private:

    KisSprayPaintOpSettingsWidget* m_options;

};

#endif
