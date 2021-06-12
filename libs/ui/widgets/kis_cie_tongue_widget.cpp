/* 
 * SPDX-FileCopyrightText: 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * Based on the Digikam CIE Tongue widget
 * SPDX-FileCopyrightText: 2006-2013 Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Any source code are inspired from lprof project and
 * SPDX-FileCopyrightText: 1998-2001 Marti Maria
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 **/

/**
The following table gives the CIE  color matching functions
\f$\bar{x}(\lambda)\f$, \f$\bar{y}(\lambda)\f$, and
\f$\bar{z}(\lambda)\f$, for wavelengths \f$\lambda\f$ at 5 nanometer
increments from 380 nm through 780 nm. This table is used in conjunction
with Planck's law for the energy spectrum of a black body at a given
temperature to plot the black body curve on the CIE chart.
 
The following table gives the spectral chromaticity coordinates
\f$x(\lambda)\f$ and \f$y(\lambda)\f$ for wavelengths in 5 nanometer
increments from 380 nm through 780 nm. These coordinates represent the
position in the CIE x-y space of pure spectral colors of the given
wavelength, and thus define the outline of the CIE "tongue" diagram.
*/

#include <QPointF>
#include <QPainter>
#include <QPainterPath>
#include <QFile>
#include <QTimer>
#include <QPaintEvent>
#include <QImage>
#include <cmath>

#include <klocalizedstring.h>
#include <kpixmapsequence.h>

#include <kis_icon.h>
#include <KoColorSpaceRegistry.h>

#include "kis_cie_tongue_widget.h"

static const double spectral_chromaticity[81][3] =
{
    { 0.1741, 0.0050 },               // 380 nm
    { 0.1740, 0.0050 },
    { 0.1738, 0.0049 },
    { 0.1736, 0.0049 },
    { 0.1733, 0.0048 },
    { 0.1730, 0.0048 },
    { 0.1726, 0.0048 },
    { 0.1721, 0.0048 },
    { 0.1714, 0.0051 },
    { 0.1703, 0.0058 },
    { 0.1689, 0.0069 },
    { 0.1669, 0.0086 },
    { 0.1644, 0.0109 },
    { 0.1611, 0.0138 },
    { 0.1566, 0.0177 },
    { 0.1510, 0.0227 },
    { 0.1440, 0.0297 },
    { 0.1355, 0.0399 },
    { 0.1241, 0.0578 },
    { 0.1096, 0.0868 },
    { 0.0913, 0.1327 },
    { 0.0687, 0.2007 },
    { 0.0454, 0.2950 },
    { 0.0235, 0.4127 },
    { 0.0082, 0.5384 },
    { 0.0039, 0.6548 },
    { 0.0139, 0.7502 },
    { 0.0389, 0.8120 },
    { 0.0743, 0.8338 },
    { 0.1142, 0.8262 },
    { 0.1547, 0.8059 },
    { 0.1929, 0.7816 },
    { 0.2296, 0.7543 },
    { 0.2658, 0.7243 },
    { 0.3016, 0.6923 },
    { 0.3373, 0.6589 },
    { 0.3731, 0.6245 },
    { 0.4087, 0.5896 },
    { 0.4441, 0.5547 },
    { 0.4788, 0.5202 },
    { 0.5125, 0.4866 },
    { 0.5448, 0.4544 },
    { 0.5752, 0.4242 },
    { 0.6029, 0.3965 },
    { 0.6270, 0.3725 },
    { 0.6482, 0.3514 },
    { 0.6658, 0.3340 },
    { 0.6801, 0.3197 },
    { 0.6915, 0.3083 },
    { 0.7006, 0.2993 },
    { 0.7079, 0.2920 },
    { 0.7140, 0.2859 },
    { 0.7190, 0.2809 },
    { 0.7230, 0.2770 },
    { 0.7260, 0.2740 },
    { 0.7283, 0.2717 },
    { 0.7300, 0.2700 },
    { 0.7311, 0.2689 },
    { 0.7320, 0.2680 },
    { 0.7327, 0.2673 },
    { 0.7334, 0.2666 },
    { 0.7340, 0.2660 },
    { 0.7344, 0.2656 },
    { 0.7346, 0.2654 },
    { 0.7347, 0.2653 },
    { 0.7347, 0.2653 },
    { 0.7347, 0.2653 },
    { 0.7347, 0.2653 },
    { 0.7347, 0.2653 },
    { 0.7347, 0.2653 },
    { 0.7347, 0.2653 },
    { 0.7347, 0.2653 },
    { 0.7347, 0.2653 },
    { 0.7347, 0.2653 },
    { 0.7347, 0.2653 },
    { 0.7347, 0.2653 },
    { 0.7347, 0.2653 },
    { 0.7347, 0.2653 },
    { 0.7347, 0.2653 },
    { 0.7347, 0.2653 },
    { 0.7347, 0.2653 }  // 780 nm
};

