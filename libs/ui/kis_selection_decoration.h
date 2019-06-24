/*
 * Copyright (c) 2008 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_SELECTION_DECORATION_H_
#define _KIS_SELECTION_DECORATION_H_

#include <QTimer>
#include <QPolygon>
#include <QPen>

#include <kis_signal_compressor.h>
#include "canvas/kis_canvas_decoration.h"

class KisView;

class KRITAUI_EXPORT KisSelectionDecoration : public KisCanvasDecoration
{
    Q_OBJECT
public:
    KisSelectionDecoration(QPointer<KisView> view);
    ~KisSelectionDecoration() override;

    enum Mode {
        Ants,
        Mask
    };

    Mode mode() const;
    void setMode(Mode mode);
    void setVisible(bool v) override;

protected:
    void drawDecoration(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter,KisCanvas2* canvas) override;

private Q_SLOTS:
    void slotStartUpdateSelection();
    void slotConfigChanged();

public Q_SLOTS:
    void selectionChanged();
    void antsAttackEvent();
private:
    bool selectionIsActive();

private:
    KisSignalCompressor m_signalCompressor;
    QPainterPath m_outlinePath;
    QImage m_thumbnailImage;
    QTransform m_thumbnailImageTransform;
    QTimer* m_antsTimer;
    int m_offset;

    QPen m_antsPen;
    QPen m_outlinePen;
    Mode m_mode;

    QColor m_maskColor;
    bool m_antialiasSelectionOutline;
};

#endif
