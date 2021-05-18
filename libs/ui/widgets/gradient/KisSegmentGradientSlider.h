/*
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *                2004 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_SEGMENT_GRADIENT_SLIDER_H_
#define _KIS_SEGMENT_GRADIENT_SLIDER_H_

#include <QWidget>
#include <QMouseEvent>
#include <QPaintEvent>

#include <KoSegmentGradient.h>

class QAction;
class QMenu;

class KoGradientSegment;

#include "kritaui_export.h"

/**
 * @brief The KisSegmentGradientSlider class makes it possible to edit gradients.
 */
class KRITAUI_EXPORT KisSegmentGradientSlider : public QWidget
{
    Q_OBJECT

public:
    KisSegmentGradientSlider(QWidget *parent = 0, const char* name = 0, Qt::WindowFlags f = Qt::WindowFlags());

public:
    void paintEvent(QPaintEvent *) override;
    void setGradientResource(KoSegmentGradientSP agr);
    KoGradientSegment *selectedSegment() { return m_selectedSegment; }

Q_SIGNALS:
    void sigSelectedSegment(KoGradientSegment*);
    void sigChangedSegment(KoGradientSegment*);

protected:
    void mousePressEvent(QMouseEvent * e) override;
    void mouseReleaseEvent(QMouseEvent * e) override;
    void mouseMoveEvent(QMouseEvent * e) override;
    void contextMenuEvent(QContextMenuEvent * e) override;

    void paintSegmentHandle(int position, const QString text, const QPoint& textPos, QPainter& painter);

private Q_SLOTS:
    void slotSplitSegment();
    void slotDuplicateSegment();
    void slotMirrorSegment();
    void slotRemoveSegment();

private:

    enum {
        NO_DRAG,
        LEFT_DRAG,
        RIGHT_DRAG,
        MIDDLE_DRAG
    };

    enum {
        SPLIT_SEGMENT,
        DUPLICATE_SEGMENT,
        MIRROR_SEGMENT,
        REMOVE_SEGMENT
    };

    KoSegmentGradientSP m_autogradientResource;
    KoGradientSegment* m_currentSegment;
    KoGradientSegment* m_selectedSegment;
    QMenu* m_segmentMenu;
    int m_drag;
    QAction *m_removeSegmentAction;
};

#endif