class Q_DECL_HIDDEN KisCIETongueWidget::Private
{
public:
 
    Private() :
        profileDataAvailable(false),
        needUpdatePixmap(false),
        cieTongueNeedsUpdate(true),
        uncalibratedColor(false),
        xBias(0),
        yBias(0),
        pxcols(0),
        pxrows(0),
        progressCount(0),
        gridside(0),
        progressTimer(0),
        Primaries(9),
        whitePoint(3)
        
    {
        progressPix = KPixmapSequence("process-working", KisIconUtils::SizeSmallMedium);
    }
    
    bool            profileDataAvailable;
    bool            needUpdatePixmap;
    bool            cieTongueNeedsUpdate;
    bool            uncalibratedColor;
 
    int             xBias;
    int             yBias;
    int             pxcols;
    int             pxrows;
    int             progressCount;           // Position of animation during loading/calculation.
 
    double          gridside;
 
    QPainter        painter;
 
    QTimer*         progressTimer;
 
    QPixmap         pixmap;
    QPixmap         cietongue;
    QPixmap         gamutMap;
    KPixmapSequence progressPix;
 
    QVector <double> Primaries;
    QVector <double> whitePoint;
    QPolygonF       gamut;
    model colorModel;
};

KisCIETongueWidget::KisCIETongueWidget(QWidget *parent) :
    QWidget(parent), d(new Private)
{
    d->progressTimer = new QTimer(this);
    setAttribute(Qt::WA_DeleteOnClose);
    
    d->Primaries.resize(9);
    d->Primaries.fill(0.0);
    d->whitePoint.resize(3);
    d->whitePoint<<0.34773<<0.35952<<1.0;
    d->gamut = QPolygonF();

    connect(d->progressTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimerDone()));
}

KisCIETongueWidget::~KisCIETongueWidget()
{
    delete d;
}

int KisCIETongueWidget::grids(double val) const
{
    return (int) floor(val * d->gridside + 0.5);
}

