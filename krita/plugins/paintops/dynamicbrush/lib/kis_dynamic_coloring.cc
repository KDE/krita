/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_dynamic_coloring.h"

#include <kis_paint_device.h>

#include <KoColorTransformation.h>
#include <kis_datamanager.h>

KisDynamicColoring::~KisDynamicColoring() { }

KisPlainColoring::KisPlainColoring(const KoColor& backGroundColor, const KoColor& foreGroundColor) : m_backGroundColor(backGroundColor), m_foreGroundColor(foreGroundColor), m_color(0), m_cachedColor(0), m_cachedBackGroundColor(0)
{

}

KisPlainColoring::~KisPlainColoring()
{
    delete m_color;
    delete m_cachedColor;
    delete m_cachedBackGroundColor;
}

KisDynamicColoring* KisPlainColoring::clone() const
{
    return new KisPlainColoring(m_backGroundColor, m_foreGroundColor);
}

void KisPlainColoring::selectColor(double mix)
{
    if(not m_color or not (*m_color->colorSpace() == *m_foreGroundColor.colorSpace()))
    {
        delete m_color;
        m_color = new KoColor( m_foreGroundColor.colorSpace() );
        delete m_cachedBackGroundColor;
        m_cachedBackGroundColor = new KoColor( m_foreGroundColor.colorSpace() );
        m_cachedBackGroundColor->fromKoColor( m_backGroundColor );
    }

    const quint8 * colors[2];
    colors[0] = m_cachedBackGroundColor->data();
    colors[1] = m_foreGroundColor.data();
    int weight = (int)(mix * 255);
    const quint8 weights[2] = { 255 - weight, weight };
    
    m_color->colorSpace()->mixColorsOp()->mixColors(colors, weights, 2, m_color->data());

}

void KisPlainColoring::darken(qint32 v)
{
    KoColorTransformation* transfo = m_color->colorSpace()->createDarkenAdjustement(v, false, 0.0);
    transfo->transform( m_color->data(),  m_color->data(), 1);
    delete transfo;
}

void KisPlainColoring::rotate(double )
{}

void KisPlainColoring::resize(double , double ) {
    // Do nothing as plain color doesn't have size
}

void KisPlainColoring::colorize(KisPaintDeviceSP dev )
{
    if(not m_cachedColor or not (*dev->colorSpace() == *m_cachedColor->colorSpace()))
    {
        if(m_cachedColor) delete m_cachedColor;
        m_cachedColor = new KoColor( dev->colorSpace() );
        m_cachedColor->fromKoColor(*m_color);
    }
    
    dev->dataManager()->setDefaultPixel(  m_cachedColor->data());
    dev->clear();
}

void KisPlainColoring::colorAt(int x, int y, KoColor* c)
{
    Q_UNUSED( x );
    Q_UNUSED( y );

    if(not m_cachedColor or not (*c->colorSpace() == *m_cachedColor->colorSpace()))
    {
        if(m_cachedColor) delete m_cachedColor;
        m_cachedColor = new KoColor( c->colorSpace() );
        m_cachedColor->fromKoColor(*m_color);
    }
    c->fromKoColor(*m_cachedColor);
}
