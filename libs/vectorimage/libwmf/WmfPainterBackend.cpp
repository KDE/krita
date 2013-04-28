/* This file is part of the KDE libraries
 *
 * Copyright (c) 2003 thierry lorthiois (lorthioist@wanadoo.fr)
 *               2009-2011 Inge Wallin <inge@lysator.liu.se>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "WmfPainterBackend.h"

#include <QPolygon>
#include <QPrinter>
#include <QFontMetrics>

#include <kdebug.h>

#include "WmfEnums.h"
#include "WmfParser.h"


#define DEBUG_WMFPAINT 0

/**
   Namespace for Windows Metafile (WMF) classes
*/
namespace Libwmf
{


WmfPainterBackend::WmfPainterBackend(QPainter *painter, const QSizeF &outputSize)
    : WmfAbstractBackend()
    , mPainter(painter)
    , mOutputSize(outputSize)
    , mSaveCount(0)
{
    mTarget = painter->device();
    mIsInternalPainter = false;
    mWorldTransform = QTransform();
}

WmfPainterBackend::~WmfPainterBackend()
{
}

#if 0
bool WmfPainterBackend::play(QPaintDevice& target)
{
    if (!mPainter)
        mPainter = new QPainter(&target);
    mIsInternalPainter = true;

    if (mPainter->isActive()) return false;
    mTarget = &target;

    // Play the wmf file
    mSaveCount = 0;
    bool ret = WmfAbstractBackend::play();

    // Make sure that the painter is in the same state as before WmfParser::play()
    for (; mSaveCount > 0; mSaveCount--)
        restore();
    return ret;
}
#endif

bool WmfPainterBackend::play()
{
    // If there is already a painter and it's owned by us, then delete it.
    if (mPainter && mIsInternalPainter)
        delete mPainter;

    mTarget = mPainter->device();

    // Play the wmf file
    bool ret = m_parser->play(this);

    return ret;
}


//-----------------------------------------------------------------------------
// Virtual Painter


bool WmfPainterBackend::begin(const QRect &boundingBox)
{
    // If the painter is our own, we have to call begin() on it.
    // If it's external, we assume that it's already done for us.
    if (mIsInternalPainter) {
        if (!mPainter->begin(mTarget))
            return false;
    }

    // For calculations of window / viewport during the painting
    mWindowOrg = QPoint(0, 0);
    mViewportOrg = QPoint(0, 0);
    mWindowExtIsSet = false;
    mViewportExtIsSet = false;
    mOutputTransform = mPainter->transform();
    mWorldTransform = QTransform();

    //mPainter->setBrush(QBrush(Qt::NoBrush));

#if DEBUG_WMFPAINT
    kDebug(31000) << "Using QPainter: " << mPainter->pen() << mPainter->brush() 
                  << "Background: " << mPainter->background() << " " << mPainter->backgroundMode();
#endif

    qreal  scaleX = mOutputSize.width()  / boundingBox.width();
    qreal  scaleY = mOutputSize.height() / boundingBox.height();

    // Transform the WMF object so that it fits in the shape as much
    // as possible.  The topleft will be the top left of the shape.
    mPainter->scale( scaleX, scaleY );

    mOutputTransform = mPainter->transform();

    mPainter->setRenderHint(QPainter::Antialiasing);
    mPainter->setRenderHint(QPainter::TextAntialiasing);

    mSaveCount = 0;

    return true;
}


bool WmfPainterBackend::end()
{
    // Make sure that the painter is in the same state as before calling play above().
    for (; mSaveCount > 0; mSaveCount--)
        restore();

    bool ret = true;
    if (mIsInternalPainter)
        ret = mPainter->end();

    return ret;
}


void WmfPainterBackend::save()
{
    // A little trick here: Save the worldTransform in the painter.
    // If we didn't do this, we would have to create a separate stack
    // for these.
    //
    // FIXME: We should collect all the parts of the DC that are not
    //        stored in the painter and save them separately.
    QTransform  savedTransform = mPainter->worldTransform();
    mPainter->setWorldTransform(mWorldTransform);

    mPainter->save();
    ++mSaveCount;

    mPainter->setWorldTransform(savedTransform);
}


void WmfPainterBackend::restore()
{
    if (mSaveCount > 0) {
        mPainter->restore();
        mSaveCount--;
    }
    else {
        kDebug(31000) << "restore(): try to restore painter without save";
    }

    // We used a trick in save() and stored the worldTransform in
    // the painter.  Now restore the full transformation.
    mWorldTransform = mPainter->worldTransform();
    recalculateWorldTransform();
}


void WmfPainterBackend::setCompositionMode(QPainter::CompositionMode mode)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << mode << "(ignored)";
#endif

