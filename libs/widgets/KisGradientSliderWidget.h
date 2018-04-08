/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *                2004 Sven Langkamp <sven.langkamp@gmail.com>
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

#ifndef _KIS_GRADIENT_SLIDER_WIDGET_H_
#define _KIS_GRADIENT_SLIDER_WIDGET_H_

#include <QWidget>
#include <QMouseEvent>
#include <QPaintEvent>

class QAction;
class QMenu;
class KoGradientSegment;
class KoSegmentGradient;

#include "kritawidgets_export.h"

/**
 * @brief The KisGradientSliderWidget class makes it possible to edit gradients.
 */
class KRITAWIDGETS_EXPORT KisGradientSliderWidget : public QWidget
{
    Q_OBJECT

public:
    KisGradientSliderWidget(QWidget *parent = 0, const char* name = 0, Qt::WindowFlags f = 0);

public:
    void paintEvent(QPaintEvent *) override;
    void setGradientResource(KoSegmentGradient* agr);
    KoGradientSegment* selectedSegment() {
        return m_selectedSegment;
    }

Q_SIGNALS:
    void sigSelectedSegment(KoGradientSegment*);
    void sigChangedSegment(KoGradientSegment*);

protected:
    void mousePressEvent(QMouseEvent * e) override;
    void mouseReleaseEvent(QMouseEvent * e) override;
    void mouseMoveEvent(QMouseEvent * e) override;
    void contextMenuEvent(QContextMenuEvent * e) override;

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

    KoSegmentGradient* m_autogradientResource;
    KoGradientSegment* m_currentSegment;
    KoGradientSegment* m_selectedSegment;
    QMenu* m_segmentMenu;
    int m_drag;
    QAction *m_removeSegmentAction;
};

#endif
