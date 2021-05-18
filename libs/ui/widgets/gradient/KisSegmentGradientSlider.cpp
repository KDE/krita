/*
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *                2004 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSegmentGradientSlider.h"
#include <QPainter>
#include <QContextMenuEvent>
#include <QPixmap>
#include <QMouseEvent>
#include <QPolygon>
#include <QPaintEvent>
#include <QMenu>

#include <kis_debug.h>
#include <klocalizedstring.h>
#include <QAction>

#include <resources/KoSegmentGradient.h>

#define MARGIN 5
#define HANDLE_SIZE 10
#define MIN_HEIGHT 60

KisSegmentGradientSlider::KisSegmentGradientSlider(QWidget *parent, const char* name, Qt::WindowFlags f)
        : QWidget(parent, f),
        m_currentSegment(0),
        m_selectedSegment(0),
        m_drag(0)
{
    setObjectName(name);
    setMinimumHeight(MIN_HEIGHT);

    m_segmentMenu = new QMenu();
    m_segmentMenu->addAction(i18n("Split Segment"), this, SLOT(slotSplitSegment()));
    m_segmentMenu->addAction(i18n("Duplicate Segment"), this, SLOT(slotDuplicateSegment()));
    m_segmentMenu->addAction(i18n("Mirror Segment"), this, SLOT(slotMirrorSegment()));

    m_removeSegmentAction  = new QAction(i18n("Remove Segment"), this);
    connect(m_removeSegmentAction, SIGNAL(triggered()), this, SLOT(slotRemoveSegment()));

    m_segmentMenu->addAction(m_removeSegmentAction);
}

void KisSegmentGradientSlider::setGradientResource(KoSegmentGradientSP agr)
{
    m_autogradientResource = agr;
    m_selectedSegment = m_autogradientResource->segmentAt(0.0);
    emit sigSelectedSegment(m_selectedSegment);
}



void KisSegmentGradientSlider::paintSegmentHandle(int position, const QString text, const QPoint& textPos, QPainter& painter)
{
    QPolygon triangle(3);
    triangle[0] = QPoint(position, height() - HANDLE_SIZE - MARGIN);
    triangle[1] = QPoint(position + (HANDLE_SIZE / 2 - 1), height() - MARGIN);
    triangle[2] = QPoint(position - (HANDLE_SIZE / 2 - 1), height() - MARGIN);
    painter.drawPolygon(triangle);
    painter.drawText(textPos, text);
}

void KisSegmentGradientSlider::paintEvent(QPaintEvent* pe)
{
    QWidget::paintEvent(pe);
    QPainter painter(this);
    painter.fillRect(rect(), palette().window());
    painter.setPen(Qt::black);
    painter.drawRect(MARGIN, MARGIN, width() - 2 * MARGIN, height() - 2 * MARGIN - HANDLE_SIZE);
    if (m_autogradientResource) {
        QImage image = m_autogradientResource->generatePreview(width() - 2 * MARGIN - 2, height() - 2 * MARGIN - HANDLE_SIZE - 2);
        QPixmap pixmap(image.width(), image.height());
        if (!image.isNull()) {
            painter.drawImage(MARGIN + 1, MARGIN + 1, image);
        }

        painter.fillRect(MARGIN + 1, height() - MARGIN - HANDLE_SIZE, width() - 2 * MARGIN, HANDLE_SIZE, QBrush(Qt::white));
        if (m_selectedSegment) {
            QRect selection(qRound(m_selectedSegment->startOffset()*(double)(width() - 2 * MARGIN - 2)) + 6,
                            height() - HANDLE_SIZE - MARGIN,
                            qRound((m_selectedSegment->endOffset() - m_selectedSegment->startOffset())*(double)(width() - 12)),
                            HANDLE_SIZE);
            painter.fillRect(selection, QBrush(palette().highlight()));
        }

        QList<KoGradientSegment*> segments = m_autogradientResource->segments();
        for (int i = 0; i < segments.count(); i++) {
            KoGradientSegment* segment = segments[i];

            //paint segment start
            int position = qRound(segment->startOffset() * (double)(width() - 12)) + 6;
            QPoint textPos(position, height() - 2 * (HANDLE_SIZE + MARGIN));
            QString text = segment->startType() == FOREGROUND_ENDPOINT ? "FG" : (segment->startType() == BACKGROUND_ENDPOINT ? "BG" : "");
            paintSegmentHandle(position, text, textPos, painter);

            //paint segment end
            position = qRound(segment->endOffset() * (double)(width() - 12)) + 6;
            textPos.setX(position - HANDLE_SIZE);
            text = segment->endType() == FOREGROUND_ENDPOINT ? "FG" : (segment->endType() == BACKGROUND_ENDPOINT ? "BG" : "");
            paintSegmentHandle(position, text, textPos, painter);

            //paint midpoint
            position = qRound(segment->middleOffset() * (double)(width() - 12)) + 6;
            painter.setBrush(QBrush(Qt::white));
            paintSegmentHandle(position, "", textPos, painter);
        }
    }
}

void KisSegmentGradientSlider::mousePressEvent(QMouseEvent * e)
{
    if ((e->y() < MARGIN || e->y() > height() - MARGIN) || (e->x() < MARGIN || e->x() > width() - MARGIN) || e-> button() != Qt::LeftButton) {
        QWidget::mousePressEvent(e);
        return;
    }

    double t = (double)(e->x() - MARGIN) / (double)(width() - 2 * MARGIN);
    KoGradientSegment* segment = 0;
    segment = m_autogradientResource->segmentAt(t);
    if (segment != 0) {
        m_currentSegment = segment;
        QRect leftHandle(qRound(m_currentSegment->startOffset() *(double)(width() - 2*MARGIN - 2) + MARGIN - (HANDLE_SIZE / 2 - 1)),
                         height() - HANDLE_SIZE,
                         HANDLE_SIZE - 1,
                         HANDLE_SIZE);
        QRect middleHandle(qRound(m_currentSegment->middleOffset() *(double)(width() - 2*MARGIN - 2) + MARGIN - (HANDLE_SIZE / 2 - 2)),
                           height() - HANDLE_SIZE - MARGIN,
                           HANDLE_SIZE - 1,
                           HANDLE_SIZE);
        QRect rightHandle(qRound(m_currentSegment->endOffset() *(double)(width() - 2*MARGIN - 2) + MARGIN - (HANDLE_SIZE / 2 - 1)),
                          height() - HANDLE_SIZE,
                          HANDLE_SIZE - 1,
                          HANDLE_SIZE);
        // Change the activation order of the handles to avoid deadlocks
        if (t > 0.5) {
            if (leftHandle.contains(e->pos()))
                m_drag = LEFT_DRAG;
            else if (middleHandle.contains(e->pos()))
                m_drag = MIDDLE_DRAG;
            else if (rightHandle.contains(e->pos()))
                m_drag = RIGHT_DRAG;
        } else {
            if (rightHandle.contains(e->pos()))
                m_drag = RIGHT_DRAG;
            else if (middleHandle.contains(e->pos()))
                m_drag = MIDDLE_DRAG;
            else if (leftHandle.contains(e->pos()))
                m_drag = LEFT_DRAG;
        }

        if (m_drag == NO_DRAG) {
            m_selectedSegment = m_currentSegment;
            emit sigSelectedSegment(m_selectedSegment);
        }
    }
    repaint();
}

void KisSegmentGradientSlider::mouseReleaseEvent(QMouseEvent * e)
{
    Q_UNUSED(e);
    m_drag = NO_DRAG;
}

void KisSegmentGradientSlider::mouseMoveEvent(QMouseEvent * e)
{
    if ((e->y() < MARGIN || e->y() > height() - MARGIN) || (e->x() < MARGIN || e->x() > width() - MARGIN)) {
        QWidget::mouseMoveEvent(e);
        return;
    }

    double t = (double)(e->x() - MARGIN) / (double)(width() - 2 * MARGIN);
    switch (m_drag) {
    case RIGHT_DRAG:
        m_autogradientResource->moveSegmentEndOffset(m_currentSegment, t);
        break;
    case LEFT_DRAG:
        m_autogradientResource->moveSegmentStartOffset(m_currentSegment, t);
        break;
    case MIDDLE_DRAG:
        m_autogradientResource->moveSegmentMiddleOffset(m_currentSegment, t);
        break;
    }

    if (m_drag != NO_DRAG)
        emit sigChangedSegment(m_currentSegment);

    repaint();
}

void KisSegmentGradientSlider::contextMenuEvent(QContextMenuEvent * e)
{
    m_removeSegmentAction->setEnabled(m_autogradientResource->removeSegmentPossible());
    m_segmentMenu->popup(e->globalPos());
}

void KisSegmentGradientSlider::slotSplitSegment()
{
    m_autogradientResource->splitSegment(m_selectedSegment);
    emit sigSelectedSegment(m_selectedSegment);
    repaint();
}

void KisSegmentGradientSlider::slotDuplicateSegment()
{
    m_autogradientResource->duplicateSegment(m_selectedSegment);
    emit sigSelectedSegment(m_selectedSegment);
    repaint();
}

void KisSegmentGradientSlider::slotMirrorSegment()
{
    m_autogradientResource->mirrorSegment(m_selectedSegment);
    emit sigSelectedSegment(m_selectedSegment);
    repaint();
}

void KisSegmentGradientSlider::slotRemoveSegment()
{
    m_selectedSegment = m_autogradientResource->removeSegment(m_selectedSegment);
    emit sigSelectedSegment(m_selectedSegment);
    repaint();
}

