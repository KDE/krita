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
#include "kis_spray_paintop_settings.h"

#include <KoColorSpaceRegistry.h>
#include <KoColor.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <QDebug>

#include <kis_paintop_registry.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_paint_information.h>
#include <kis_paint_action_type_option.h>

#include "kis_spray_paintop_settings_widget.h"
#include "kis_sprayop_option.h"
#include "kis_spray_shape_option.h"
#include "kis_spray_color_option.h"

KisSprayPaintOpSettings::KisSprayPaintOpSettings(KisSprayPaintOpSettingsWidget* settingsWidget)
        : KisPaintOpSettings(settingsWidget)
{
    m_options = settingsWidget;
    m_options->writeConfiguration( this );
}

KisPaintOpSettingsSP KisSprayPaintOpSettings::clone() const
{
    KisPaintOpSettings* settings =
        static_cast<KisPaintOpSettings*>( m_options->configuration() );
    return settings;
}


bool KisSprayPaintOpSettings::paintIncremental()
{
    return m_options->m_paintActionTypeOption->paintActionType() == BUILDUP;
}



void KisSprayPaintOpSettings::fromXML(const QDomElement& elt)
{
    KisPaintOpSettings::fromXML( elt );
    m_options->setConfiguration( this );
}

void KisSprayPaintOpSettings::toXML(QDomDocument& doc, QDomElement& rootElt) const
{
    KisPropertiesConfiguration * settings = m_options->configuration();
    settings->KisPropertiesConfiguration::toXML( doc, rootElt );
    delete settings;
}

QRectF KisSprayPaintOpSettings::paintOutlineRect(const QPointF& pos, KisImageSP image) const
{
    qreal size = diameter();
    size += 2;
    return image->pixelToDocument(
            QRectF(0,0, size,size )
        ).translated( pos - QPointF( size * 0.5 , size * 0.5 ) );
}

void KisSprayPaintOpSettings::paintOutline(const QPointF& pos, KisImageSP image, QPainter &painter, const KoViewConverter &converter) const
{
    qreal size = diameter();
    QRectF brushSize(0,0, size, size );
    painter.setPen(Qt::black);
    //painter.translate(converter.documentToView( pos - image->pixelToDocument( QPointF( size * 0.5,size * 0.5 ) + QPointF(0.5, 0.5) )) );
    painter.drawEllipse( converter.documentToView( image->pixelToDocument( brushSize ).translated(pos - QPointF( size * 0.5, size * 0.5 ) ) ) );
    qDebug() <<"pos:" << pos;
}

int KisSprayPaintOpSettings::diameter() const
{
    return m_options->m_sprayOption->diameter();
}

qreal KisSprayPaintOpSettings::coverage() const
{
    return m_options->m_sprayOption->coverage();
}

qreal KisSprayPaintOpSettings::amount() const
{
    return m_options->m_sprayOption->amount();
}

qreal KisSprayPaintOpSettings::spacing() const
{
    return m_options->m_sprayOption->spacing();
}

qreal KisSprayPaintOpSettings::scale() const {
    return m_options->m_sprayOption->scale();
}

bool KisSprayPaintOpSettings::jitterMovement() const
{
    return m_options->m_sprayOption->jitterMovement();
}

bool KisSprayPaintOpSettings::jitterSize() const
{
    return m_options->m_sprayOption->jitterSize();
}


int KisSprayPaintOpSettings::width() const {
    return m_options->m_sprayShapeOption->width();
}


int KisSprayPaintOpSettings::height() const {
    return m_options->m_sprayShapeOption->height();
}

int KisSprayPaintOpSettings::object() const {
    return m_options->m_sprayShapeOption->object();
}


int KisSprayPaintOpSettings::shape() const {
    return m_options->m_sprayShapeOption->shape();
}

bool KisSprayPaintOpSettings::jitterShapeSize() const {
    return m_options->m_sprayShapeOption->jitterShapeSize();
}

qreal KisSprayPaintOpSettings::heightPerc() const {
    return m_options->m_sprayShapeOption->heightPerc();
}

bool KisSprayPaintOpSettings::proportional() const {
    return m_options->m_sprayShapeOption->proportional();
}

qreal KisSprayPaintOpSettings::widthPerc() const {
    return m_options->m_sprayShapeOption->widthPerc();
}

bool KisSprayPaintOpSettings::useDensity() const
{
    return m_options->m_sprayOption->useDensity();
}

int KisSprayPaintOpSettings::particleCount() const
{
    return m_options->m_sprayOption->particleCount();
}


qreal KisSprayPaintOpSettings::maxTresh() const
{
    return m_options->m_sprayShapeOption->maxTresh();
}


qreal KisSprayPaintOpSettings::minTresh() const
{
    return m_options->m_sprayShapeOption->minTresh();
}


bool KisSprayPaintOpSettings::highRendering() const
{
    return m_options->m_sprayShapeOption->highRendering();
}



bool KisSprayPaintOpSettings::useRandomOpacity() const
{
    return m_options->m_sprayColorOption->useRandomOpacity();
}


int KisSprayPaintOpSettings::hue() const
{
    return m_options->m_sprayColorOption->hue();
}


int KisSprayPaintOpSettings::saturation() const
{
    return m_options->m_sprayColorOption->saturation();
}

int KisSprayPaintOpSettings::value() const
{
    return m_options->m_sprayColorOption->value();
}


bool KisSprayPaintOpSettings::useRandomHSV() const
{
    return m_options->m_sprayColorOption->useRandomHSV();
}



