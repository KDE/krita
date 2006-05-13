/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
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

#ifndef KIS_STRATEGY_MOVE_H_
#define KIS_STRATEGY_MOVE_H_

#include <QPoint>
#include <QRect>

#include <krita_export.h>

class KisCanvasController;
class KisCanvasSubject;

class KRITAUI_EXPORT KisStrategyMove {
public:
    KisStrategyMove();
    explicit KisStrategyMove(KisCanvasSubject *subject);
    virtual ~KisStrategyMove();

public:
    void reset(KisCanvasSubject *subject);
    void startDrag(const QPoint& pos);
    void drag(const QPoint& pos);
    void endDrag(const QPoint& pos, bool undo = true);
    void simpleMove(const QPoint& pt1, const QPoint& pt2);
    void simpleMove(qint32 x1, qint32 y1, qint32 x2, qint32 y2);

private:
    KisStrategyMove(const KisStrategyMove&);
    KisStrategyMove& operator=(const KisStrategyMove&);

private:
    KisCanvasController *m_controller;
    KisCanvasSubject *m_subject;
    QRect m_deviceBounds;
    QPoint m_dragStart;
    QPoint m_layerStart;
    QPoint m_layerPosition;
    bool m_dragging;
};

#endif // KIS_STRATEGY_MOVE_H_