    // FIXME: This doesn't work.  I don't understand why, but when I
    //        enable this all backgrounds become black. /iw
    Q_UNUSED(mode);
    //mPainter->setCompositionMode(mode);
}


// ---------------------------------------------------------------------
//                 World Transform, Window and Viewport


// General note about coordinate spaces and transforms:
//
// There are several coordinate spaces in use when drawing an WMF file:
//  1. The object space, in which the objects' coordinates are expressed inside the WMF.
//     In general there are several of these.
//  2. The page space, which is where they end up being painted in the WMF picture.
//     The union of these form the bounding box of the WMF.
//  3. (possibly) the output space, where the WMF picture itself is placed
//     and/or scaled, rotated, etc
//
// The transform between spaces 1. and 2. is called the World Transform.
// The world transform can be changed either through calls to change
// the window or viewport or through calls to setWorldTransform() or
// modifyWorldTransform().
//
// The transform between spaces 2. and 3. is the transform that the QPainter
// already contains when it is given to us.  We need to save this and reapply
// it after the world transform has changed. We call this transform the Output
// Transform in lack of a better word. (Some sources call it the Device Transform.)
//


// FIXME:
// To change those functions it's better to have
// a large set of WMF files. WMF special case includes :
// - without call to setWindowOrg and setWindowExt
// - change the origin or the scale in the middle of the drawing
// - negative width or height
// and relative/absolute coordinate


void WmfPainterBackend::recalculateWorldTransform()
{
    mWorldTransform = QTransform();

    if (!mWindowExtIsSet && !mViewportExtIsSet)
        return;

    // FIXME: Check windowExt == 0 in any direction
    qreal windowViewportScaleX;
    qreal windowViewportScaleY;
    if (mWindowExtIsSet && mViewportExtIsSet) {
        // Both window and viewport are set.
        windowViewportScaleX = qreal(mViewportExt.width()) / qreal(mWindowExt.width());
        windowViewportScaleY = qreal(mViewportExt.height()) / qreal(mWindowExt.height());
#if DEBUG_WMFPAINT
        kDebug(31000) << "Scale for Window -> Viewport"
                      << windowViewportScaleX << windowViewportScaleY;
#endif
    }
    else {
        // At most one of window and viewport ext is set: Use same width for window and viewport
        windowViewportScaleX = qreal(1.0);
        windowViewportScaleY = qreal(1.0);
#if DEBUG_WMFPAINT
        kDebug(31000) << "Only one of Window or Viewport set: scale window -> viewport = 1";
#endif
    }

    // Negative window extensions mean flip the picture.  Handle this here.
    bool  flip = false;
    qreal midpointX = 0.0;
    qreal midpointY = 0.0;
    qreal scaleX = 1.0;
    qreal scaleY = 1.0;
    if (mWindowExt.width() < 0) {
        midpointX = (mWindowOrg.x() + mWindowExt.width()) / qreal(2.0);
        scaleX = -1.0;
        flip = true;
    }
    if (mWindowExt.height() < 0) {
        midpointY = (mWindowOrg.y() + mWindowExt.height()) / qreal(2.0);
        scaleY = -1.0;
        flip = true;
    }
    if (flip) {
        //kDebug(31000) << "Flipping round midpoint" << midpointX << midpointY << scaleX << scaleY;
        mWorldTransform.translate(midpointX, midpointY);
        mWorldTransform.scale(scaleX, scaleY);
        mWorldTransform.translate(-midpointX, -midpointY);
        //kDebug(31000) << "After flipping for window" << mWorldTransform;
    }

    // Calculate the world transform.
    mWorldTransform.translate(-mWindowOrg.x(), -mWindowOrg.y());
    mWorldTransform.scale(windowViewportScaleX, windowViewportScaleY);
    if (mViewportExtIsSet) {
        mWorldTransform.translate(mViewportOrg.x(), mViewportOrg.y());
    } 
    else {
        // If viewport is not set, but window is *and* the window
        // width/height is negative, then we must compensate for this.
        // If the width/height is positive, we already did it with the
        // first translate before the scale() above.
        if (mWindowExt.width() < 0) 
            mWorldTransform.translate(mWindowOrg.x() + mWindowExt.width(), qreal(0.0));
        if (mWindowExt.height() < 0) 
            mWorldTransform.translate(qreal(0.0), mWindowOrg.y() + mWindowExt.height());
    }
    //kDebug(31000) << "After window viewport calculation" << mWorldTransform;

    // FIXME: also handle negative viewport extensions?  If so, do it here.

    // Apply the world transform to the painter.
    mPainter->setWorldTransform(mWorldTransform);

    // Apply the output transform.
    QTransform currentMatrix = mPainter->worldTransform();
    QTransform newMatrix = currentMatrix * mOutputTransform;
    //kDebug(31000) << "Output transform" << mOutputTransform;
    //kDebug(31000) << "Total  transform" << newMatrix;
    mPainter->setWorldTransform( newMatrix );
}




