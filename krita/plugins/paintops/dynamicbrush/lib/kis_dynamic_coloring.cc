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

#include <KoAbstractGradient.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorTransformation.h>
#include <kis_datamanager.h>

KisDynamicColoring::~KisDynamicColoring() { }

KisUniformColoring::KisUniformColoring() : m_color(0), m_cachedColor(0)
{

}

KisUniformColoring::~KisUniformColoring()
{
    delete m_color;
    delete m_cachedColor;
}

void KisUniformColoring::darken(qint32 v)
{
    KoColorTransformation* transfo = m_color->colorSpace()->createDarkenAdjustment(v, false, 0.0);
    transfo->transform(m_color->data(),  m_color->data(), 1);
    delete transfo;
}

void KisUniformColoring::rotate(double)
{}

void KisUniformColoring::resize(double , double)
{
    // Do nothing as plain color doesn't have size
}

void KisUniformColoring::colorize(KisPaintDeviceSP dev, const QRect& size)
{
    Q_UNUSED(size);
    if (!m_cachedColor || !(*dev->colorSpace() == *m_cachedColor->colorSpace())) {
        if (m_cachedColor) delete m_cachedColor;
        m_cachedColor = new KoColor(dev->colorSpace());
        m_cachedColor->fromKoColor(*m_color);
    }

    dev->dataManager()->setDefaultPixel(m_cachedColor->data());
    dev->clear();
}

void KisUniformColoring::colorAt(int x, int y, KoColor* c)
{
    Q_UNUSED(x);
    Q_UNUSED(y);

    if (!m_cachedColor || !(*c->colorSpace() == *m_cachedColor->colorSpace())) {
        if (m_cachedColor) delete m_cachedColor;
        m_cachedColor = new KoColor(c->colorSpace());
        m_cachedColor->fromKoColor(*m_color);
    }
    c->fromKoColor(*m_cachedColor);
}

void KisUniformColoring::applyColorTransformation(const KoColorTransformation* transfo)
{
    transfo->transform(m_color->data(), m_color->data(), 1);
}

const KoColorSpace* KisUniformColoring::colorSpace() const
{
    return m_color->colorSpace();
}

//-------------------------------------------------//
//---------------- KisPlainColoring ---------------//
//-------------------------------------------------//

KisPlainColoring::KisPlainColoring(const KoColor& backGroundColor, const KoColor& foreGroundColor) : m_backGroundColor(backGroundColor), m_foreGroundColor(foreGroundColor), m_cachedBackGroundColor(0)
{

}

KisPlainColoring::~KisPlainColoring()
{
    delete m_cachedBackGroundColor;
}

KisDynamicColoring* KisPlainColoring::clone() const
{
    return new KisPlainColoring(m_backGroundColor, m_foreGroundColor);
}

void KisPlainColoring::selectColor(double mix)
{
    if (!m_color || !(*m_color->colorSpace() == *m_foreGroundColor.colorSpace())) {
        delete m_color;
        m_color = new KoColor(m_foreGroundColor.colorSpace());
        delete m_cachedBackGroundColor;
        m_cachedBackGroundColor = new KoColor(m_foreGroundColor.colorSpace());
        m_cachedBackGroundColor->fromKoColor(m_backGroundColor);
    }

    const quint8 * colors[2];
    colors[0] = m_cachedBackGroundColor->data();
    colors[1] = m_foreGroundColor.data();
    int weight = (int)(mix * 255);
    const qint16 weights[2] = { 255 - weight, weight };

    m_color->colorSpace()->mixColorsOp()->mixColors(colors, weights, 2, m_color->data());

}

//-------------------------------------------------//
//--------------- KisGradientColoring -------------//
//-------------------------------------------------//

KisGradientColoring::KisGradientColoring(const KoAbstractGradient* gradient, const KoColorSpace* workingCS) : m_gradient(gradient), m_colorSpace(workingCS)
{
    m_color = new KoColor(workingCS);
}

KisGradientColoring::~KisGradientColoring()
{
}

KisDynamicColoring* KisGradientColoring::clone() const
{
    return new KisGradientColoring(m_gradient, m_colorSpace);
}

void KisGradientColoring::selectColor(double mix)
{
    m_gradient->colorAt(*m_color, mix);
}

//-------------------------------------------------//
//--------------- KisGradientColoring -------------//
//-------------------------------------------------//

KisUniformRandomColoring::KisUniformRandomColoring()
{
    m_color = new KoColor();
}

KisUniformRandomColoring::~KisUniformRandomColoring()
{
}

KisDynamicColoring* KisUniformRandomColoring::clone() const
{
    return new KisUniformRandomColoring();
}

void KisUniformRandomColoring::selectColor(double mix)
{
    Q_UNUSED(mix);
    m_color->fromQColor(QColor((int)((255.0*rand()) / RAND_MAX), (int)((255.0*rand()) / RAND_MAX), (int)((255.0*rand()) / RAND_MAX)));
}


//------------------------------------------------------//
//--------------- KisTotalRandomColoring ---------------//
//------------------------------------------------------//

KisTotalRandomColoring::KisTotalRandomColoring() : m_colorSpace(KoColorSpaceRegistry::instance()->rgb8())
{

}

KisTotalRandomColoring::~KisTotalRandomColoring()
{
}

void KisTotalRandomColoring::selectColor(double)
{
}

KisDynamicColoring* KisTotalRandomColoring::clone() const
{
    return new KisTotalRandomColoring;
}

void KisTotalRandomColoring::darken(qint32) {}
void KisTotalRandomColoring::applyColorTransformation(const KoColorTransformation*) {}
const KoColorSpace* KisTotalRandomColoring::colorSpace() const
{
    return m_colorSpace;
}
void KisTotalRandomColoring::colorize(KisPaintDeviceSP dev, const QRect& rect)
{
    KoColor kc(dev->colorSpace());

    QColor qc;

    int pixelSize = dev->colorSpace()->pixelSize();

    KisHLineIteratorPixel it = dev->createHLineIterator(rect.x(), rect.y(), rect.width(), 0);
    for (int y = 0; y < rect.height(); y++) {
        while (!it.isDone()) {
            qc.setRgb((int)((255.0*rand()) / RAND_MAX), (int)((255.0*rand()) / RAND_MAX), (int)((255.0*rand()) / RAND_MAX));
            kc.fromQColor(qc);
            memcpy(it.rawData(), kc.data(), pixelSize);
            ++it;
        }
        it.nextRow();
    }

}
void KisTotalRandomColoring::colorAt(int x, int y, KoColor* c)
{
    Q_UNUSED(x);
    Q_UNUSED(y);
    c->fromQColor(QColor((int)((255.0*rand()) / RAND_MAX), (int)((255.0*rand()) / RAND_MAX), (int)((255.0*rand()) / RAND_MAX)));
}

void KisTotalRandomColoring::rotate(double) {}
void KisTotalRandomColoring::resize(double , double) {}
