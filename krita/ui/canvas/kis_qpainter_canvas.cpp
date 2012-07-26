/*
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
 * Copyright (C) Lukas Tvrdy <lukast.dev@gmail.com>, (C) 2009
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

#include "kis_qpainter_canvas.h"


#include <QPaintEvent>
#include <QPoint>
#include <QRect>
#include <QPainter>
#include <QImage>
#include <QBrush>
#include <QColor>
#include <QString>
#include <QTime>
#include <QTimer>
#include <QApplication>
#include <QMenu>

#include <kis_debug.h>

#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoShapeManager.h>
#include "kis_coordinates_converter.h"
#include <KoZoomHandler.h>
#include <KoToolManager.h>
#include <KoToolProxy.h>

#include <kis_image.h>
#include <kis_layer.h>

#include "kis_view2.h"
#include "kis_canvas2.h"
#include "kis_prescaled_projection.h"
#include "kis_config.h"
#include "kis_canvas_resource_provider.h"
#include "kis_doc2.h"
#include "kis_selection_manager.h"
#include "kis_selection.h"
#include "kis_perspective_grid_manager.h"
#include "kis_config_notifier.h"
#include "kis_group_layer.h"

//#define DEBUG_REPAINT
#include <KoCanvasController.h>

class KisQPainterCanvas::Private
{
public:
    Private()
        : smooth(true)
    {}

    KisPrescaledProjectionSP prescaledProjection;
    QBrush checkBrush;
    bool smooth;
};

KisQPainterCanvas::KisQPainterCanvas(KisCanvas2 *canvas, KisCoordinatesConverter *coordinatesConverter, QWidget * parent)
        : QWidget(parent)
        , KisCanvasWidgetBase(canvas, coordinatesConverter)
        , m_d(new Private())
{
    setAutoFillBackground(true);
    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled, true);
    setAttribute(Qt::WA_StaticContents);
    setAttribute(Qt::WA_OpaquePaintEvent);

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));
    slotConfigChanged();
}

KisQPainterCanvas::~KisQPainterCanvas()
{
    delete m_d;
}

void KisQPainterCanvas::setPrescaledProjection(KisPrescaledProjectionSP prescaledProjection)
{
    m_d->prescaledProjection = prescaledProjection;
}

void KisQPainterCanvas::setSmoothingEnabled(bool smooth)
{
    m_d->smooth = smooth;
}

void KisQPainterCanvas::paintEvent(QPaintEvent * ev)
{
    KisImageWSP image = canvas()->image();
    if (image == 0) return;

    setAutoFillBackground(false);

    if (m_buffer.size() != size()) {
        m_buffer = QImage(size(), QImage::Format_ARGB32_Premultiplied);
    }


    QPainter gc(&m_buffer);

    // we double buffer, so we paint on an image first, then from the image onto the canvas,
    // so copy the clip region since otherwise we're filling the whole buffer every time with
    // the background color _and_ the transparent squares.
    gc.setClipRegion(ev->region());

    KisCoordinatesConverter *converter = coordinatesConverter();
    QTransform imageTransform = converter->viewportToWidgetTransform();

    gc.save();

    gc.setCompositionMode(QPainter::CompositionMode_Source);
    gc.fillRect(QRect(QPoint(0, 0), size()), borderColor());

    QTransform checkersTransform;
    QPointF brushOrigin;
    QPolygonF polygon;

    converter->getQPainterCheckersInfo(&checkersTransform, &brushOrigin, &polygon);
    gc.setPen(Qt::NoPen);
    gc.setBrush(m_d->checkBrush);
    gc.setBrushOrigin(brushOrigin);
    gc.setTransform(checkersTransform);
    gc.drawPolygon(polygon);

    gc.setTransform(imageTransform);
    if (m_d->smooth) {
        gc.setRenderHint(QPainter::SmoothPixmapTransform, true);
    }

    QRectF viewportRect = converter->widgetToViewport(ev->rect());

    gc.setCompositionMode(QPainter::CompositionMode_SourceOver);
    gc.drawImage(viewportRect, m_d->prescaledProjection->prescaledQImage(),
                 viewportRect);

    gc.restore();


#ifdef DEBUG_REPAINT
    QColor color = QColor(random() % 255, random() % 255, random() % 255, 150);
    gc.fillRect(ev->rect(), color);
#endif

    QRect boundingRect = converter->imageRectInWidgetPixels().toAlignedRect();
    drawDecorations(gc, boundingRect);
    gc.end();

    QPainter painter(this);
    painter.drawImage(ev->rect(), m_buffer, ev->rect());
}

bool KisQPainterCanvas::event(QEvent *e)
{
    if(toolProxy()) {
        toolProxy()->processEvent(e);
    }
    return QWidget::event(e);
}

QVariant KisQPainterCanvas::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return processInputMethodQuery(query);
}

void KisQPainterCanvas::inputMethodEvent(QInputMethodEvent *event)
{
    processInputMethodEvent(event);
}

void KisQPainterCanvas::resizeEvent(QResizeEvent *e)
{
    QSize size(e->size());
    if (size.width() <= 0) {
        size.setWidth(1);
    }
    if (size.height() <= 0) {
        size.setHeight(1);
    }

    coordinatesConverter()->setCanvasWidgetSize(size);
    m_d->prescaledProjection->notifyCanvasSizeChanged(size);
    emit needAdjustOrigin();
}

void KisQPainterCanvas::slotConfigChanged()
{
    m_d->checkBrush = QBrush(checkImage());
    notifyConfigChanged();
}

bool KisQPainterCanvas::callFocusNextPrevChild(bool next)
{
    return focusNextPrevChild(next);
}

#include "kis_qpainter_canvas.moc"
