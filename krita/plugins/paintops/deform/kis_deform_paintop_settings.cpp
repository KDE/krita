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
#include <KoViewConverter.h>

#include <kis_deform_paintop_settings.h>
#include <kis_deform_paintop_settings_widget.h>

KisDeformPaintOpSettings::KisDeformPaintOpSettings()
        : m_options(0)
{
}


KisPaintOpSettingsSP KisDeformPaintOpSettings::clone() const
{
    KisPaintOpSettings* settings =
        static_cast<KisPaintOpSettings*>(m_options->configuration());
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

qreal KisDeformPaintOpSettings::spacing() const
{
    return m_options->spacing();
}

void KisDeformPaintOpSettings::fromXML(const QDomElement& elt)
{
    // First, call the parent class fromXML to make sure all the
    // properties are saved to the map
    KisPaintOpSettings::fromXML(elt);
    // Then load the properties for all widgets
    m_options->setConfiguration(this);
}

void KisDeformPaintOpSettings::toXML(QDomDocument& doc, QDomElement& rootElt) const
{
    // First, make sure all the option widgets have saved their state
    // to the property configuration
    KisPropertiesConfiguration * settings = m_options->configuration();
    // Then call the parent class fromXML
    settings->KisPropertiesConfiguration::toXML(doc, rootElt);
    delete settings;
}

QRectF KisDeformPaintOpSettings::paintOutlineRect(const QPointF& pos, KisImageWSP image, OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return QRectF();
    qreal size = radius() * 2;
    size += 10;
    return image->pixelToDocument(QRectF(0, 0, size, size).translated(- QPoint(size * 0.5, size * 0.5))).translated(pos);
}

void KisDeformPaintOpSettings::paintOutline(const QPointF& pos, KisImageWSP image, QPainter &painter, const KoViewConverter &converter, OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return;
    qreal size = radius() * 2;

#if 0
//     painter.setPen( QColor(128,255,128) );
//     painter.setCompositionMode(QPainter::CompositionMode_Exclusion);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QRectF sizerc = converter.documentToView(image->pixelToDocument(QRectF(0, 0, size, size).translated(- QPoint(size * 0.5, size * 0.5))).translated(pos));
    QPen pen = painter.pen();
    pen.setColor(QColor(3, 3, 3, 150));
    pen.setWidth(5);
    painter.setPen(pen);
    painter.drawEllipse(sizerc);
    pen.setColor(Qt::white);
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawEllipse(sizerc);
#else
    painter.setPen(Qt::black);
#endif
    painter.drawEllipse(converter.documentToView(image->pixelToDocument(QRectF(0, 0, size, size).translated(- QPoint(size * 0.5, size * 0.5))).translated(pos)));
}



void KisDeformPaintOpSettings::changePaintOpSize(qreal x, qreal y) const
{
    // if the movement is more left<->right then up<->down
    if (qAbs(x) > qAbs(y)){
        m_options->setRadius( radius() + qRound(x) );
    }
    else // vice-versa
    {
        // we can do something different, e.g. change deform mode or ...
    }
}
