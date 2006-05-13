/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#include <QPixmap>
#include <QImage>
#include <QLayout>
#include <QPainter>
#include <q3frame.h>
#include <QLabel>
#include <QToolButton>
#include <QSlider>
#include <QCursor>
#include <QPaintEvent>
#include <QEvent>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QMouseEvent>

#include <kdebug.h>
#include <kglobalsettings.h>
#include <kaction.h>
#include <ktoolbar.h>
#include <knuminput.h>
#include <klocale.h>

#include <KoDocument.h>

#include "kobirdeyepanel.h"
#include "kis_int_spinbox.h"

KoCanvasAdapter::KoCanvasAdapter() {}
KoCanvasAdapter::~KoCanvasAdapter() {}

KoZoomAdapter::KoZoomAdapter() {}
KoZoomAdapter::~KoZoomAdapter() {}

KoThumbnailAdapter::KoThumbnailAdapter() {}
KoThumbnailAdapter::~KoThumbnailAdapter() {}

KoBirdEyePanel::KoBirdEyePanel( KoZoomAdapter * zoomListener, 
                                KoThumbnailAdapter * thumbnailProvider,
                                KoCanvasAdapter * canvas,
                                QWidget * parent,
                                const char * name,
                                Qt::WFlags f)
    : QWidget(parent, f)
    , m_zoomListener(zoomListener)
    , m_thumbnailProvider(thumbnailProvider)
    , m_canvas(canvas)
    , m_dragging(false)
{
    setObjectName(name);

    QHBoxLayout * l = new QHBoxLayout(this);
    m_page = new WdgBirdEye(this, "birdeye_panel");
    m_page->zoom->setRange((int) (qMax(1, (int)(100 * zoomListener->getMinZoom()))), (int) (100 * zoomListener->getMaxZoom()));
    m_page->zoom->setValue(100);
    m_page->zoom->setSuffix("%");

    m_page->toolbar->setIconSize(QSize(16, 16));
    m_page->view->installEventFilter(this);
    m_page->view->setAutoFillBackground(false);
    m_page->view->setAttribute(Qt::WA_OpaquePaintEvent);

    m_zoomIn = new KAction(KIcon("birdeye_zoom_plus"), i18n("Zoom In"), 0, "zoomIn");
    connect(m_zoomIn, SIGNAL(triggered()), this, SLOT(zoomPlus()));

    m_zoomOut = new KAction(KIcon("birdeye_zoom_minus"), i18n("Zoom Out"), 0, "zoomOut");
    connect(m_zoomOut, SIGNAL(triggered()), this, SLOT(zoomMinus()));

    l->addWidget(m_page);

    connect(m_page->zoom, SIGNAL(valueChanged(int)), SLOT(zoomValueChanged(int)));
    connect(m_page->bn100, SIGNAL(clicked()), SLOT(zoom100()));
    connect(m_page->slZoom, SIGNAL(valueChanged(int)), SLOT(sliderChanged( int )));
}

KoBirdEyePanel::~KoBirdEyePanel()
{
    delete m_canvas;
    delete m_thumbnailProvider;
    delete m_zoomListener;
}

void KoBirdEyePanel::setZoom(int zoom)
{
    m_page->zoom->blockSignals(true);
    m_page->slZoom->blockSignals(true);
    
    m_page->zoom->setValue(zoom);

    if (zoom < 10) {
        m_page->slZoom->setValue(0);
    }
    else if (zoom > 10 && zoom < 100) {
        m_page->slZoom->setValue(zoom / 10);
    }
    else if (zoom >= 100 && zoom < 150) {
        m_page->slZoom->setValue(10);
    }
    else if (zoom >= 150 && zoom < 250) {
        m_page->slZoom->setValue(11);
    }
    else if (zoom >= 250 && zoom < 350) {
        m_page->slZoom->setValue(12);
    }
    else if (zoom >= 350 && zoom < 450) {
        m_page->slZoom->setValue(13);
    }
    else if (zoom >= 450 && zoom < 550) {
        m_page->slZoom->setValue(14);
    }
    else if (zoom >= 550 && zoom < 650) {
        m_page->slZoom->setValue(15);
    }
    else if (zoom >= 650 && zoom < 875) {
        m_page->slZoom->setValue(16);
    }
    else if (zoom >= 875 && zoom < 1150) {
        m_page->slZoom->setValue(17);
    }
    else if (zoom >= 1150 && zoom < 1450) {
        m_page->slZoom->setValue(18);
    }
    else if (zoom >= 1450) {
        m_page->slZoom->setValue(19);
    }
    
    
    m_page->zoom->blockSignals(false);
    m_page->slZoom->blockSignals(false);


}

