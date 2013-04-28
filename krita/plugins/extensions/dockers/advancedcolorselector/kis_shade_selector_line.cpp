/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
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

#include "kis_shade_selector_line.h"

#include <QPainter>
#include <QColor>
#include <QMouseEvent>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcomponentdata.h>
#include <kglobal.h>
#include <klocale.h>
#include <KoColor.h>
#include "KoColorSpaceRegistry.h"

#include "kis_canvas2.h"

#include "kis_color_selector_base.h"
#include "kis_minimal_shade_selector.h"

KisShadeSelectorLine::KisShadeSelectorLine(QWidget *parent) :
    KisShadeSelectorLineBase(parent), m_displayHelpText(false)
{
    setParam(0, 0, 0, 0, 0, 0);
    updateSettings();
    setMouseTracking(true);
}

KisShadeSelectorLine::KisShadeSelectorLine(qreal hueDelta, qreal satDelta, qreal valDelta, QWidget *parent, qreal hueShift, qreal satShift, qreal valShift) :
    KisShadeSelectorLineBase(parent), m_displayHelpText(false)
{
    setParam(hueDelta, satDelta, valDelta, hueShift, satShift, valShift);
    updateSettings();
}

void KisShadeSelectorLine::setParam(qreal hueDelta, qreal satDelta, qreal valDelta, qreal hueShift, qreal satShift, qreal valShift)
{
    m_hueDelta = hueDelta;
    m_saturationDelta = satDelta;
    m_valueDelta = valDelta;

    m_hueShift = hueShift;
    m_saturationShift = satShift;
    m_valueShift = valShift;
}

void KisShadeSelectorLine::setColor(const QColor &color)
{
    m_color=color;
    update();
}

void KisShadeSelectorLine::updateSettings()
{
    KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");

    m_gradient = cfg.readEntry("minimalShadeSelectorAsGradient", false);
    m_patchCount = cfg.readEntry("minimalShadeSelectorPatchCount", 10);
    m_lineHeight = cfg.readEntry("minimalShadeSelectorLineHeight", 20);
    m_backgroundColor = QColor(128, 128, 128);

    setMaximumHeight(m_lineHeight);
    setMinimumHeight(m_lineHeight);
}

QString KisShadeSelectorLine::toString() const
{
    return QString("%1|%2|%3|%4|%5|%6|%7").arg(m_lineNumber).arg(m_hueDelta).arg(m_saturationDelta).arg(m_valueDelta).arg(m_hueShift).arg(m_saturationShift).arg(m_valueShift);
}

void KisShadeSelectorLine::fromString(const QString& string)
{
    QStringList strili = string.split('|');
    m_lineNumber = strili.at(0).toInt();
    m_hueDelta = strili.at(1).toDouble();
    m_saturationDelta = strili.at(2).toDouble();
    m_valueDelta = strili.at(3).toDouble();
    if(strili.size()==4) return;            // don't crash, if reading old config files.
    m_hueShift = strili.at(4).toDouble();
    m_saturationShift = strili.at(5).toDouble();
    m_valueShift = strili.at(6).toDouble();
}

void KisShadeSelectorLine::paintEvent(QPaintEvent *)
{
    m_pixelCache = QImage(width(), height(), QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&m_pixelCache);
    painter.fillRect(0,0, width(), height(), m_backgroundColor);

    KisMinimalShadeSelector* parent = dynamic_cast<KisMinimalShadeSelector*>(parentWidget());

    const KoColorSpace* colorspace;
    if(parent)
        colorspace = parent->colorSpace();
    else
        colorspace = KoColorSpaceRegistry::instance()->rgb8();
    KoColor koColor(colorspace);

    int patchCount;
    int patchSpacing;

    if(m_gradient) {
        patchCount = width();
        patchSpacing = 0;
    }
    else {
        patchCount = m_patchCount;
        patchSpacing = 3;
    }
    qreal patchWidth = (width()-patchSpacing*patchCount)/qreal(patchCount);

    qreal hueStep=m_hueDelta/qreal(patchCount);
    qreal saturationStep=m_saturationDelta/qreal(patchCount);
    qreal valueStep=m_valueDelta/qreal(patchCount);

    int z=0;
    for(int i=-patchCount/2; i<=patchCount/2; i++) {
        if(i==0 && patchCount%2==0) continue;

        qreal hue=m_color.hueF()+(i*hueStep)+m_hueShift;
        while(hue<0) hue+=1.;
        while(hue>1) hue-=1.;

        qreal saturation = qBound<qreal>(0., m_color.saturationF()+(i*saturationStep)+m_saturationShift, 1.);

        qreal value = qBound<qreal>(0., m_color.valueF()+(i*valueStep)+m_valueShift, 1.);


        koColor.fromQColor(QColor::fromHsvF(hue, saturation, value));
        painter.fillRect(z*(patchWidth+patchSpacing)/*+1*/, 0, patchWidth, m_lineHeight, koColor.toQColor());
        z++;
    }

    QPainter wpainter(this);
    wpainter.drawImage(0,0, m_pixelCache);
    if(m_displayHelpText) {
        QString helpText(i18n("delta h=%1 s=%2 v=%3 shift h=%4 s=%5 v=%6",
                         m_hueDelta,
                         m_saturationDelta,
                         m_valueDelta,
                         m_hueShift,
                         m_saturationShift,
                         m_valueShift));
        wpainter.setPen(QColor(255,255,255));
        wpainter.drawText(rect(), helpText);
    }
}

void KisShadeSelectorLine::mousePressEvent(QMouseEvent* e)
{
    if(e->button()!=Qt::LeftButton && e->button()!=Qt::RightButton) {
        e->setAccepted(false);
        return;
    }

    QColor color(m_pixelCache.pixel(e->pos()));
    if(color==m_backgroundColor)
        return;

    KisColorSelectorBase* parent = dynamic_cast<KisColorSelectorBase*>(parentWidget());
    Q_ASSERT(parent);

    KisColorSelectorBase::ColorRole role = KisColorSelectorBase::Foreground;
    if(e->button()==Qt::RightButton)
        role = KisColorSelectorBase::Background;

    parent->commitColor(KoColor(color, KoColorSpaceRegistry::instance()->rgb8()), role);
    parent->KisColorSelectorBase::mousePressEvent(e);

    KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");

    bool onRightClick = cfg.readEntry("shadeSelectorUpdateOnRightClick", false);
    bool onLeftClick = cfg.readEntry("shadeSelectorUpdateOnLeftClick", false);

    if((e->button()==Qt::LeftButton && onLeftClick) || (e->button()==Qt::RightButton && onRightClick)) {
        parent->setColor(parent->findGeneratingColor(KoColor(color, KoColorSpaceRegistry::instance()->rgb8())));
    }

    e->accept();
}

void KisShadeSelectorLine::mouseMoveEvent(QMouseEvent *e)
{
    KisMinimalShadeSelector* parent = dynamic_cast<KisMinimalShadeSelector*>(parentWidget());
    QColor color(m_pixelCache.pixel(e->pos()));

    if(parent != 0)
        parent->updateColorPreview(color);
}

void KisShadeSelectorLine::mouseReleaseEvent(QMouseEvent *)
{}
