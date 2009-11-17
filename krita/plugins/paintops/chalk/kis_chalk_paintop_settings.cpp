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

#include "kis_chalk_paintop_settings.h"

#include "kis_chalk_paintop_settings_widget.h"
#include "kis_chalkop_option.h"

#include <kis_paint_action_type_option.h>

KisChalkPaintOpSettings::KisChalkPaintOpSettings()
        : m_options(0)
{
}

KisPaintOpSettingsSP KisChalkPaintOpSettings::clone() const
{
    KisPaintOpSettings* settings =
        static_cast<KisPaintOpSettings*>(m_options->configuration());
    return settings;
}


bool KisChalkPaintOpSettings::paintIncremental()
{
    return m_options->m_paintActionTypeOption->paintActionType() == BUILDUP;
}



void KisChalkPaintOpSettings::fromXML(const QDomElement& elt)
{
    KisPaintOpSettings::fromXML(elt);
    m_options->setConfiguration(this);
}

void KisChalkPaintOpSettings::toXML(QDomDocument& doc, QDomElement& rootElt) const
{
    KisPropertiesConfiguration * settings = m_options->configuration();
    settings->KisPropertiesConfiguration::toXML(doc, rootElt);
    delete settings;
}


void KisChalkPaintOpSettings::changePaintOpSize(qreal x, qreal y) const
{
    // if the movement is more left<->right then up<->down
    if (qAbs(x) > qAbs(y)){
        m_options->m_chalkOption->setRadius( radius() + qRound(x) );
    }
    else // vice-versa
    {
        // we can do something different
    }

}


void KisChalkPaintOpSettings::paintOutline(const QPointF& pos, KisImageWSP image, QPainter& painter, const KoViewConverter& converter, KisPaintOpSettings::OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return;
    qreal size = radius() * 2 * + 1;
    painter.setPen(Qt::black);
    painter.drawEllipse(converter.documentToView(image->pixelToDocument(QRectF(0, 0, size, size).translated(- QPoint(size * 0.5, size * 0.5))).translated(pos)));
}


QRectF KisChalkPaintOpSettings::paintOutlineRect(const QPointF& pos, KisImageWSP image, KisPaintOpSettings::OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return QRectF();
    qreal size = radius() * 2;
    size += 10;
    return image->pixelToDocument(QRectF(0, 0, size, size).translated(- QPoint(size * 0.5, size * 0.5))).translated(pos);
}

int KisChalkPaintOpSettings::radius() const
{
    return m_options->m_chalkOption->radius();
}


KisPressureOpacityOption* KisChalkPaintOpSettings::opacityOption() const
{
    return m_options->m_opacityOption;
}



bool KisChalkPaintOpSettings::inkDepletion() const
{
    return m_options->m_chalkOption->inkDepletion();
}


bool KisChalkPaintOpSettings::opacity() const
{
    return m_options->m_chalkOption->opacity();
}


bool KisChalkPaintOpSettings::saturation() const
{
    return m_options->m_chalkOption->saturation();
}


#if defined(HAVE_OPENGL)
QString KisChalkPaintOpSettings::modelName() const
{
    return "3d-pencil";
}
#endif

