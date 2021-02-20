/*
 * SPDX-FileCopyrightText: 2008 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KIS_SELECTION_DECORATION_H_
#define _KIS_SELECTION_DECORATION_H_

#include <QPainterPath>
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