void KisCIETongueWidget::setProfileData(QVector <double> p, QVector <double> w, bool profileData)
{
    d->profileDataAvailable = profileData;
    if (profileData){
        d->Primaries= p;
        
        d->whitePoint = w;
        d->needUpdatePixmap = true;      
    } else {
        return;
    }
}
void KisCIETongueWidget::setGamut(QPolygonF gamut)
{
    d->gamut=gamut;
}
void KisCIETongueWidget::setRGBData(QVector <double> whitepoint, QVector <double> colorants)
{
    if (colorants.size()==9){
        d->Primaries= colorants;
        
        d->whitePoint = whitepoint;
        d->needUpdatePixmap = true;
        d->colorModel = KisCIETongueWidget::RGBA;
        d->profileDataAvailable = true;
    } else {
        return;
    }
}
void KisCIETongueWidget::setCMYKData(QVector <double> whitepoint)
{
    if (whitepoint.size()==3){
        //d->Primaries= colorants;
        
        d->whitePoint = whitepoint;
        d->needUpdatePixmap = true;
        d->colorModel = KisCIETongueWidget::CMYKA;
        d->profileDataAvailable = true;
    } else {
        return;
    }
}
void KisCIETongueWidget::setXYZData(QVector <double> whitepoint)
{
    if (whitepoint.size()==3){
        d->whitePoint = whitepoint;
        d->needUpdatePixmap = true;
        d->colorModel = KisCIETongueWidget::XYZA;
        d->profileDataAvailable = true;
    } else {
        return;
    }
}
void KisCIETongueWidget::setGrayData(QVector <double> whitepoint)
{
    if (whitepoint.size()==3){
        d->whitePoint = whitepoint;
        d->needUpdatePixmap = true;
        d->colorModel = KisCIETongueWidget::GRAYA;
        d->profileDataAvailable = true;
    } else {
        return;
    }
}
void KisCIETongueWidget::setLABData(QVector <double> whitepoint)
{
    if (whitepoint.size()==3){
        d->whitePoint = whitepoint;
        d->needUpdatePixmap = true;
        d->colorModel = KisCIETongueWidget::LABA;
        d->profileDataAvailable = true;
    } else {
        return;
    }
}
void KisCIETongueWidget::setYCbCrData(QVector <double> whitepoint)
{
    if (whitepoint.size()==3){
        d->whitePoint = whitepoint;
        d->needUpdatePixmap = true;
        d->colorModel = KisCIETongueWidget::YCbCrA;
        d->profileDataAvailable = true;
    } else {
        return;
    }
}

void KisCIETongueWidget::setProfileDataAvailable(bool dataAvailable)
{
    d->profileDataAvailable = dataAvailable;
}
void KisCIETongueWidget::mapPoint(int& icx, int& icy, QPointF xy)
{
    icx = (int) floor((xy.x() * (d->pxcols - 1)) + .5);
    icy = (int) floor(((d->pxrows - 1) - xy.y() * (d->pxrows - 1)) + .5);
}
 
void KisCIETongueWidget::biasedLine(int x1, int y1, int x2, int y2)
{
    d->painter.drawLine(x1 + d->xBias, y1, x2 + d->xBias, y2);
}
 
void KisCIETongueWidget::biasedText(int x, int y, const QString& txt)
{
    d->painter.drawText(QPoint(d->xBias + x, y), txt);
}
 
QRgb KisCIETongueWidget::colorByCoord(double x, double y)
{
    // Get xyz components scaled from coordinates
 
    double cx =       ((double) x) / (d->pxcols - 1);
    double cy = 1.0 - ((double) y) / (d->pxrows - 1);
    double cz = 1.0 - cx - cy;
 
    // Project xyz to XYZ space. Note that in this
    // particular case we are substituting XYZ with xyz
    
    //Need to use KoColor here.
    const KoColorSpace* xyzColorSpace = KoColorSpaceRegistry::instance()->colorSpace("XYZA", "U8");
    quint8 data[4]; 
    data[0]= cx*255;
    data[1]= cy*255;
    data[2]= cz*255;
    data[3]= 1.0*255;
    KoColor colXYZ(data, xyzColorSpace);
    QColor colRGB = colXYZ.toQColor();
    return qRgb(colRGB.red(), colRGB.green(), colRGB.blue());
}
 
void KisCIETongueWidget::outlineTongue()
{
    int lx=0, ly=0;
    int fx=0, fy=0;
 
    for (int x = 380; x <= 700; x += 5) {
        int ix = (x - 380) / 5;

        QPointF p(spectral_chromaticity[ix][0], spectral_chromaticity[ix][1]);
        int icx, icy;
        mapPoint(icx, icy, p);
 
        if (x > 380) {
            biasedLine(lx, ly, icx, icy);
        }
        else {
            fx = icx;
            fy = icy;
        }
 
        lx = icx;
        ly = icy;

    }
 
    biasedLine(lx, ly, fx, fy);
}
 
