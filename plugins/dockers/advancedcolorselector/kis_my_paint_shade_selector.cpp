/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *  SPDX-FileCopyrightText: 2008 Martin Renold <martinxyz@gmx.ch>
 *  SPDX-FileCopyrightText: 2009 Ilya Portnov <nomail>
 *
 *  This class is based on "lib/colorchanger.hpp" from MyPaint (mypaint.intilinux.com)
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_my_paint_shade_selector.h"

#include <cmath>
#include <cstdlib>

#include <QImage>
#include <QColor>
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QtGlobal>
#include <QTimer>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"
#include "KoColor.h"
#include "KoCanvasResourceProvider.h"

#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_display_color_converter.h"

inline int sqr(int x);
inline qreal sqr2(qreal x);
inline int signedSqr(int x);


KisMyPaintShadeSelector::KisMyPaintShadeSelector(QWidget *parent) :
        KisColorSelectorBase(parent),
        m_updateTimer(new QTimer(this))
{
    setAcceptDrops(true);

    updateSettings();

    setMinimumSize(80, 80);
    setColor(KoColor(Qt::red, colorSpace()));

    m_updateTimer->setInterval(1);
    m_updateTimer->setSingleShot(true);
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(update()));
}

void KisMyPaintShadeSelector::paintEvent(QPaintEvent *) {
    // Hint to the casual reader: some of the calculation here do not
    // what Martin Renold originally intended. Not everything here will make sense.
    // It does not matter in the end, as long as the result looks good.

    // This selector was ported from MyPaint in 2010
    if (m_cachedColorSpace != colorSpace()) {
        m_realPixelCache = new KisPaintDevice(colorSpace());
        m_realCircleBorder = new KisPaintDevice(colorSpace());
        m_cachedColorSpace = colorSpace();
    }
    else {
        m_realPixelCache->clear();
        m_realCircleBorder->clear();
    }
	KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");
	QString shadeMyPaintType=cfg.readEntry("shadeMyPaintType", "HSV");

    int sizeHD = qMin(width(), height())*devicePixelRatioF();
    int s_radiusHD = sizeHD/2.6;

    int widthHD = width()*devicePixelRatioF();
    int heightHD = height()*devicePixelRatioF();

    const int pixelSize = m_realPixelCache->colorSpace()->pixelSize();
    const int borderPixelSize = m_realCircleBorder->colorSpace()->pixelSize();

    QRect pickRectHighDPI = QRect(QPoint(0, 0), size()*devicePixelRatioF());
    KisSequentialIterator it(m_realPixelCache, pickRectHighDPI);
    KisSequentialIterator borderIt(m_realCircleBorder, pickRectHighDPI);


    while(it.nextPixel()) {
        borderIt.nextPixel();

        int x = it.x();
        int y = it.y();

        float v_factor = 0.6f;
        float s_factor = 0.6f;
        float v_factor2 = 0.013f;
        float s_factor2 = 0.013f;

        int stripe_width = 15*sizeHD/255.;

        float h = 0;
        float s = 0;
        float v = 0;

        int dx = x-widthHD/2;
        int dy = y-heightHD/2;
        int diag = sqrt(2.0)*sizeHD/2;

        int dxs, dys;
        if (dx > 0)
            dxs = dx - stripe_width;
        else
            dxs = dx + stripe_width;
        if (dy > 0)
            dys = dy - stripe_width;
        else
            dys = dy + stripe_width;

        qreal r = std::sqrt(qreal(sqr(dxs)+sqr(dys)));

        if (qMin(abs(dx), abs(dy)) < stripe_width) {
            // horizontal and vertical lines
            bool horizontal = std::abs(dx) > std::abs(dy);
            dx = (dx/qreal(sizeHD))*255;
            dy = (dy/qreal(sizeHD))*255;

            h = 0;
            // x-axis = value, y-axis = saturation
            v =    dx*v_factor + signedSqr(dx)*v_factor2;
            s = - (dy*s_factor + signedSqr(dy)*s_factor2);
            // but not both at once
            if (horizontal) {
                // horizontal stripe
                s = 0.0;
            } else {
                // vertical stripe
                v = 0.0;
            }
        }
        else if (std::min(std::abs(dx - dy), std::abs(dx + dy)) < stripe_width) {

            dx = (dx/qreal(sizeHD))*255;
            dy = (dy/qreal(sizeHD))*255;

            h = 0;
            // x-axis = value, y-axis = saturation
            v =    dx*v_factor + signedSqr(dx)*v_factor2;
            s = - (dy*s_factor + signedSqr(dy)*s_factor2);
            // both at once
        }
        else if (r < s_radiusHD+1) {
            // hue
            if (dx > 0)
                h = 90*sqr2(r/s_radiusHD);
            else
                h = 360 - 90*sqr2(r/s_radiusHD);
            s = 256*(atan2f(std::abs(dxs),dys)/M_PI) - 128;

            if (r > s_radiusHD) {
                // antialiasing boarder
                qreal aaFactor = r-floor(r); // part after the decimal point
                aaFactor = 1-aaFactor;

                qreal fh = m_colorH + h/360.0;
                qreal fs = m_colorS + s/255.0;
                qreal fv = m_colorV + v/255.0;

                fh -= floor(fh);
                fs = qBound(qreal(0.0), fs, qreal(1.0));
                fv = qBound(qreal(0.01), fv, qreal(1.0));
                KoColor color;
                //KoColor color = converter()->fromHsvF(fh, fs, fv);
                if(shadeMyPaintType=="HSV"){color = converter()->fromHsvF(fh, fs, fv);}
                else if(shadeMyPaintType=="HSL"){color = converter()->fromHslF(fh, fs, fv);}
                else if(shadeMyPaintType=="HSI"){color = converter()->fromHsiF(fh, fs, fv);}
                else if(shadeMyPaintType=="HSY"){color = converter()->fromHsyF(fh, fs, fv, R, G, B);}
                else{dbgKrita<<"MyPaint Color selector don't work right.";
                color = converter()->fromHsvF(fh, fs, fv);}
//dbgKrita<<color->toQcolor();
                color.setOpacity(aaFactor);
                Acs::setColorWithIterator(borderIt, color, borderPixelSize);

                h = 180 + 180*atan2f(dys,-dxs)/M_PI;
                v = 255*(r-s_radiusHD)/(diag-s_radiusHD) - 128;
                s = 0; // overwrite the s value that was meant for the inside of the circle
                // here we already have drawn the inside, and the value left should be just the background value

            }
        }
        else {
            // background (hue+darkness gradient)
            h = 180 + 180*atan2f(dys,-dxs)/M_PI;
            v = 255*(r-s_radiusHD)/(diag-s_radiusHD) - 128;
        }

        qreal fh = m_colorH + h/360.0;
        qreal fs = m_colorS + s/255.0;
        qreal fv = m_colorV + v/255.0;

        fh -= floor(fh);
        fs = qBound(qreal(0.0), fs, qreal(1.0));
        fv = qBound(qreal(0.01), fv, qreal(1.0));
        KoColor color;
        //KoColor color = converter()->fromHsvF(fh, fs, fv);
        if(shadeMyPaintType=="HSV"){color = converter()->fromHsvF(fh, fs, fv);}
        else if(shadeMyPaintType=="HSL"){color = converter()->fromHslF(fh, fs, fv);}
        else if(shadeMyPaintType=="HSI"){color = converter()->fromHsiF(fh, fs, fv);}
        else if(shadeMyPaintType=="HSY"){color = converter()->fromHsyF(fh, fs, fv);}
        else{dbgKrita<<"MyPaint Color selector don't work right.";
        color = converter()->fromHsvF(fh, fs, fv);}

        Acs::setColorWithIterator(it, color, pixelSize);
    }

    KisPainter gc(m_realPixelCache);
    gc.bitBlt(QPoint(0,0), m_realCircleBorder, QRect(rect().topLeft(), rect().size()*devicePixelRatioF()));

    QPainter painter(this);
    QImage renderedImage = converter()->toQImage(m_realPixelCache);
    renderedImage.setDevicePixelRatio(devicePixelRatioF());

    painter.drawImage(0, 0, renderedImage);
}