void WmfPainterBackend::setWindowOrg(int left, int top)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << left << " " << top;
#endif

    // Only set WindowOrg if it changes.  See the Unanswered Question in libemf.
    if (mWindowOrg == QPoint(left, top))
        return;

    mWindowOrg = QPoint(left, top);

    recalculateWorldTransform();

#if 0
    // Debug code.  Draw a rectangle with some decoration to show see
    // if all the transformations work.
    mPainter->save();

    // Paint a black rectangle around the current window.
    mPainter->setPen(Qt::green);
    QRect windowRect = QRect(mWindowOrg, mWindowExt);
    mPainter->drawRect(windowRect);

#if 0
    // Paint a black line from the Window origin to (0, 0)
    mPainter->drawLine(mWindowOrg, QPoint(0, 0));

    mPainter->setPen(Qt::red);
    mPainter->drawRect(boundingRect());

    mPainter->drawLine(boundingRect().topLeft(), QPoint(0, 0));
#endif
    mPainter->restore();

    kDebug(31000) << "Window rect: " << windowRect;
    kDebug(31000) << "Bounding rect: " << boundingRect();
#endif
}


void WmfPainterBackend::setWindowExt(int width, int height)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << width << " " << height;
#endif

    // Only set WindowExt if it changes.  See the Unanswered Question in libemf.
    if (mWindowExt == QSize(width, height))
        return;

    mWindowExt = QSize(width, height);
    mWindowExtIsSet = true;

    recalculateWorldTransform();

#if 0
    // Debug code.  Draw a rectangle with some decoration to show see
    // if all the transformations work.
    mPainter->save();

    // Paint a red rectangle around the current window.
    mPainter->setPen(Qt::red);
    QRect windowRect = QRect(mWindowOrg, mWindowExt);
    mPainter->drawRect(windowRect);

    // Paint a line from the Window origin to (0, 0)
    mPainter->drawLine(mWindowOrg, QPoint(0, 0));

    mPainter->setPen(Qt::black);
    mPainter->drawRect(boundingRect());

    mPainter->drawLine(boundingRect().topLeft(), QPoint(0, 0));
    mPainter->restore();

    kDebug(31000) << "Window rect: " << windowRect;
    kDebug(31000) << "Bounding rect: " << boundingRect();
#endif
}

void WmfPainterBackend::setViewportOrg( int left, int top )
{
#if DEBUG_WMFPAINT
    kDebug(31000) << left << top;
#endif

    // Only set ViewportOrg if it changes.  See the Unanswered Question in libemf.
    if (mViewportOrg == QPoint(left, top))
        return;

    mViewportOrg = QPoint(left, top);

    recalculateWorldTransform();
}

void WmfPainterBackend::setViewportExt( int width, int height )
{
#if DEBUG_WMFPAINT
    kDebug(31000) << width << height;
#endif

    // Only set ViewportOrg if it changes.  See the Unanswered Question in libemf.
    if (mViewportExt == QSize(width, height))
        return;

    mViewportExt = QSize(width, height);
    mViewportExtIsSet = true;

    recalculateWorldTransform();
}