void KisCIETongueWidget::fillTongue()
{
    QImage Img = d->cietongue.toImage();
 
    int x;
 
    for (int y = 0; y < d->pxrows; ++y)
    {
        int xe = 0;
 
        // Find horizontal extents of tongue on this line.
 
        for (x = 0; x < d->pxcols; ++x)
        {
            if (QColor(Img.pixel(x + d->xBias, y)) != QColor(Qt::black))
            {
                for (xe = d->pxcols - 1; xe >= x; --xe)
                {
                    if (QColor(Img.pixel(xe + d->xBias, y)) != QColor(Qt::black))
                    {
                        break;
                    }
                }
 
                break;
            }
        }
 
        if (x < d->pxcols)
        {
            for ( ; x <= xe; ++x)
            {
                QRgb Color = colorByCoord(x, y);
                Img.setPixel(x + d->xBias, y, Color);
            }
        }
    }
 
    d->cietongue = QPixmap::fromImage(Img, Qt::AvoidDither);
}
 
void KisCIETongueWidget::drawTongueAxis()
{
    QFont font;
    font.setPointSize(6);
    d->painter.setFont(font);
 
    d->painter.setPen(qRgb(255, 255, 255));
 
    biasedLine(0, 0,           0,           d->pxrows - 1);
    biasedLine(0, d->pxrows-1, d->pxcols-1, d->pxrows - 1);
 
    for (int y = 1; y <= 9; y += 1)
    {
        QString s;
        int xstart = (y * (d->pxcols - 1)) / 10;
        int ystart = (y * (d->pxrows - 1)) / 10;
 
        QTextStream(&s) << y;
        biasedLine(xstart, d->pxrows - grids(1), xstart,   d->pxrows - grids(4));
        biasedText(xstart - grids(11), d->pxrows + grids(15), s);
 
        QTextStream(&s) << 10 - y;
        biasedLine(0, ystart, grids(3), ystart);
        biasedText(grids(-25), ystart + grids(5), s);
    }
}
 
void KisCIETongueWidget::drawTongueGrid()
{
    d->painter.setPen(qRgb(128, 128, 128));
    d->painter.setOpacity(0.5);
 
    for (int y = 1; y <= 9; y += 1)
    {
        int xstart =  (y * (d->pxcols - 1)) / 10;
        int ystart =  (y * (d->pxrows - 1)) / 10;
 
        biasedLine(xstart, grids(4), xstart,   d->pxrows - grids(4) - 1);
        biasedLine(grids(7), ystart, d->pxcols-1-grids(7), ystart);
    }
    d->painter.setOpacity(1.0);
}
 
void KisCIETongueWidget::drawLabels()
{
    QFont font;
    font.setPointSize(5);
    d->painter.setFont(font);
 
    for (int x = 450; x <= 650; x += (x > 470 && x < 600) ? 5 : 10)
    {
        QString wl;
        int bx = 0, by = 0, tx, ty;
 
        if (x < 520)
        {
            bx = grids(-22);
            by = grids(2);
        }
        else if (x < 535)
        {
            bx = grids(-8);
            by = grids(-6);
        }
        else
        {
            bx = grids(4);
        }
 
        int ix = (x - 380) / 5;
 
        QPointF p(spectral_chromaticity[ix][0], spectral_chromaticity[ix][1]);
 
        int icx, icy;
        mapPoint(icx, icy, p);
 
        tx = icx + ((x < 520) ? grids(-2) : ((x >= 535) ? grids(2) : 0));
        ty = icy + ((x < 520) ? 0 : ((x >= 535) ? grids(-1) : grids(-2)));
 
        d->painter.setPen(qRgb(255, 255, 255));
        biasedLine(icx, icy, tx, ty);
 
        QRgb Color = colorByCoord(icx, icy);
        d->painter.setPen(Color);
 
        QTextStream(&wl) << x;
        biasedText(icx+bx, icy+by, wl);
    }
}
 
