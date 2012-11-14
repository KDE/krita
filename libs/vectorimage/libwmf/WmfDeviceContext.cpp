/* This file is part of the Calligra project

  Copyright 2011 Inge Wallin <inge@lysator.liu.se>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either 
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public 
  License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "WmfDeviceContext.h"
#include "WmfEnums.h"

#include <QtGlobal>

#include <kdebug.h>


/**
   Namespace for Windows Metafile (WMF) classes
*/
namespace Libwmf
{


WmfDeviceContext::WmfDeviceContext()
{
    reset();
}


void WmfDeviceContext::reset()
{
    // Graphics Objects
    brush = QBrush(Qt::NoBrush);
    image = QImage();
    font = QFont();
    escapement = 0;
    orientation = 0;
    height = 0;
    //Palette
    pen = QPen(Qt::black);
    clipRegion = QRegion();

    // Structure Objects
    backgroundColor = QColor(Qt::white);
    currentPosition = QPoint(0, 0);
    foregroundTextColor = QColor(Qt::black);
    //Output Surface**  (what is this good for?  Mixing colors?)
    viewportExt = QSize();
    viewportOrg = QPoint();
    windowExt = QSize();
    windowOrg = QPoint();

    // Graphic Properties
    bgMixMode = 0;// FIXME: Check the real default
    //Break extra space
    //Font mapping mode
    rop = 0;// FIXME: Check the real default
    layoutMode = 0;// FIXME: Check the real default
    //Mapping mode
    polyFillMode = Libwmf::ALTERNATE;
    //Stretchblt mode
    textAlign = 0;// FIXME: Check the real default
    //Text extra space

    changedItems = 0xffffffff;  // Everything changed the first time.

    // Derivative values.
    m_windowExtIsSet = false;
    m_viewportExtIsSet = false;
    m_worldTransform.reset();
}


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


void WmfDeviceContext::recalculateWorldTransform()
{
    m_worldTransform = QTransform();

    if (!m_windowExtIsSet && !m_viewportExtIsSet)
        return;

    // FIXME: Check windowExt == 0 in any direction
    qreal windowViewportScaleX;
    qreal windowViewportScaleY;
    if (m_windowExtIsSet && m_viewportExtIsSet) {
        // Both window and viewport are set.
        windowViewportScaleX = qreal(viewportExt.width()) / qreal(windowExt.width());
        windowViewportScaleY = qreal(viewportExt.height()) / qreal(windowExt.height());
#if 0
        kDebug(31000) << "Scale for Window -> Viewport"
                      << windowViewportScaleX << windowViewportScaleY;
#endif
    }
    else {
        // At most one of window and viewport ext is set: Use same width for window and viewport
        windowViewportScaleX = qreal(1.0);
        windowViewportScaleY = qreal(1.0);
#if 0
        kDebug(31000) << "Only one of Window or Viewport set: scale window -> viewport = 1";
#endif
    }

    // Negative window extensions mean flip the picture.  Handle this here.
    bool  flip = false;
    qreal midpointX = 0.0;
    qreal midpointY = 0.0;
    qreal scaleX = 1.0;
    qreal scaleY = 1.0;
    if (windowExt.width() < 0) {
        midpointX = (windowOrg.x() + windowExt.width()) / qreal(2.0);
        scaleX = -1.0;
        flip = true;
    }
    if (windowExt.height() < 0) {
        midpointY = (windowOrg.y() + windowExt.height()) / qreal(2.0);
        scaleY = -1.0;
        flip = true;
    }
    if (flip) {
        kDebug(31000) << "Flipping round midpoint" << midpointX << midpointY << scaleX << scaleY;
        m_worldTransform.translate(midpointX, midpointY);
        m_worldTransform.scale(scaleX, scaleY);
        m_worldTransform.translate(-midpointX, -midpointY);
        //kDebug(31000) << "After flipping for window" << m_worldTransform;
    }

    // Calculate the world transform.
    m_worldTransform.translate(-windowOrg.x(), -windowOrg.y());
    m_worldTransform.scale(windowViewportScaleX, windowViewportScaleY);
    if (m_viewportExtIsSet) {
        m_worldTransform.translate(viewportOrg.x(), viewportOrg.y());
    } 
    else {
        // If viewport is not set, but window is *and* the window
        // width/height is negative, then we must compensate for this.
        // If the width/height is positive, we already did it with the
        // first translate before the scale() above.
        if (windowExt.width() < 0) 
            m_worldTransform.translate(windowOrg.x() + windowExt.width(), qreal(0.0));
        if (windowExt.height() < 0) 
            m_worldTransform.translate(qreal(0.0), windowOrg.y() + windowExt.height());
    }
    //kDebug(31000) << "After window viewport calculation" << m_worldTransform;
}


}