void WmfPainterBackend::setMatrix(WmfDeviceContext &context, const QMatrix &wm, bool combine)
{
    Q_UNUSED(context);
#if DEBUG_WMFPAINT
    kDebug(31000) << wm << " " << combine;
#endif
    mPainter->setMatrix(wm, combine);

    recalculateWorldTransform();
}


// ----------------------------------------------------------------
//                         Drawing


void WmfPainterBackend::setPixel(WmfDeviceContext &context, int x, int y, QColor color)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << x << y << color;
#endif

    updateFromDeviceContext(context);

    QPen oldPen = mPainter->pen();
    QPen pen = oldPen;
    pen.setColor(color);
    mPainter->setPen(pen);
    mPainter->drawLine(x, y, x, y);
    mPainter->setPen(oldPen);
}


void WmfPainterBackend::lineTo(WmfDeviceContext &context, int x, int y)
{
    updateFromDeviceContext(context);

#if DEBUG_WMFPAINT
    kDebug(31000) << x << ", " << y << " using " << mPainter->pen();
#endif

    QPoint newPoint(x, y);
    mPainter->drawLine(context.currentPosition, newPoint);
    context.currentPosition = newPoint;
}


void WmfPainterBackend::drawRect(WmfDeviceContext &context, int x, int y, int w, int h)
{
    updateFromDeviceContext(context);

#if DEBUG_WMFPAINT
    kDebug(31000) << x << ", " << y << ", " << w << ", " << h;
    kDebug(31000) << "Using QPainter: " << mPainter->pen() << mPainter->brush();
#endif

    mPainter->drawRect(x, y, w, h);
}


void WmfPainterBackend::drawRoundRect(WmfDeviceContext &context, int x, int y, int w, int h,
                                      int roudw, int roudh)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << x << ", " << y << ", " << w << ", " << h;
#endif

    updateFromDeviceContext(context);
    mPainter->drawRoundRect(x, y, w, h, roudw, roudh);
}


void WmfPainterBackend::drawEllipse(WmfDeviceContext &context, int x, int y, int w, int h)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << x << ", " << y << ", " << w << ", " << h;
#endif
    updateFromDeviceContext(context);
    mPainter->drawEllipse(x, y, w, h);
}


void WmfPainterBackend::drawArc(WmfDeviceContext &context, int x, int y, int w, int h,
                                int a, int alen)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << x << ", " << y << ", " << w << ", " << h;
#endif
    updateFromDeviceContext(context);
    mPainter->drawArc(x, y, w, h, a, alen);
}


void WmfPainterBackend::drawPie(WmfDeviceContext &context, int x, int y, int w, int h,
                                int a, int alen)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << x << ", " << y << ", " << w << ", " << h;
#endif
    updateFromDeviceContext(context);
    mPainter->drawPie(x, y, w, h, a, alen);
}


void WmfPainterBackend::drawChord(WmfDeviceContext &context, int x, int y, int w, int h,
                                  int a, int alen)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << x << ", " << y << ", " << w << ", " << h
                  << ", " << a << ", " << alen;
#endif
    updateFromDeviceContext(context);
    mPainter->drawChord(x, y, w, h, a, alen);
}


void WmfPainterBackend::drawPolyline(WmfDeviceContext &context, const QPolygon &pa)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << pa;
#endif
    updateFromDeviceContext(context);
    mPainter->drawPolyline(pa);
}


void WmfPainterBackend::drawPolygon(WmfDeviceContext &context, const QPolygon &pa)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << pa;
    kDebug(31000) << "Using QPainter: " << mPainter->pen() << mPainter->brush();
#endif

    updateFromDeviceContext(context);
    if (context.polyFillMode)
        mPainter->drawPolygon(pa, Qt::WindingFill);
    else
        mPainter->drawPolygon(pa, Qt::OddEvenFill);
}


