/*
 *  Copyright (c) 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include <kis_deform_paintop_settings.h>

#include <KoColorSpaceRegistry.h>
#include <KoViewConverter.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <kis_paintop_registry.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_paint_information.h>

#include <KoColor.h>
#include <qdebug.h>

#include <kis_deform_paintop_settings_widget.h>

KisDeformPaintOpSettings::KisDeformPaintOpSettings(KisDeformPaintOpSettingsWidget* settingsWidget)
        : KisPaintOpSettings(settingsWidget)
{
    m_options = settingsWidget;
    // Initialize with the default settings from the widget
    m_options->writeConfiguration( this );
}


KisPaintOpSettingsSP KisDeformPaintOpSettings::clone() const
{
    KisPaintOpSettings* settings =
        static_cast<KisPaintOpSettings*>( m_options->configuration() );
    return settings;
}

bool KisDeformPaintOpSettings::paintIncremental()
{
    return true;
}

int KisDeformPaintOpSettings::radius() const
{
    return m_options->radius();
}


double KisDeformPaintOpSettings::deformAmount() const
{
    return m_options->deformAmount();
}

bool KisDeformPaintOpSettings::bilinear() const
{
    return m_options->bilinear();
}

bool KisDeformPaintOpSettings::useMovementPaint() const
{
    return m_options->useMovementPaint();
}

bool KisDeformPaintOpSettings::useCounter() const
{
    return m_options->useCounter();
}

bool KisDeformPaintOpSettings::useOldData() const
{
    return m_options->useOldData();
}

int KisDeformPaintOpSettings::deformAction() const
{
    return m_options->deformAction();
}

void KisDeformPaintOpSettings::fromXML(const QDomElement& elt)
{
    // First, call the parent class fromXML to make sure all the
    // properties are saved to the map
    KisPaintOpSettings::fromXML( elt );
    // Then load the properties for all widgets
    m_options->setConfiguration( this );
}

void KisDeformPaintOpSettings::toXML(QDomDocument& doc, QDomElement& rootElt) const
{
    // First, make sure all the option widgets have saved their state
    // to the property configuration
    KisPropertiesConfiguration * settings = m_options->configuration();
    // Then call the parent class fromXML
    settings->KisPropertiesConfiguration::toXML( doc, rootElt );
    delete settings;
}

QRectF KisDeformPaintOpSettings::paintOutlineRect(const QPointF& pos, KisImageSP image) const
{
    QPointF hotSpot = QPointF( radius(), radius() );
    return image->pixelToDocument(
            QRectF(0,0, radius() * 2, radius() * 2).translated(-(hotSpot + QPointF(0.5, 0.5)) )
        ).translated( pos );
}

void KisDeformPaintOpSettings::paintOutline(const QPointF& pos, KisImageSP image, QPainter& painter, const KoViewConverter& converter) const
{
    QPointF hotSpot = QPointF( radius(), radius() );
    painter.setPen(Qt::black);
    painter.setBackground(Qt::black);
    painter.translate( converter.documentToView( pos - image->pixelToDocument(hotSpot + QPointF(0.5, 0.5) )) );

    QPointF p1 = converter.documentToView( image->pixelToDocument( pos ) );
    // if you zoom, radius() have to be according zoom so something like radius() * zoomFromImage * maybeAlsoDPI;
    painter.drawEllipse(p1, radius(),radius() );
}