void KoBirdEyePanel::zoomValueChanged(int zoom)
{
    KoPoint center;
    center = m_canvas->visibleArea().center();
    m_zoomListener->zoomTo(center.x(), center.y(), zoom / 100.0);
    setZoom(zoom);
}

void KoBirdEyePanel::zoom100()
{
    zoomValueChanged( 100 );
}

void KoBirdEyePanel::sliderChanged( int v )
{
    if (v < 10) {
        zoomValueChanged((v + 1) * 10);
    }
    else {
        switch(v) {
            case 10:
                zoomValueChanged(100);
                break;
            case 11:
                zoomValueChanged(200);
                break;
            case 12:
                zoomValueChanged(300);
            case 13:
                zoomValueChanged(400);
                break;
            case 14:
                zoomValueChanged(500);
                break;
            case 15:
                zoomValueChanged(600);
                break;
            case 16:
                zoomValueChanged(750);
                break;
            case 17:
                zoomValueChanged(1000);
                break;
            case 18:
                zoomValueChanged(1300);
                break;
            case 19:
                zoomValueChanged(1600);
                break;
        }
    }
}

void KoBirdEyePanel::cursorPosChanged(qint32 xpos, qint32 ypos)
{
    m_page->txtX->setText(QString("%L1").arg(xpos, 5));
    m_page->txtY->setText(QString("%L1").arg(ypos, 5));
}

void KoBirdEyePanel::setThumbnailProvider(KoThumbnailAdapter * thumbnailProvider)
{
    delete m_thumbnailProvider;
    m_thumbnailProvider = thumbnailProvider;
}

void KoBirdEyePanel::slotViewTransformationChanged()
{
    updateVisibleArea();
    renderView();
    m_page->view->update();
    setZoom(qRound(m_canvas->zoomFactor() * 100));
}

void KoBirdEyePanel::slotUpdate(const QRect & r)
{
    QRect updateRect = r;

    if (m_thumbnailProvider->pixelSize() != m_documentSize) {
        m_documentSize = m_thumbnailProvider->pixelSize();
        fitThumbnailToView();
        updateRect = QRect(0, 0, m_documentSize.width(), m_documentSize.height());
    }

    updateRect &= QRect(0, 0, m_documentSize.width(), m_documentSize.height());

    if (!updateRect.isEmpty() && !m_documentSize.isEmpty()) {

        QRect thumbnailRect = documentToThumbnail(KoRect::fromQRect(updateRect));

        if (!thumbnailRect.isEmpty()) {

            QImage thumbnailImage = m_thumbnailProvider->image(thumbnailRect, m_thumbnail.size());

            if (!thumbnailImage.isNull()) {

                Q_ASSERT(thumbnailImage.size() == thumbnailRect.size());

                QPainter painter(&m_thumbnail);

                painter.fillRect(thumbnailRect, palette().mid());
                painter.drawImage(thumbnailRect.x(), thumbnailRect.y(), thumbnailImage);
            }
        }
    }

    renderView();
    m_page->view->update();
}

QRect KoBirdEyePanel::documentToThumbnail(const KoRect& docRect)
{
    if (docRect.isEmpty() || m_documentSize.isEmpty() || m_thumbnail.isNull()) {
        return QRect();
    }

    qint32 thumbnailLeft = static_cast<qint32>((docRect.left() * m_thumbnail.width()) / m_documentSize.width());
    qint32 thumbnailRight = static_cast<qint32>(((docRect.right() + 1) * m_thumbnail.width()) / m_documentSize.width());
    qint32 thumbnailTop = static_cast<qint32>((docRect.top() * m_thumbnail.height()) / m_documentSize.height());
    qint32 thumbnailBottom = static_cast<qint32>(((docRect.bottom() + 1) * m_thumbnail.height()) / m_documentSize.height());

    QRect thumbnailRect(thumbnailLeft, thumbnailTop, thumbnailRight - thumbnailLeft + 1, thumbnailBottom - thumbnailTop + 1);
    thumbnailRect &= m_thumbnail.rect();

    return thumbnailRect;
}