void KisMyPaintShadeSelector::mousePressEvent(QMouseEvent* e)
{
    e->setAccepted(false);
    KisColorSelectorBase::mousePressEvent(e);

    if (!e->isAccepted()) {
        if(rect().contains(e->pos())) {
            KoColor color(Acs::sampleColor(m_realPixelCache, e->pos()*devicePixelRatioF()));
            this->updateColorPreview(color);
            updatePreviousColorPreview();
        }
    }
}

void KisMyPaintShadeSelector::mouseMoveEvent(QMouseEvent *e)
{
    if(rect().contains(e->pos())) {
        KoColor color(Acs::sampleColor(m_realPixelCache, e->pos()*devicePixelRatioF()));
        this->updateColorPreview(color);
    }
    KisColorSelectorBase::mouseMoveEvent(e);
}

void KisMyPaintShadeSelector::mouseReleaseEvent(QMouseEvent *e)
{
    e->setAccepted(false);
    KisColorSelectorBase::mouseReleaseEvent(e);

    if(!e->isAccepted()) {
        KoColor color(Acs::sampleColor(m_realPixelCache, e->pos()*devicePixelRatioF()));

        Acs::ColorRole role = Acs::buttonToRole(e->button());

        KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");

        bool onRightClick = cfg.readEntry("shadeSelectorUpdateOnRightClick", false);
        bool onLeftClick = cfg.readEntry("shadeSelectorUpdateOnLeftClick", false);

        bool explicitColorReset =
            (e->button() == Qt::LeftButton && onLeftClick) ||
            (e->button() == Qt::RightButton && onRightClick);

        this->updateColor(color, role, explicitColorReset);
        updateBaseColorPreview(color);
        e->accept();
    }
}

