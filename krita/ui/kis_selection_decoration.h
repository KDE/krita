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

#include <kis_signal_compressor.h>
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
    void setVisible(bool v);

protected:
    void drawDecoration(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter,KisCanvas2* canvas);

private slots:
    void slotStartUpdateSelection();

public slots:
    void selectionChanged();
    void antsAttackEvent();
private:
    bool selectionIsActive();

private:
    KisSignalCompressor m_signalCompressor;
    QPainterPath m_outlinePath;
    QTimer* m_antsTimer;
    int m_offset;

    QList<QBrush> m_brushes;
    Mode m_mode;
};

#endif