void WmfPainterBackend::drawPolyPolygon(WmfDeviceContext &context, QList<QPolygon>& listPa)
{
#if DEBUG_WMFPAINT
    kDebug(31000);
#endif

    updateFromDeviceContext(context);

    mPainter->save();
    QBrush brush = mPainter->brush();

    // define clipping region
    QRegion region;
    foreach(const QPolygon & pa, listPa) {
        region = region.xored(pa);
    }
    mPainter->setClipRegion(region);

    // fill polygons
    if (brush != Qt::NoBrush) {
        //kDebug(31000) << "Filling polygon with " << brush;
        mPainter->fillRect(region.boundingRect(), brush);
    }

    // draw polygon's border
    mPainter->setClipping(false);
    if (mPainter->pen().style() != Qt::NoPen) {
        mPainter->setBrush(Qt::NoBrush);
        foreach(const QPolygon & pa, listPa) {
#if DEBUG_WMFPAINT
            kDebug(31000) << pa;
#endif
            if (context.polyFillMode == WINDING)
                mPainter->drawPolygon(pa, Qt::WindingFill);
            else
                mPainter->drawPolygon(pa, Qt::OddEvenFill);
        }
    }

    // restore previous state
    mPainter->restore();
}


void WmfPainterBackend::drawImage(WmfDeviceContext &context, int x, int y, const QImage &img,
                                  int sx, int sy, int sw, int sh)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << x << " " << y << " " << sx << " " << sy << " " << sw << " " << sh;
#endif
    updateFromDeviceContext(context);
    mPainter->drawImage(x, y, img, sx, sy, sw, sh);
}


void WmfPainterBackend::patBlt(WmfDeviceContext &context, int x, int y, int width, int height,
                               quint32 rasterOperation)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << x << y << width << height << hex << rasterOperation << dec;
#endif

    updateFromDeviceContext(context);

    // 0x00f00021 is the PatCopy raster operation which just fills a rectangle with a brush.
    // This seems to be the most common one.
    //
    // FIXME: Implement the rest of the raster operations.
    if (rasterOperation == 0x00f00021) {
        // Would have been nice if we didn't have to pull out the
        // brush to use it with fillRect()...
        QBrush brush = mPainter->brush();
        mPainter->fillRect(x, y, width, height, brush);
    }
}


void WmfPainterBackend::drawText(WmfDeviceContext &context, int x, int y, const QString& text)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << x << y << hex << dec << text;
#endif

    updateFromDeviceContext(context);

    // The TA_UPDATECP flag tells us to use the current position
    if (context.textAlign & TA_UPDATECP) {
        // (left, top) position = current logical position
        x = context.currentPosition.x();
        y = context.currentPosition.y();
#if DEBUG_WMFPAINT
        kDebug(31000) << "Using current position:" << x << y;
#endif
    }

    QFontMetrics  fm(mPainter->font(), mTarget);
    int width  = fm.width(text) + fm.descent();    // fm.width(text) isn't right with Italic text
    int height = fm.height();

    // Horizontal align.  These flags are supposed to be mutually exclusive.
    if ((context.textAlign & TA_CENTER) == TA_CENTER)
        x -= (width / 2);
    else if ((context.textAlign & TA_RIGHT) == TA_RIGHT)
        x -= width;

    // Vertical align. 
    if ((context.textAlign & TA_BASELINE) == TA_BASELINE)
        y -= fm.ascent();  // (height - fm.descent()) is used in qwmf.  This should be the same.
    else if ((context.textAlign & TA_BOTTOM) == TA_BOTTOM) {
        y -= height;
    }

#if DEBUG_WMFPAINT
    kDebug(31000) << "font = " << mPainter->font() << " pointSize = " << mPainter->font().pointSize()
                  << "ascent = " << fm.ascent() << " height = " << fm.height()
                  << "leading = " << fm.leading();
#endif

    // Use the special pen defined by the foregroundTextColor in the device context for text.
    mPainter->save();
    mPainter->setPen(context.foregroundTextColor);

    // If the actual height is < 0, we should use device units.  This
    // means that if the text is currently upside-down due to some
    // transformations, we should un-upside-down it before painting.
    //kDebug(31000) << "fontheight:" << context.height << "height:" << height << "y" << y;
    if (context.height < 0 && mPainter->worldTransform().m22() < 0) {
        mPainter->translate(0, -(y - height / 2));
        mPainter->scale(qreal(1.0), qreal(-1.0));
        mPainter->translate(0, +(y - height / 2));

        // This is necessary to get drawText(x, y, ...) right below.
        y = -3 * y;
    }

    mPainter->translate(x, y);
    if (context.escapement != 0) {
        mPainter->rotate(qreal(context.escapement) / qreal(-10.0));
    }
    mPainter->drawText(0, 0, width, height, Qt::AlignLeft|Qt::AlignTop, text);

    mPainter->restore();
}


