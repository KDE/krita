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
class KisPattern;
class KoAbstractGradient;
class KisFilterConfiguration;
class KisPainter;
class QPainter;
class QPainterPath;

enum PaintMode { XOR_MODE, BW_MODE };

/// Definitions of the toolgroups of Krita
static const QString TOOL_TYPE_SHAPE = "0 Krita/Shape";         // Geometric shapes like ellipses and lines
static const QString TOOL_TYPE_FREEHAND = "1 Krita/Freehand";   // Freehand drawing tools
static const QString TOOL_TYPE_TRANSFORM = "2 Krita/Transform"; // Tools that transform the layer;
static const QString TOOL_TYPE_FILL = "3 Krita/Fill";                // Tools that fill parts of the canvas
static const QString TOOL_TYPE_VIEW = "4 Krita/View";                // Tools that affect the canvas: pan, zoom, etc.
static const QString TOOL_TYPE_SELECTED = "5 Krita/Select";          // Tools that select pixels

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

    virtual void resourceChanged(int key, const QVariant & res);

public:

    /// reimplemented from superclass
    virtual void mousePressEvent(KoPointerEvent *event);

    /// reimplemented from superclass
    virtual void mouseMoveEvent(KoPointerEvent *event);

    /// reimplemented from superclass
    virtual void mouseReleaseEvent(KoPointerEvent *event);

    /// reimplemented from superclass
    virtual void mouseDoubleClickEvent(KoPointerEvent *) {}  // when a krita tool is enabled, don't push double click on

    /// Convert from native (postscript points) to image pixel
    /// coordinates.
    QPointF convertToPixelCoord(KoPointerEvent *e);
    QPointF convertToPixelCoord(const QPointF& pt);

    /// Convert from native (postscript points) to integer image pixel
    /// coordinates. This truncates the floating point components and
    /// should be used in preference to QPointF::toPoint(), which rounds,
    /// to ensure the cursor acts on the pixel it is visually over.
    QPoint convertToIntPixelCoord(KoPointerEvent *e);

    QRectF convertToPt(const QRectF &rect);

    QPointF viewToPixel(const QPointF &viewCoord);
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

    virtual QWidget* createOptionWidget();
    virtual QWidget* optionWidget();

    inline void setOutlineStyle(PaintMode mode) {
        m_outlinePaintMode = mode;
    }

protected:

    KisImageWSP image() const;
    QCursor cursor() const;

    /// @return the currently active selection
    KisSelectionSP currentSelection() const;

    /// Call this to set the document modified
    void notifyModified() const;

    KisImageWSP currentImage();
    KisPattern* currentPattern();
    KoAbstractGradient * currentGradient();
    KisNodeSP currentNode();
    KoColor currentFgColor();
    KoColor currentBgColor();
    KisPaintOpPresetSP currentPaintOpPreset();
    KisFilterConfiguration * currentGenerator();

    /// convenience method to fill the painter's settings with all the current resources
    virtual void setupPainter(KisPainter * painter);

    /// paint the path which is in view coordinates, default paint mode is XOR_MODE, BW_MODE is also possible
    void paintToolOutline(QPainter * painter, QPainterPath &path);

    /// Returns true if the canvas this tool is associated with supports OpenGL rendering.
    bool isCanvasOpenGL() const;

    /// Call before starting to use native OpenGL commands when painting this tool's decorations.
    /// This is a convenience method that calls beginOpenGL() on the OpenGL canvas object.
    void beginOpenGL();

    /// Call after finishing use of native OpenGL commands when painting this tool's decorations.
    /// This is a convenience method that calls endOpenGL() on the OpenGL canvas object.
    void endOpenGL();

protected slots:
    /**
     * Called whenever the configuration settings change.
     */
    virtual void resetCursorStyle();

private slots:

    void slotToggleFgBg();
    void slotResetFgBg();

private:
    PaintMode m_outlinePaintMode;

    struct Private;
    Private* const d;
};



#endif // KIS_TOOL_H_

