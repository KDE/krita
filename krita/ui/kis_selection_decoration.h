/*
 * Copyright (c) 2008 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_SELECTION_DECORATION_H_
#define _KIS_SELECTION_DECORATION_H_

#include <QTimer>
#include <QPolygon>
#include <QBrush>

#include "canvas/kis_canvas_decoration.h"

class KRITAUI_EXPORT KisSelectionDecoration : public KisCanvasDecoration
{
    Q_OBJECT
public:
    KisSelectionDecoration(KisView2* view);
    ~KisSelectionDecoration();

    enum Mode {
        Ants,
        Mask
    };

    void setMode(Mode mode);
protected:
    virtual void drawDecoration(QPainter& gc, const QPoint & documentOffset, const QRect& area, const KoViewConverter &converter);
public slots:
    void selectionChanged();
    void selectionTimerEvent();
    void resourceChanged(int key, const QVariant & res);
private:
    bool selectionIsActive();
    void updateSimpleOutline();
    void updateMaskVisualisation(const QRect & r);
private:
    QVector<QPolygon> outline;
    QVector<QPolygon> simpleOutline;
    QTimer* timer;
    int offset;
    QList<QBrush> brushes;
    QImage m_image;
    Mode m_mode;
};

#endif