KoRect KoBirdEyePanel::thumbnailToDocument(const QRect& thumbnailRect)
{
    if (thumbnailRect.isEmpty() || m_documentSize.isEmpty() || m_thumbnail.isNull()) {
        return KoRect();
    }

    double docLeft = (static_cast<double>(thumbnailRect.left()) * m_documentSize.width()) / m_thumbnail.width();
    double docRight = (static_cast<double>(thumbnailRect.right() + 1) * m_documentSize.width()) / m_thumbnail.width();
    double docTop = (static_cast<double>(thumbnailRect.top()) * m_documentSize.height()) / m_thumbnail.height();
    double docBottom = (static_cast<double>(thumbnailRect.bottom() + 1) * m_documentSize.height()) / m_thumbnail.height();

    KoRect docRect(docLeft, docTop, docRight - docLeft + 1, docBottom - docTop + 1);
    docRect &= KoRect(0, 0, m_documentSize.width(), m_documentSize.height());

    return docRect;
}

QPoint KoBirdEyePanel::viewToThumbnail(const QPoint& viewPoint)
{
    int thumbnailX = (m_viewBuffer.width() - m_thumbnail.width()) / 2;
    int thumbnailY = (m_viewBuffer.height() - m_thumbnail.height()) / 2;

    return QPoint(viewPoint.x() - thumbnailX, viewPoint.y() - thumbnailY);
}


void KoBirdEyePanel::zoomMinus()
{
}

void KoBirdEyePanel::zoomPlus()
{
}

void KoBirdEyePanel::updateVisibleArea()
{
    m_visibleAreaInThumbnail = documentToThumbnail(m_canvas->visibleArea());
}


bool KoBirdEyePanel::eventFilter(QObject* o, QEvent* ev)
{
    if (o == m_page->view && ev->type() == QEvent::Resize) {
        resizeViewEvent(static_cast<QResizeEvent *>(ev)->size());
    }

    if (o == m_page->view && ev->type() == QEvent::Paint) {
        paintViewEvent(static_cast<QPaintEvent *>(ev));
    }

    if (o == m_page->view && ev->type() == QEvent::MouseMove) {

        QMouseEvent* me = (QMouseEvent*)ev;
        QPoint thumbnailPos = viewToThumbnail(me->pos());

        if (m_dragging) {
            handleMouseMoveAction(thumbnailPos);
        } else {
            handleMouseMove(thumbnailPos);
        }

        return true;
    }

    if (o == m_page->view && ev->type() == QEvent::MouseButtonPress) {

        QMouseEvent* me = (QMouseEvent*)ev;
        QPoint thumbnailPos = viewToThumbnail(me->pos());

        if (me->button() == Qt::LeftButton) {
            handleMousePress(thumbnailPos);
        }

        return true;
    }

    if (o == m_page->view && ev->type() == QEvent::MouseButtonRelease) {

        QMouseEvent* me = (QMouseEvent*)ev;

        if (me->button() == Qt::LeftButton) {
            m_dragging = false;
        }

        return true;
    }

    return m_page->eventFilter(o, ev);
}

KoBirdEyePanel::enumDragHandle KoBirdEyePanel::dragHandleAt(QPoint p)
{
    QRect left = QRect(m_visibleAreaInThumbnail.left()-1, m_visibleAreaInThumbnail.top()-1, 3, m_visibleAreaInThumbnail.height()+2);
    QRect right = QRect(m_visibleAreaInThumbnail.right()-1, m_visibleAreaInThumbnail.top()-1, 3, m_visibleAreaInThumbnail.height()+2);
    QRect top = QRect(m_visibleAreaInThumbnail.left()-1, m_visibleAreaInThumbnail.top()-1, m_visibleAreaInThumbnail.width()+2, 3);
    QRect bottom = QRect(m_visibleAreaInThumbnail.left()-1, m_visibleAreaInThumbnail.bottom()-1, m_visibleAreaInThumbnail.width()+2, 3);

    if (left.contains(p)) {
        return DragHandleLeft;
    }

    if (right.contains(p)) {
        return DragHandleRight;
    }

    if (top.contains(p)) {
        return DragHandleTop;
    }

    if (bottom.contains(p)) {
        return DragHandleBottom;
    }

    if (m_visibleAreaInThumbnail.contains(p)) {
        return DragHandleCentre;
    }

    return DragHandleNone;
}

