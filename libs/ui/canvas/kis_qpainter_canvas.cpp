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
#include <kis_config.h>

#include <KoColorProfile.h>
#include "kis_coordinates_converter.h"
#include <KoZoomHandler.h>
#include <KoToolManager.h>
#include <KoToolProxy.h>

#include <kis_image.h>
#include <kis_layer.h>

#include "KisViewManager.h"
#include "kis_canvas2.h"
#include "kis_prescaled_projection.h"
#include "kis_canvas_resource_provider.h"
#include "KisDocument.h"
#include "kis_selection_manager.h"
#include "kis_selection.h"
#include "kis_canvas_updates_compressor.h"
#include "kis_config_notifier.h"
#include "kis_group_layer.h"
#include "canvas/kis_display_color_converter.h"

//#define DEBUG_REPAINT
#include <KoCanvasController.h>

class KisQPainterCanvas::Private
{
public:
    KisPrescaledProjectionSP prescaledProjection;
    QBrush checkBrush;
    bool scrollCheckers;
};

KisQPainterCanvas::KisQPainterCanvas(KisCanvas2 *canvas, KisCoordinatesConverter *coordinatesConverter, QWidget * parent)
        : QWidget(parent)
        , KisCanvasWidgetBase(canvas, coordinatesConverter)
        , m_d(new Private())
{
    setAutoFillBackground(true);
    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled, false);
    setAttribute(Qt::WA_StaticContents);
    setAttribute(Qt::WA_OpaquePaintEvent);
#ifdef Q_OS_OSX
    setAttribute(Qt::WA_AcceptTouchEvents, false);
#else
    setAttribute(Qt::WA_AcceptTouchEvents, true);
#endif
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

void KisQPainterCanvas::paintEvent(QPaintEvent * ev)
{
    KisImageWSP image = canvas()->image();
    if (image == 0) return;

    setAutoFillBackground(false);

    QPainter gc(this);
    gc.setClipRegion(ev->region());

    KisCoordinatesConverter *converter = coordinatesConverter();

    gc.save();

    gc.setCompositionMode(QPainter::CompositionMode_Source);
    gc.fillRect(QRect(QPoint(0, 0), size()), borderColor());

    QTransform checkersTransform;
    QPointF brushOrigin;
    QPolygonF polygon;

    converter->getQPainterCheckersInfo(&checkersTransform, &brushOrigin, &polygon, m_d->scrollCheckers);
    gc.setPen(Qt::NoPen);
    gc.setBrush(m_d->checkBrush);
    gc.setBrushOrigin(brushOrigin);
    gc.setTransform(checkersTransform);
    gc.drawPolygon(polygon);

    drawImage(gc, ev->rect());

    gc.restore();

#ifdef DEBUG_REPAINT
    QColor color = QColor(random() % 255, random() % 255, random() % 255, 150);
    gc.fillRect(ev->rect(), color);
#endif

    drawDecorations(gc, ev->rect());
}

void KisQPainterCanvas::drawImage(QPainter & gc, const QRect &updateWidgetRect) const
{
    KisCoordinatesConverter *converter = coordinatesConverter();

    QTransform imageTransform = converter->viewportToWidgetTransform();
    gc.setTransform(imageTransform);
    gc.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QRectF viewportRect = converter->widgetToViewport(updateWidgetRect);

    gc.setCompositionMode(QPainter::CompositionMode_SourceOver);
    gc.drawImage(viewportRect, m_d->prescaledProjection->prescaledQImage(),
                 viewportRect);
}

QVariant KisQPainterCanvas::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return processInputMethodQuery(query);
}

void KisQPainterCanvas::inputMethodEvent(QInputMethodEvent *event)
{
    processInputMethodEvent(event);
}

void KisQPainterCanvas::channelSelectionChanged(const QBitArray &channelFlags)
{
    Q_ASSERT(m_d->prescaledProjection);
    m_d->prescaledProjection->setChannelFlags(channelFlags);
}

void KisQPainterCanvas::setDisplayColorConverter(KisDisplayColorConverter *colorConverter)
{
    Q_ASSERT(m_d->prescaledProjection);
    m_d->prescaledProjection->setMonitorProfile(colorConverter->monitorProfile(),
                                                colorConverter->renderingIntent(),
                                                colorConverter->conversionFlags());
}

void KisQPainterCanvas::setDisplayFilter(QSharedPointer<KisDisplayFilter> displayFilter)
{
        Q_ASSERT(m_d->prescaledProjection);
        m_d->prescaledProjection->setDisplayFilter(displayFilter);

        canvas()->startUpdateInPatches(canvas()->image()->bounds());
}

void KisQPainterCanvas::notifyImageColorSpaceChanged(const KoColorSpace *cs)
{
    Q_UNUSED(cs);
    // FIXME: on color space change the data is refetched multiple
    //        times by different actors!
    canvas()->startUpdateInPatches(canvas()->image()->bounds());
}

void KisQPainterCanvas::setWrapAroundViewingMode(bool value)
{
    Q_UNUSED(value);
    dbgKrita << "Wrap around viewing mode not implemented in QPainter Canvas.";
    return;
}

void KisQPainterCanvas::finishResizingImage(qint32 w, qint32 h)
{
    m_d->prescaledProjection->slotImageSizeChanged(w, h);
}

KisUpdateInfoSP KisQPainterCanvas::startUpdateCanvasProjection(const QRect & rc, const QBitArray &channelFlags)
{
    Q_UNUSED(channelFlags);

    return m_d->prescaledProjection->updateCache(rc);
}


QRect KisQPainterCanvas::updateCanvasProjection(KisUpdateInfoSP info)
{
    /**
    * It might happen that the canvas type is switched while the
    * update info is being stuck in the Qt's signals queue. Than a wrong
    * type of the info may come. So just check it here.
    */
    bool isPPUpdateInfo = dynamic_cast<KisPPUpdateInfo*>(info.data());
    if (isPPUpdateInfo) {
        m_d->prescaledProjection->recalculateCache(info);
        return info->dirtyViewportRect();
    } else {
        return QRect();
    }
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
}

void KisQPainterCanvas::slotConfigChanged()
{
    KisConfig cfg(true);

    m_d->checkBrush = QBrush(createCheckersImage());
    m_d->scrollCheckers = cfg.scrollCheckers();
    notifyConfigChanged();
}

bool KisQPainterCanvas::callFocusNextPrevChild(bool next)
{
    return focusNextPrevChild(next);
}
