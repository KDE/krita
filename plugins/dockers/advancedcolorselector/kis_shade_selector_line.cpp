/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_shade_selector_line.h"

#include <QPainter>
#include <QColor>
#include <QMouseEvent>

#include <ksharedconfig.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <KoColorSpaceRegistry.h>

#include "kis_canvas2.h"

#include "kis_color_selector_base_proxy.h"
#include "kis_display_color_converter.h"
#include "kis_paint_device.h"


KisShadeSelectorLine::KisShadeSelectorLine(KisColorSelectorBaseProxy *parentProxy,
                                           QWidget *parent)
    : KisShadeSelectorLineBase(parent),
      m_cachedColorSpace(0),
      m_displayHelpText(false),
      m_parentProxy(parentProxy)
{
    setParam(0, 0, 0, 0, 0, 0);
    updateSettings();
    setMouseTracking(true);
    m_mouseX=width()/2;
    m_isDown=false;
}

KisShadeSelectorLine::KisShadeSelectorLine(qreal hueDelta, qreal satDelta, qreal valDelta,
                                           KisColorSelectorBaseProxy *parentProxy, QWidget *parent, qreal hueShift, qreal satShift, qreal valShift) :
    KisShadeSelectorLineBase(parent),
    m_cachedColorSpace(0),
    m_displayHelpText(false),
    m_parentProxy(parentProxy)
{
    setParam(hueDelta, satDelta, valDelta, hueShift, satShift, valShift);
    updateSettings();
    m_mouseX=width()/2;
    m_isDown=false;
}

KisShadeSelectorLine::~KisShadeSelectorLine()
{
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

void KisShadeSelectorLine::setColor(const KoColor &color)
{
    m_realColor = color;
    m_realColor.convertTo(m_parentProxy->colorSpace());
    m_mouseX=width()/2;

    update();
}

void KisShadeSelectorLine::updateSettings()
{
    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");

    m_gradient = cfg.readEntry("minimalShadeSelectorAsGradient", false);
    m_patchCount = cfg.readEntry("minimalShadeSelectorPatchCount", 10);
    m_lineHeight = cfg.readEntry("minimalShadeSelectorLineHeight", 20);

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

    if (m_cachedColorSpace != m_parentProxy->colorSpace()) {
        m_realPixelCache = new KisPaintDevice(m_parentProxy->colorSpace());
        m_cachedColorSpace = m_parentProxy->colorSpace();
    }
    else {
        m_realPixelCache->clear();
    }

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
    qreal hueStep;
    qreal saturationStep;
    qreal valueStep;

    if(m_gradient){
       hueStep=m_hueDelta/qreal(patchCount-10);
       saturationStep=m_saturationDelta/qreal(patchCount-10);
       valueStep=m_valueDelta/qreal(patchCount-10);
    }
    else{
       hueStep=m_hueDelta/qreal(patchCount);
       saturationStep=m_saturationDelta/qreal(patchCount);
       valueStep=m_valueDelta/qreal(patchCount);
    }

    qreal baseHue;
    qreal baseSaturation;
    qreal baseValue;
    m_parentProxy->converter()->
        getHsvF(m_realColor, &baseHue, &baseSaturation, &baseValue);

    int z=0;
    for(int i=-patchCount/2; i<=patchCount/2; i++) {
        if(i==0 && patchCount%2==0) continue;
        qreal hue = baseHue + (i * hueStep) + m_hueShift;
        while (hue < 0.0) hue += 1.0;
        while (hue > 1.0) hue -= 1.0;

        qreal saturation = qBound<qreal>(0., baseSaturation + (i * saturationStep) + m_saturationShift, 1.);
        qreal value = qBound<qreal>(0., baseValue + (i * valueStep) + m_valueShift, 1.);

        if (qAbs(i) <= 5 && m_gradient) {
            hue = baseHue;
            saturation=baseSaturation;
            value=baseValue;
        }

        QRect patchRect(z * (patchWidth + patchSpacing), 0, patchWidth, m_lineHeight);
        KoColor patchColor = m_parentProxy->converter()->fromHsvF(hue, saturation, value);

        patchColor.convertTo(m_realPixelCache->colorSpace());
        m_realPixelCache->fill(patchRect, patchColor);

        z++;
    }

    QPainter wpainter(this);
    QImage renderedImage = m_parentProxy->converter()->toQImage(m_realPixelCache);
    wpainter.drawImage(0, 0, renderedImage);
    if (m_gradient) {
        wpainter.setPen(QColor(175,175,175));
        wpainter.drawRect(renderedImage.width()/2-5,0,10,renderedImage.height()-1);
        wpainter.setPen(QColor(75,75,75));
        wpainter.drawRect(renderedImage.width()/2-4,0,8,renderedImage.height()-1);
        wpainter.setPen(QColor(175,175,175));
        qreal mouseX = (qBound(5.0,m_mouseX,m_width-5));
        wpainter.drawRect(mouseX-5,0,10,renderedImage.height()-1);
        wpainter.setPen(QColor(75,75,75));
        wpainter.drawRect(mouseX-4,0,8,renderedImage.height()-1);
    }
    m_width=width();

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

void KisShadeSelectorLine::resizeEvent(QResizeEvent *e)
{
    QSize m_old = e->oldSize();
    QSize m_new = e->size();
    int m_nWidth = m_new.width();
    int m_oWidth = m_old.width();
    m_width = width();
    m_mouseX = m_mouseX * m_nWidth / m_oWidth;
}
void KisShadeSelectorLine::mousePressEvent(QMouseEvent* e)
{
    if(e->button()!=Qt::LeftButton && e->button()!=Qt::RightButton) {
        e->setAccepted(false);
        return;
    }
    if (e->y() > 0 && e->y() < height()) {
        m_parentProxy->showColorPreview();
        e->accept();
        m_mouseX = e->x();
        m_isDown=true;
        update();
     }
}

void KisShadeSelectorLine::mouseMoveEvent(QMouseEvent *e)
{
    if ((m_isDown) && (e->buttons() & Qt::LeftButton)) {
        m_mouseX=e->x();
        const QPoint mouseEv ((qBound(5.0,m_mouseX,m_width-5)),5);
        KoColor color(Acs::sampleColor(m_realPixelCache, mouseEv));
        m_parentProxy->updateColorPreview(color);
        update();
    }
}

void KisShadeSelectorLine::mouseReleaseEvent(QMouseEvent * e)
{
    if (e->button() != Qt::LeftButton && e->button() != Qt::RightButton) {
        e->ignore();
        return;
    }
    m_mouseX=e->x();
    const QPoint mouseEv ((qBound(5.0,m_mouseX,m_width-5)),5);
    KoColor color(Acs::sampleColor(m_realPixelCache, mouseEv));
    m_parentProxy->updateColorPreview(color);
    Acs::ColorRole role = Acs::buttonToRole(e->button());

    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");

    bool onRightClick = cfg.readEntry("shadeSelectorUpdateOnRightClick", false);
    bool onLeftClick = cfg.readEntry("shadeSelectorUpdateOnLeftClick", false);

    bool explicitColorReset =
        (e->button() == Qt::LeftButton && onLeftClick) ||
        (e->button() == Qt::RightButton && onRightClick);

    m_parentProxy->updateColor(color, role, explicitColorReset);
    e->accept();
    m_isDown=false;
}
