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
#ifndef _WMFPAINTERBACKEND_H_
#define _WMFPAINTERPACKEND_H_

#include "kovectorimage_export.h"
#include "WmfAbstractBackend.h"

#include <QPainter>
#include <QTransform>

class QPolygon;


/**
   Namespace for Windows Metafile (WMF) classes
*/
namespace Libwmf
{

class WmfDeviceContext;

/**
 * WmfPainterBackend inherits the abstract class WmfAbstractbackend
 * and redirects WMF actions onto a QPaintDevice.
 * Uses relative or absolute coordinate.
 *
 * how to use:
 * <pre>
 *   QPixmap  pix(100, 100);
 *   QPainter painter(pix);
 *   WmfPainterBackend wmf(painter, pix.size());
 *   if (wmf.load("/home/test.wmf" )) {
 *      wmf.play(pix);
 *   }
 *   paint.drawPixmap(0, 0, pix);
 * </pre>
 *
 */

class KOVECTORIMAGE_EXPORT WmfPainterBackend : public WmfAbstractBackend
{
public:
    WmfPainterBackend(QPainter *painter, const QSizeF &outputSize);
    ~WmfPainterBackend();

    using WmfAbstractBackend::play;

    /**
     * Play a WMF file on a QPaintDevice. Return true on success.
     */
    //bool play(QPaintDevice& target);
    //bool play(QPainter &painter);
    bool play();


private:
    // -------------------------------------------------------------------------
    // A virtual QPainter
    bool  begin(const QRect &boundingBox);
    bool  end();
    void  save();
    void  restore();

    /// Recalculate the world transform and then apply it to the painter
    /// This must be called at the end of every function that changes the transform.
    void recalculateWorldTransform();

    // Drawing attributes/modes
    void  setCompositionMode(QPainter::CompositionMode mode);

    /**
     * Change logical Coordinate
     * some wmf files call those functions several times in the middle of a drawing
     * others wmf files doesn't call setWindow* at all
     * negative width and height are possible
     */
    void  setWindowOrg(int left, int top);
    void  setWindowExt(int width, int height);
    void  setViewportOrg(int left, int top);
    void  setViewportExt(int width, int height);

    // Graphics drawing functions
    void  setPixel(WmfDeviceContext &context, int x, int y, QColor color);
    void  lineTo(WmfDeviceContext &context, int x, int y);
    void  drawRect(WmfDeviceContext &context, int x, int y, int w, int h);
    void  drawRoundRect(WmfDeviceContext &context, int x, int y, int w, int h, int = 25, int = 25);
    void  drawEllipse(WmfDeviceContext &context, int x, int y, int w, int h);
    void  drawArc(WmfDeviceContext &context, int x, int y, int w, int h, int a, int alen);
    void  drawPie(WmfDeviceContext &context, int x, int y, int w, int h, int a, int alen);
    void  drawChord(WmfDeviceContext &context, int x, int y, int w, int h, int a, int alen);
    void  drawPolyline(WmfDeviceContext &context, const QPolygon& pa);
    void  drawPolygon(WmfDeviceContext &context, const QPolygon& pa);
    /**
     * drawPolyPolygon draw the XOR of a list of polygons
     * listPa : list of polygons
     */
    void  drawPolyPolygon(WmfDeviceContext &context, QList<QPolygon>& listPa);
    void  drawImage(WmfDeviceContext &context, int x, int y, const QImage &,
                    int sx = 0, int sy = 0, int sw = -1, int sh = -1);
    void  patBlt(WmfDeviceContext &context, int x, int y, int width, int height,
                 quint32 rasterOperation);

    // Text drawing functions
    // rotation = the degrees of rotation in counterclockwise
    // not yet implemented in KWinMetaFile
    void  drawText(WmfDeviceContext &context, int x, int y, const QString &s);

    // matrix transformation : only used in some bitmap manipulation
    void  setMatrix(WmfDeviceContext &context, const QMatrix &, bool combine = false);

 private:
    void updateFromDeviceContext(WmfDeviceContext &context);

protected:
    bool  mIsInternalPainter;      // True if the painter wasn't externally provided.
    QPainter *mPainter;
    QSizeF    mOutputSize;
    QPaintDevice *mTarget;
    bool  mRelativeCoord;
    //QPoint mLastPos;

    // Everything that has to do with window and viewport calculation
    QPoint        mWindowOrg;
    QSize         mWindowExt;
    QPoint        mViewportOrg;
    QSize         mViewportExt;
    bool          mWindowExtIsSet;
    bool          mViewportExtIsSet;
    QTransform    mOutputTransform;
    QTransform    mWorldTransform;
    
    int mSaveCount; //number of times Save() was called without Restore()
};


}

#endif
