/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_SHAPE_LAYER_CANVAS_H
#define KIS_SHAPE_LAYER_CANVAS_H

#include <KoCanvasBase.h>

#include <krita_export.h>

#include <kis_types.h>

class KoShapeManager;
class KoToolProxy;
class KoViewConverter;
class QUndoCommand;
class QWidget;
class KoUnit;

/**
 * KisShapeLayerCanvas is a special canvas implementation that Krita
 * uses for non-krita shapes to request updates on.
 *
 * Do NOT give this canvas to tools or to the KoCanvasController, it's
 * not made for that.
 */
class KRITAUI_EXPORT KisShapeLayerCanvas : public QObject, public KoCanvasBase
{
    Q_OBJECT
public:

    KisShapeLayerCanvas(KisShapeLayer *parent, KoViewConverter * viewConverter);
    virtual ~KisShapeLayerCanvas();

    /// This canvas won't render onto a widget, but a projection
    void setProjection( KisPaintDeviceSP projection )
        {
            m_projection = projection;
        }

    void gridSize(double *horizontal, double *vertical) const;
    bool snapToGrid() const;
    void addCommand(QUndoCommand *command);
    KoShapeManager *shapeManager() const;
    void updateCanvas(const QRectF& rc);
    KoToolProxy * toolProxy();
    KoViewConverter *viewConverter();
    QWidget* canvasWidget();
    KoUnit unit();

private slots:
    void repaint();

private:

    KoViewConverter * m_viewConverter;
    KoShapeManager * m_shapeManager;
    KisPaintDeviceSP m_projection;
    KisShapeLayer *m_parentLayer;

    QRegion m_dirty;
    bool m_repaintTriggered;
};

#endif
