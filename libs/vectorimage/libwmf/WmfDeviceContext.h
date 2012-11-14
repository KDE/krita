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


#ifndef _WMFDEVICECONTEXT_H_
#define _WMFDEVICECONTEXT_H_


#include <QColor>
#include <QRect>
#include <QPen>
#include <QFont>
#include <QPolygon>
#include <QRegion>
#include <QTransform>


/**
   Namespace for Windows Metafile (WMF) classes
*/
namespace Libwmf
{


/**
 * WmfDeviceContext contains the WMF Playback Device Context.
 *
 * See [MS-WMF].pdf section 3.1.5: Playback Device Context for details.
 */


enum DeviceContextMembers {
    // Graphic objects
    DCBrush          = 0x00000001,
    DCFont           = 0x00000002,
    DCPalette        = 0x00000004,
    DCPen            = 0x00000008,
    DCClipRegion     = 0x00000010,

    // Structure objects
    DCBgTextColor    = 0x00000020,
    DCCurrentPos     = 0x00000040,
    DCFgTextColor    = 0x00000080,
    // Output surface not supported
    DCViewportExt    = 0x00000100,
    DCViewportorg    = 0x00000200,
    DCWindowExt      = 0x00000400,
    DCWindoworg      = 0x00000800,

    // Graphic properties
    DCBgMixMode      = 0x00001000,
    DCBrExtraSpace   = 0x00002000,
    DCFontMapMode    = 0x00004000,
    DCFgMixMode      = 0x00008000,
    DCLayoutMode     = 0x00010000,
    DCMapMode        = 0x00020000,
    DCPolyFillMode   = 0x00040000,
    DCStretchBltMode = 0x00080000,
    DCTextAlignMode  = 0x00100000,
    DCTextExtraSpace = 0x00200000
};

/**
   WMF Playback Device Context

   See [MS-WMF].pdf section 3.1.5
*/
class WmfDeviceContext
{
public:
    WmfDeviceContext();
    void reset();

    // Graphic Objects
    QBrush    brush;            // Brush
    QImage    image;            //   - extra image
    QFont     font;             // Font
    int       escapement;       //   - rotation of the text in 1/10th of a degree
    int       orientation;      //   - rotation of characters in 1/10th of a degree
    int       height;           //   - original font height; can be negative
    //Palette                   // Palette not supported yet
    QPen      pen;              // Pen
    QRegion   clipRegion;       // Region

    // Structure Objects
    QColor  backgroundColor;    // Background text color
    QPoint  currentPosition;    // Drawing position (Current point)
    QColor  foregroundTextColor; // Foreground text color
    ;    //Output Surface**  (what is this good for?  Mixing colors?)
    QSize  viewportExt;         // Viewport extent
    QPoint viewportOrg;         // Viewport origin
    QSize  windowExt;           // Window extent
    QPoint windowOrg;           // Window origin

    // Graphic Properties
    quint16  bgMixMode;         // Background mix mode
    //Break extra space NYI
    //Font mapping mode NYI
    quint16  rop;               // Foreground mix mode
    quint16  layoutMode;        // Layout mode
    //Mapping mode NYI
    quint16  polyFillMode;      // Polygon fill mode
    //Stretchblt mode NYI
    quint16  textAlign;         // Text alignment mode
    //Text extra space NYI

    // ----------------------------------------------------------------
    //                         Helper data

    // This is not part of the actual device context, but indicates
    // changed items.  It is used by the backends to update their
    // internal state.
    quint32  changedItems;      // bitmap of DeviceContextMembers


    // Cached values

    // window and viewport calculation
    bool        m_windowExtIsSet;
    bool        m_viewportExtIsSet;
    QTransform  m_worldTransform;

private:
    void recalculateWorldTransform();
};


}

#endif