KisColorSelectorBase* KisMyPaintShadeSelector::createPopup() const
{
    KisColorSelectorBase* popup = new KisMyPaintShadeSelector(0);
    popup->setColor(m_lastRealColor);
    return popup;
}

void KisMyPaintShadeSelector::setColor(const KoColor &color) {

	KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");
	QString shadeMyPaintType=cfg.readEntry("shadeMyPaintType", "HSV");

    R = cfg.readEntry("lumaR", 0.2126);
    G = cfg.readEntry("lumaG", 0.7152);
    B = cfg.readEntry("lumaB", 0.0722);

	if(shadeMyPaintType=="HSV"){this->converter()->getHsvF(color, &m_colorH, &m_colorS, &m_colorV);}
	if(shadeMyPaintType=="HSL"){this->converter()->getHslF(color, &m_colorH, &m_colorS, &m_colorV);}	
	if(shadeMyPaintType=="HSI"){this->converter()->getHsiF(color, &m_colorH, &m_colorS, &m_colorV);}	
	if(shadeMyPaintType=="HSY"){this->converter()->getHsyF(color, &m_colorH, &m_colorS, &m_colorV, R, G, B);}
    m_lastRealColor = color;
    this->updateColorPreview(color);

    m_updateTimer->start();
}

void KisMyPaintShadeSelector::canvasResourceChanged(int key, const QVariant &v)
{
    if(m_colorUpdateAllowed==false)
        return;

    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");

    bool onForeground = cfg.readEntry("shadeSelectorUpdateOnForeground", false);
    bool onBackground = cfg.readEntry("shadeSelectorUpdateOnBackground", true);

    if ((key == KoCanvasResource::ForegroundColor && onForeground) ||
        (key == KoCanvasResource::BackgroundColor && onBackground)) {

        setColor(v.value<KoColor>());
    }
}

inline int sqr(int x) {
    return x*x;
}

inline qreal sqr2(qreal x) {
    return (x*x)/2+x/2;
}

inline int signedSqr(int x) {
    int sign = x>0?1:-1;
    return x*x*sign;
}