void KisCIETongueWidget::drawSmallEllipse(QPointF xy, int r, int g, int b, int sz)
{
    int icx, icy;
 
    mapPoint(icx, icy, xy);
    d->painter.save();
    d->painter.setRenderHint(QPainter::Antialiasing);
    d->painter.setPen(qRgb(r, g, b));
    d->painter.drawEllipse(icx + d->xBias- sz/2, icy-sz/2, sz, sz);
    d->painter.setPen(qRgb(r/2, g/2, b/2));
    int sz2 = sz-2;
    d->painter.drawEllipse(icx + d->xBias- sz2/2, icy-sz2/2, sz2, sz2);
    d->painter.restore();
}
 
void KisCIETongueWidget::drawColorantTriangle()
{
    d->painter.save();
    d->painter.setPen(qRgb(80, 80, 80));
    d->painter.setRenderHint(QPainter::Antialiasing);
    if (d->colorModel ==KisCIETongueWidget::RGBA) {
        drawSmallEllipse((QPointF(d->Primaries[0],d->Primaries[1])),   255, 128, 128, 6);
        drawSmallEllipse((QPointF(d->Primaries[3],d->Primaries[4])), 128, 255, 128, 6);
        drawSmallEllipse((QPointF(d->Primaries[6],d->Primaries[7])),  128, 128, 255, 6);
        
        int x1, y1, x2, y2, x3, y3;
 
        mapPoint(x1, y1, (QPointF(d->Primaries[0],d->Primaries[1])) );
        mapPoint(x2, y2, (QPointF(d->Primaries[3],d->Primaries[4])) );
        mapPoint(x3, y3, (QPointF(d->Primaries[6],d->Primaries[7])) );
        
        biasedLine(x1, y1, x2, y2);
        biasedLine(x2, y2, x3, y3);
        biasedLine(x3, y3, x1, y1);
    } /*else if (d->colorModel ==CMYK){
        for (i=0; i<d->Primaries.size();i+++){
            drawSmallEllipse((QPointF(d->Primaries[0],d->Primaries[1])),   160, 160, 160, 6);//greyscale for now
            //int x1, y1, x2, y2;
            //mapPoint(x1, y1, (QPointF(d->Primaries[i],d->Primaries[i+1])) );
            //mapPoint(x2, y2, (QPointF(d->Primaries[i+3],d->Primaries[i+4])) );
            //biasedLine(x1, y1, x2, y2);
        }
    }
    */
 
    d->painter.restore();
}
 
void KisCIETongueWidget::drawWhitePoint()
{
    drawSmallEllipse(QPointF (d->whitePoint[0],d->whitePoint[1]),  255, 255, 255, 8);
}

void KisCIETongueWidget::drawGamut()
{
    d->gamutMap=QPixmap(size());
    d->gamutMap.fill(Qt::black);
    QPainter gamutPaint;
    gamutPaint.begin(&d->gamutMap);
    QPainterPath path;
    //gamutPaint.setCompositionMode(QPainter::CompositionMode_Clear);
    gamutPaint.setRenderHint(QPainter::Antialiasing);
    path.setFillRule(Qt::WindingFill);
    gamutPaint.setBrush(Qt::white);
    gamutPaint.setPen(Qt::white);
    int x, y = 0;
    if (!d->gamut.empty()) {
        gamutPaint.setOpacity(0.5);
        if (d->colorModel == KisCIETongueWidget::RGBA) {
            mapPoint(x, y, (QPointF(d->Primaries[0],d->Primaries[1])) );
            path.moveTo(QPointF(x + d->xBias,y));
            mapPoint(x, y, (QPointF(d->Primaries[3],d->Primaries[4])) );
            path.lineTo(QPointF(x + d->xBias,y));
            mapPoint(x, y, (QPointF(d->Primaries[6],d->Primaries[7])) );
            path.lineTo(QPointF(x + d->xBias,y));
            mapPoint(x, y, (QPointF(d->Primaries[0],d->Primaries[1])) );
            path.lineTo(QPointF(x + d->xBias,y));
        }
        gamutPaint.drawPath(path);
        gamutPaint.setOpacity(1.0);
        foreach (QPointF Point, d->gamut) {
            mapPoint(x, y, Point);
            gamutPaint.drawEllipse(x + d->xBias- 2, y-2, 4, 4);
            //Point.setX(x);
            //Point.setY(y);
            //path.lineTo(Point);
        }
    }
    
    gamutPaint.end();
    d->painter.save();
    d->painter.setOpacity(0.5);
    d->painter.setCompositionMode(QPainter::CompositionMode_Multiply);
    QRect area(d->xBias, 0, d->pxcols, d->pxrows);
    d->painter.drawPixmap(area,d->gamutMap, area);
    d->painter.setOpacity(1.0);
    d->painter.restore();
}