// ----------------------------------------------------------------
//                         Private functions


// If anything has changed in the device context that is relevant to
// the QPainter, then update the painter with the corresponding data.
//
void WmfPainterBackend::updateFromDeviceContext(WmfDeviceContext &context)
{
    // Graphic objects
    if (context.changedItems & DCBrush) {
        mPainter->setBrush(context.brush);
#if DEBUG_WMFPAINT
        kDebug(31000) << "*** Setting fill brush to" << context.brush;
#endif
    }
    // FIXME: context.image
    if (context.changedItems & DCFont) {
        mPainter->setFont(context.font);
#if DEBUG_WMFPAINT
        kDebug(31000) << "*** Setting font to" << context.font;
#endif
    }
    if (context.changedItems & DCPalette) {
        // NYI
#if DEBUG_WMFPAINT
        kDebug(31000) << "*** Setting palette (NYI)";
#endif
    }
    if (context.changedItems & DCPen) {
        QPen p = context.pen;
        int width = p.width();

        if (dynamic_cast<QPrinter *>(mTarget)) {
            width = 0;
        }
        else  if (width == 1)
            // I'm unsure of this, but it seems that WMF uses line
            // width == 1 as cosmetic pen.  Or it could just be that
            // any line width < 1 should be drawn as width == 1.  The
            // WMF spec doesn't mention the term "cosmetic pen"
            // anywhere so we don't get any clue there.
            //
            // For an example where this is shown clearly, see
            // wmf_tests.doc, in the colored rectangles and the polypolygon.
            width = 0;
#if 0
        else {
            // WMF spec: width of pen in logical coordinate
            // => width of pen proportional with device context width
            QRect rec = mPainter->window();
            // QPainter documentation says this is equivalent of xFormDev, but it doesn't compile. Bug reported.

            QRect devRec = rec * mPainter->matrix();
            if (rec.width() != 0)
                width = (width * devRec.width()) / rec.width() ;
            else
                width = 0;
        }
#endif

        p.setWidth(width);
        mPainter->setPen(p);
#if DEBUG_WMFPAINT
        kDebug(31000) << "*** Setting pen to" << p;
#endif
    }
    if (context.changedItems & DCClipRegion) {
        // Not used until SETCLIPREGION is used
#if DEBUG_WMFPAINT
        //kDebug(31000) << "*** region changed to" << context.region;
#endif
    }

    // Structure objects
    if (context.changedItems & DCBgTextColor) {
        mPainter->setBackground(QBrush(context.backgroundColor));
#if DEBUG_WMFPAINT
        kDebug(31000) << "*** Setting background text color to" << context.backgroundColor;
#endif
    }
    //----------------------------------------------------------------
    // Output surface not supported
    //DCViewportExt
    //DCViewportorg
    //DCWindowExt  
    //DCWindoworg  

    //----------------------------------------------------------------
    // Graphic Properties

    if (context.changedItems & DCBgMixMode) {
        // FIXME: Check the default value for this.
        mPainter->setBackgroundMode(context.bgMixMode == TRANSPARENT ? Qt::TransparentMode
                                                                     : Qt::OpaqueMode);
#if DEBUG_WMFPAINT
        kDebug(31000) << "*** Setting background mode to" << context.bgMixMode;
#endif
    }
    //Break extra space NYI
    //Font mapping mode NYI
    if (context.changedItems & DCFgMixMode) {
        // FIXME: Check the default value for this.
        QPainter::CompositionMode  compMode = QPainter::CompositionMode_Source;
        if (context.rop < 17)
            compMode = koWmfOpTab16[context.rop];
        mPainter->setCompositionMode(compMode);

#if DEBUG_WMFPAINT
        kDebug(31000) << "*** Setting composition mode to" << context.rop;
#endif
    }
    //layoutMode not necessary to handle here
    //Mapping mode NYI
    //PolyFillMode not necessary to handle here
    //Stretchblt mode NYI
    //textAlign not necessary to handle here
    //Text extra space NYI

    // Reset all changes until next time.
    context.changedItems = 0;
}


}