void KoBirdEyePanel::handleMouseMove(QPoint p)
{
    QCursor cursor;

    switch (dragHandleAt(p)) {
    case DragHandleLeft:
    case DragHandleRight:
        cursor = Qt::SizeHorCursor;
        break;
    case DragHandleTop:
    case DragHandleBottom:
        cursor = Qt::SizeHorCursor;
        break;
    case DragHandleCentre:
        cursor = Qt::SizeAllCursor;
        break;
    default:
    case DragHandleNone:
        if (m_thumbnail.rect().contains(p)) {
            cursor = Qt::PointingHandCursor;
        } else {
            cursor = Qt::ArrowCursor;
        }
        break;
    }

    m_page->view->setCursor(cursor);
}

void KoBirdEyePanel::handleMouseMoveAction(QPoint p)
{
    if (m_dragging) {

        qint32 dx = p.x() - m_lastDragPos.x();
        qint32 dy = p.y() - m_lastDragPos.y();

        m_lastDragPos = p;

        QRect thumbnailRect = m_visibleAreaInThumbnail;

        switch (m_dragHandle) {
        case DragHandleLeft: {
            thumbnailRect.setLeft(thumbnailRect.left()+dx);
            break;
        }
        case DragHandleRight: {
            thumbnailRect.setRight(thumbnailRect.right()+dx);
            break;
        }
        case DragHandleTop: {
            thumbnailRect.setTop(thumbnailRect.top()+dy);
            break;
        }
        case DragHandleBottom: {
            thumbnailRect.setBottom(thumbnailRect.bottom()+dy);
            break;
        }
        case DragHandleCentre: {
            thumbnailRect.translate(dx, dy);
            break;
        }
        default:
        case DragHandleNone:
            break;
        }

        makeThumbnailRectVisible(thumbnailRect);
    }
}

void KoBirdEyePanel::handleMousePress(QPoint p)
{
    if (!m_dragging) {

        enumDragHandle dragHandle = dragHandleAt(p);

        if (dragHandle == DragHandleNone) {
            if (m_thumbnail.rect().contains(p)) {
    
                // Snap visible area centre to p and begin a centre drag.
    
                QRect thumbnailRect = m_visibleAreaInThumbnail;
                thumbnailRect.moveCenter(p);
                makeThumbnailRectVisible(thumbnailRect);
    
                m_dragHandle = DragHandleCentre;
                m_page->view->setCursor(Qt::SizeAllCursor);
                m_dragging = true;
            }
        } else {
            m_dragHandle = dragHandle;
            m_dragging = true;
        }
        m_lastDragPos = p;
    }
}

void KoBirdEyePanel::makeThumbnailRectVisible(const QRect& r)
{
    if (r.isEmpty()) {
        return;
    }

    QRect thumbnailRect = r;

    if (thumbnailRect.left() < m_thumbnail.rect().left()) {
        thumbnailRect.moveLeft(m_thumbnail.rect().left());
    }
    if (thumbnailRect.right() > m_thumbnail.rect().right()) {
        thumbnailRect.moveRight(m_thumbnail.rect().right());
    }
    if (thumbnailRect.top() < m_thumbnail.rect().top()) {
        thumbnailRect.moveTop(m_thumbnail.rect().top());
    }
    if (thumbnailRect.bottom() > m_thumbnail.rect().bottom()) {
        thumbnailRect.moveBottom(m_thumbnail.rect().bottom());
    }

    if (thumbnailRect.width() > m_thumbnail.rect().width()) {
        thumbnailRect.setLeft(m_thumbnail.rect().left());
        thumbnailRect.setRight(m_thumbnail.rect().right());
    }
    if (thumbnailRect.height() > m_thumbnail.rect().height()) {
        thumbnailRect.setTop(m_thumbnail.rect().top());
        thumbnailRect.setBottom(m_thumbnail.rect().bottom());
    }

    double zoomFactor = m_canvas->zoomFactor();

    if (thumbnailRect.size() == m_visibleAreaInThumbnail.size()) {
        // No change to zoom
    } else if (thumbnailRect.width() != m_visibleAreaInThumbnail.width()) {

        Q_ASSERT(thumbnailRect.height() == m_visibleAreaInThumbnail.height());

        zoomFactor *= static_cast<double>(m_visibleAreaInThumbnail.width()) / thumbnailRect.width();
    } else {

        Q_ASSERT(thumbnailRect.width() == m_visibleAreaInThumbnail.width());

        zoomFactor *= static_cast<double>(m_visibleAreaInThumbnail.height()) / thumbnailRect.height();
    }

    if (zoomFactor < m_zoomListener->getMinZoom()) {
        zoomFactor = m_zoomListener->getMinZoom();
    } else if (zoomFactor > m_zoomListener->getMaxZoom()) {
        zoomFactor = m_zoomListener->getMaxZoom();
    }

    KoRect docRect = thumbnailToDocument(thumbnailRect);
    m_zoomListener->zoomTo(docRect.center().x(), docRect.center().y(), zoomFactor);
}

