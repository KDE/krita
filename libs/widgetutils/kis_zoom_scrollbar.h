/*
 *  Copyright (c) 2020 Eoin O'Neill <eoinoneill1991@gmail.com>
 *  Copyright (c) 2020 Emmet O'Neill <emmetoneill.pdx@gmail.com>
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


#ifndef KIS_ZOOM_SCROLLBAR_H
#define KIS_ZOOM_SCROLLBAR_H

#include <QScrollBar>
#include <kritawidgetutils_export.h>

class KRITAWIDGETUTILS_EXPORT KisZoomableScrollbar : public QScrollBar
{
    Q_OBJECT

private:
    QPoint previousPosition;
    bool catchTeleportCorrection = false;
    qreal scrollAccumulator;
    qreal zoomAccumulator;

public:
    KisZoomableScrollbar(QWidget* parent = 0);
    KisZoomableScrollbar(Qt::Orientation orientation, QWidget * parent = 0);
    ~KisZoomableScrollbar();

    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

Q_SIGNALS:
    void zoom(qreal delta);
    void overscroll(int delta);
};

#endif // KIS_ZOOM_SCROLLBAR_H
