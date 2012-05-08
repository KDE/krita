/*
 *  Copyright (c) 2006-2007, 2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_color_source.h"
#include <kis_paint_device.h>

#include <KoAbstractGradient.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorTransformation.h>
#include <kis_datamanager.h>
#include <kis_fill_painter.h>
#include "kis_iterator_ng.h"

KisColorSource::~KisColorSource() { }

const KoColor black;

const KoColor& KisColorSource::uniformColor() const
{
    qFatal("Not an uniform color.");
    return black;
}


KisUniformColorSource::KisUniformColorSource() : m_color(0), m_cachedColor(0)
{
}

KisUniformColorSource::~KisUniformColorSource()
{
    delete m_color;
    delete m_cachedColor;
}

void KisUniformColorSource::rotate(double)
{}

void KisUniformColorSource::resize(double , double)
{
    // Do nothing as plain color does not have size
}

void KisUniformColorSource::colorize(KisPaintDeviceSP dev, const QRect& size, const QPoint&) const
{
    Q_UNUSED(size);
    if (!m_cachedColor || !(*dev->colorSpace() == *m_cachedColor->colorSpace())) {
        delete m_cachedColor;
        m_cachedColor = new KoColor(dev->colorSpace());
        m_cachedColor->fromKoColor(*m_color);
    }

    dev->dataManager()->setDefaultPixel(m_cachedColor->data());
    dev->clear();
}

const KoColor& KisUniformColorSource::uniformColor() const
{
    return *m_color;
}

void KisUniformColorSource::applyColorTransformation(const KoColorTransformation* transfo)
{
    transfo->transform(m_color->data(), m_color->data(), 1);
}

const KoColorSpace* KisUniformColorSource::colorSpace() const
{
    return m_color->colorSpace();
}

bool KisUniformColorSource::isUniformColor() const
{
    return true;
}

//-------------------------------------------------//
//---------------- KisPlainColorSource ---------------//
//-------------------------------------------------//

KisPlainColorSource::KisPlainColorSource(const KoColor& backGroundColor, const KoColor& foreGroundColor) : m_backGroundColor(backGroundColor), m_foreGroundColor(foreGroundColor), m_cachedBackGroundColor(0)
{
}

KisPlainColorSource::~KisPlainColorSource()
{
    delete m_cachedBackGroundColor;
}

void KisPlainColorSource::selectColor(double mix)
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
//--------------- KisGradientColorSource -------------//
//-------------------------------------------------//

KisGradientColorSource::KisGradientColorSource(const KoAbstractGradient* gradient, const KoColorSpace* workingCS) : m_gradient(gradient), m_colorSpace(workingCS)
{
    m_color = new KoColor(workingCS);
    Q_ASSERT(gradient);
}

KisGradientColorSource::~KisGradientColorSource()
{
}

void KisGradientColorSource::selectColor(double mix)
{
    m_gradient->colorAt(*m_color, mix);
}

//-------------------------------------------------//
//--------------- KisGradientColorSource -------------//
//-------------------------------------------------//

KisUniformRandomColorSource::KisUniformRandomColorSource()
{
    m_color = new KoColor();
}

KisUniformRandomColorSource::~KisUniformRandomColorSource()
{
}

void KisUniformRandomColorSource::selectColor(double mix)
{
    Q_UNUSED(mix);
    m_color->fromQColor(QColor((int)((255.0*rand()) / RAND_MAX), (int)((255.0*rand()) / RAND_MAX), (int)((255.0*rand()) / RAND_MAX)));
}


//------------------------------------------------------//
//--------------- KisTotalRandomColorSource ---------------//
//------------------------------------------------------//

KisTotalRandomColorSource::KisTotalRandomColorSource() : m_colorSpace(KoColorSpaceRegistry::instance()->rgb8())
{
}

KisTotalRandomColorSource::~KisTotalRandomColorSource()
{
}

void KisTotalRandomColorSource::selectColor(double)
{
}

void KisTotalRandomColorSource::applyColorTransformation(const KoColorTransformation*) {}
const KoColorSpace* KisTotalRandomColorSource::colorSpace() const
{
    return m_colorSpace;
}

void KisTotalRandomColorSource::colorize(KisPaintDeviceSP dev, const QRect& rect, const QPoint&) const
{
    KoColor kc(dev->colorSpace());

    QColor qc;

    int pixelSize = dev->colorSpace()->pixelSize();

    KisHLineIteratorSP it = dev->createHLineIteratorNG(rect.x(), rect.y(), rect.width());
    for (int y = 0; y < rect.height(); y++) {
        do {
            qc.setRgb((int)((255.0*rand()) / RAND_MAX), (int)((255.0*rand()) / RAND_MAX), (int)((255.0*rand()) / RAND_MAX));
            kc.fromQColor(qc);
            memcpy(it->rawData(), kc.data(), pixelSize);
        } while (it->nextPixel());
        it->nextRow();
    }

}

bool KisTotalRandomColorSource::isUniformColor() const
{
    return false;
}

void KisTotalRandomColorSource::rotate(double) {}
void KisTotalRandomColorSource::resize(double , double) {}



KisPatternColorSource::KisPatternColorSource(KisPaintDeviceSP _pattern, int _width, int _height, bool _locked)
    : m_device(_pattern)
    , m_bounds(QRect(0, 0, _width, _height))
    , m_locked(_locked)
{
}

KisPatternColorSource::~KisPatternColorSource()
{
}

void KisPatternColorSource::selectColor(double mix)
{
    Q_UNUSED(mix);
}

void KisPatternColorSource::applyColorTransformation(const KoColorTransformation* transfo)
{
    Q_UNUSED(transfo);
}

const KoColorSpace* KisPatternColorSource::colorSpace() const
{
    return m_device->colorSpace();
}

void KisPatternColorSource::colorize(KisPaintDeviceSP device, const QRect& rect, const QPoint& offset) const
{
    KisFillPainter painter(device);
    if (m_locked) {
        painter.fillRect(rect.x(), rect.y(), rect.width(), rect.height(), m_device, m_bounds);
    }
    else {
        int x = offset.x() % m_bounds.width();
        int y = offset.y() % m_bounds.height();
        
        // Change the position, because the pattern is always applied starting
        // from (0,0) in the paint device reference
        device->setX(x);
        device->setY(y);
        painter.fillRect(rect.x() + x, rect.y() + y, rect.width(), rect.height(), m_device, m_bounds);
        device->setX(0);
        device->setY(0);
    }
}

void KisPatternColorSource::rotate(double r)
{
  Q_UNUSED(r);
}

void KisPatternColorSource::resize(double xs, double ys)
{
  Q_UNUSED(xs);
  Q_UNUSED(ys);
}

bool KisPatternColorSource::isUniformColor() const
{
    return false;
}