void KoBirdEyePanel::resizeViewEvent(QSize size)
{
    m_viewBuffer = QPixmap(size);
    fitThumbnailToView();
    slotUpdate(QRect(0, 0, m_documentSize.width(), m_documentSize.height()));
}

void KoBirdEyePanel::fitThumbnailToView()
{
    QRect docRect = QRect(0, 0, m_thumbnailProvider->pixelSize().width(), m_thumbnailProvider->pixelSize().height());
    qint32 thumbnailWidth;
    qint32 thumbnailHeight;

    if (docRect.isEmpty()) {
        thumbnailWidth = 0;
        thumbnailHeight = 0;
    } else {
        const int thumbnailBorderPixels = 4;

        double xScale = double(m_page->view->contentsRect().width() - thumbnailBorderPixels) / docRect.width();
        double yScale = double(m_page->view->contentsRect().height() - thumbnailBorderPixels) / docRect.height();

        if (xScale < yScale) {
            thumbnailWidth = m_page->view->contentsRect().width() - thumbnailBorderPixels;
            thumbnailHeight = qint32(ceil(docRect.height() * xScale));
        } else {
            thumbnailWidth = qint32(ceil(docRect.width() * yScale));
            thumbnailHeight = m_page->view->contentsRect().height() - thumbnailBorderPixels;
        }
    }

    m_thumbnail = QPixmap(thumbnailWidth, thumbnailHeight);
    updateVisibleArea();
}

void KoBirdEyePanel::renderView()
{
    Q_ASSERT(!m_viewBuffer.isNull());

    if (!m_viewBuffer.isNull()) {

        updateVisibleArea();

        QPainter painter(&m_viewBuffer);

        painter.fillRect(0, 0, m_viewBuffer.width(), m_viewBuffer.height(), palette().mid());

        if (!m_thumbnail.isNull()) {

            int thumbnailX = (m_viewBuffer.width() - m_thumbnail.width()) / 2;
            int thumbnailY = (m_viewBuffer.height() - m_thumbnail.height()) / 2;

            painter.drawPixmap(thumbnailX, thumbnailY, m_thumbnail);

            painter.setPen(Qt::red);
            painter.drawRect(thumbnailX + m_visibleAreaInThumbnail.x() - 1, 
                             thumbnailY + m_visibleAreaInThumbnail.y() - 1, 
                             m_visibleAreaInThumbnail.width() + 2, 
                             m_visibleAreaInThumbnail.height() + 2);
            painter.setPen(QColor(Qt::red).light());
            painter.drawRect(thumbnailX + m_visibleAreaInThumbnail.x() - 2, 
                             thumbnailY + m_visibleAreaInThumbnail.y() - 2, 
                             m_visibleAreaInThumbnail.width() + 4, 
                             m_visibleAreaInThumbnail.height() + 4);
        }
    }
}

void KoBirdEyePanel::paintViewEvent(QPaintEvent *e)
{
    Q_ASSERT(!m_viewBuffer.isNull());

    if (!m_viewBuffer.isNull()) {
        QPainter p(m_page->view);

        p.drawPixmap(e->rect().x(), e->rect().y(), m_viewBuffer, 
               e->rect().x(), e->rect().y(), e->rect().width(), e->rect().height());
    }
}

#include "kobirdeyepanel.moc"