void KisCIETongueWidget::updatePixmap()
{
    d->needUpdatePixmap = false;
    d->pixmap = QPixmap(size());

    if (d->cieTongueNeedsUpdate){
    // Draw the CIE tongue curve. I don't see why we need to redraw it every time the whitepoint and such changes so we cache it.
        d->cieTongueNeedsUpdate = false;
        d->cietongue = QPixmap(size());
        d->cietongue.fill(Qt::black);
        d->painter.begin(&d->cietongue);
 
        int pixcols = d->pixmap.width();
        int pixrows = d->pixmap.height();

        d->gridside = (qMin(pixcols, pixrows)) / 512.0;
        d->xBias    = grids(32);
        d->yBias    = grids(20);
        d->pxcols   = pixcols - d->xBias;
        d->pxrows   = pixrows - d->yBias;

        d->painter.setBackground(QBrush(qRgb(0, 0, 0)));
        d->painter.setPen(qRgb(255, 255, 255));

        outlineTongue();
        d->painter.end();
    
        fillTongue();
    
        d->painter.begin(&d->cietongue);
        drawTongueAxis();
        drawLabels();
        drawTongueGrid();
        d->painter.end();
    }
    d->pixmap = d->cietongue;

    d->painter.begin(&d->pixmap);
    //draw whitepoint and  colorants
    if (d->whitePoint[2] > 0.0)
    {
        drawWhitePoint();
    }
 
    if (d->Primaries[2] != 0.0)
    {
        drawColorantTriangle();
    }
    drawGamut();

    d->painter.end();
}
 
void KisCIETongueWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
 
    // Widget is disable : drawing grayed frame.
 
    if ( !isEnabled() )
    {
        p.fillRect(0, 0, width(), height(),
                   palette().color(QPalette::Disabled, QPalette::Background));
 
        QPen pen(palette().color(QPalette::Disabled, QPalette::Foreground));
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(1);
 
        p.setPen(pen);
        p.drawRect(0, 0, width(), height());
 
        return;
    }

 
    // No profile data to show, or RAW file
 
    if (!d->profileDataAvailable)
    {
        p.fillRect(0, 0, width(), height(), palette().color(QPalette::Active, QPalette::Background));
        QPen pen(palette().color(QPalette::Active, QPalette::Text));
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(1);
 
        p.setPen(pen);
        p.drawRect(0, 0, width(), height());
 
        if (d->uncalibratedColor)
        {
            p.drawText(0, 0, width(), height(), Qt::AlignCenter,
                       i18n("Uncalibrated color space"));
        }
        else
        {
            p.setPen(Qt::red);
            p.drawText(0, 0, width(), height(), Qt::AlignCenter,
                       i18n("No profile available..."));
        }
 
        return;
    }
 
    // Create CIE tongue if needed
    if (d->needUpdatePixmap)
    {
        updatePixmap();
    }
 
    // draw prerendered tongue
    p.drawPixmap(0, 0, d->pixmap);
}
 
void KisCIETongueWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    setMinimumHeight(width());
    setMaximumHeight(width());
    d->needUpdatePixmap = true;
    d->cieTongueNeedsUpdate = true;
}
 
void KisCIETongueWidget::slotProgressTimerDone()
{
    update();
    d->progressTimer->start(200);
}
