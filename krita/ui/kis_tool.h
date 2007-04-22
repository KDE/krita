/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_TOOL_H_
#define KIS_TOOL_H_

#include <QCursor>

#include <KoColor.h>
#include <KoTool.h>
#include <KoID.h>
#include <KoCanvasResourceProvider.h>
#include <krita_export.h>
#include <kis_types.h>

class KoCanvasBase;
class KisBrush;
class KisPattern;
class KisGradient;
class KisPaintOpSettings;

/// Definitions of the toolgroups of Krita
static const QString TOOL_TYPE_SHAPE = "Krita/Shape"; // Geometric shapes like ellipses and lines
static const QString TOOL_TYPE_FREEHAND = "Krita/Freehand"; // Freehand drawing tools
static const QString TOOL_TYPE_TRANSFORM = "Krita/Transform"; // Tools that transform the layer;
static const QString TOOL_TYPE_FILL = "Krita/Fill"; // Tools that fill parts of the canvas
static const QString TOOL_TYPE_VIEW = "Krita/View"; // Tools that affect the canvas: pan, zoom, etc.
static const QString TOOL_TYPE_SELECTED = "Krita/Select"; // Tools that select pixels


class  KRITAUI_EXPORT KisTool
    : public KoTool
{
    Q_OBJECT

public:

    KisTool(KoCanvasBase * canvas, const QCursor & cursor);
    virtual ~KisTool();

// KoTool Implementation.

public slots:

    virtual void activate(bool temporary = false);
    virtual void deactivate();
    virtual void resourceChanged( int key, const QVariant & res );

public:

    virtual void mousePressEvent( KoPointerEvent * ) {};
    virtual void mouseDoubleClickEvent( KoPointerEvent * ) {};
    virtual void mouseMoveEvent( KoPointerEvent * ) {};
    virtual void mouseReleaseEvent( KoPointerEvent * ) {};
    virtual void wheelEvent( KoPointerEvent * ) {};

    /// Convert from native (postscript points) to image pixel
    /// coordinates.
    QPointF convertToPixelCoord( KoPointerEvent *e );

    /// Convert from native (postscript points) to integer image pixel
    /// coordinates. This truncates the floating point components and
    /// should be used in preference to QPointF::toPoint(), which rounds,
    /// to ensure the cursor acts on the pixel it is visually over.
    QPoint convertToIntPixelCoord( KoPointerEvent *e );

    QRectF convertToPt( const QRectF &rect );

    /// Convert an integer pixel coordinate into a view coordinate.
    /// The view coordinate is at the centre of the pixel.
    QPointF pixelToView(const QPoint &pixelCoord);

    /// Convert a floating point pixel coordinate into a view coordinate.
    QPointF pixelToView(const QPointF &pixelCoord);

    /// Convert a pixel rectangle into a view rectangle.
    QRectF pixelToView(const QRectF &pixelRect);

    /// Update the canvas for the given rectangle in image pixel coordinates.
    void updateCanvasPixelRect(const QRectF &pixelRect);

    /// Update the canvas for the given rectangle in view coordinates.
    void updateCanvasViewRect(const QRectF &viewRect);

protected:

    /// @return the image wrapped in the dummy shape in the shape
    /// manager. XXX: This is probably wrong!
    KisImageSP image() const;

    /// Call this to set the document modified
    void notifyModified() const;


protected:

    QCursor m_cursor; // the cursor that should be shown on tool activation.

    // From the canvas resources
    KisBrush * m_currentBrush;
    KisPattern * m_currentPattern;
    KisGradient * m_currentGradient;
    KoColor m_currentFgColor;
    KoColor m_currentBgColor;
    KoID m_currentPaintOp;
    KisPaintOpSettings * m_currentPaintOpSettings;
    KisLayerSP m_currentLayer;
    float m_currentExposure;
    KisImageSP m_currentImage;
};

#endif // KIS_TOOL_H_

