/* This file is part of the KDE project
 * Copyright 2010 (C) Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KIS_SCRATCH_PAD_H
#define KIS_SCRATCH_PAD_H

#include <QFrame>
#include <QImage>
#include <QColor>
#include <QCursor>
#include <QPoint>

#include <KoColorProfile.h>
#include <KoColor.h>
#include <KoCompositeOp.h>
#include <KoPointerEvent.h>

#include <kis_paintop_preset.h>
#include <kis_types.h>
#include <kis_paint_information.h>
#include <kis_painter.h>

#include <krita_export.h>
/**
 * A scratchpad is a painting canvas with only one zoomlevel and based on
 * a paint device, not on a KisImage. It can have a blank, tiled background or
 * a gradient background.
 */
class KRITAUI_EXPORT KisScratchPad : public QWidget
{

    Q_OBJECT

public:

    enum BackgroundMode {
        TILED,
        STRETCHED,
        CENTERED,
        GRADIENT,
        SOLID_COLOR
    };

    KisScratchPad(QWidget *parent = 0);
    virtual ~KisScratchPad();

public slots:

    void setPaintColor(const QColor& foregroundColor);
    void setPaintColor(const KoColor& foregroundColor);
    void setPreset(KisPaintOpPresetSP preset);
    void setBackgroundColor(const QColor& backgroundColor);
    void setBackgroundTile(const QImage& tile);
    void setColorSpace(const KoColorSpace* colorSpace);
    void setDisplayProfile(const KoColorProfile* colorProfile);
    void clear();
    
    void fillGradient(KoAbstractGradient* gradient);

    void fillSolid(const KoColor& color);
    
signals:

    void colorSelected(const KoColor& color);

protected:

    virtual void contextMenuEvent ( QContextMenuEvent * event );
    virtual void keyPressEvent ( QKeyEvent * event );
    virtual void keyReleaseEvent ( QKeyEvent * event );
    virtual void mouseDoubleClickEvent ( QMouseEvent * event );
    virtual void mouseMoveEvent ( QMouseEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseReleaseEvent ( QMouseEvent * event );
    virtual void tabletEvent ( QTabletEvent * event );
    virtual void wheelEvent ( QWheelEvent * event );
    virtual void paintEvent ( QPaintEvent * event );
    virtual void resizeEvent ( QResizeEvent * event );

private:

    void initPainting(QEvent* event);
    void paint(QEvent* event);
    void endPaint(QEvent* event);

    void pick(QMouseEvent* event);

    void initPan(QMouseEvent* event);
    void pan(QMouseEvent* event);
    void endPan(QMouseEvent* event);

    enum Mode {
        PAINTING,
        HOVERING,
        PANNING,
        PICKING
    };

    QPoint m_offset;
    const KoColorSpace* m_colorSpace;
    KoColor m_paintColor;
    QImage m_backgroundTile;
    QColor m_backgroundColor;
    Mode m_toolMode;
    KisPaintDeviceSP m_paintDevice;
    KisPaintOpPresetSP m_preset;
    BackgroundMode m_backgroundMode;
    const KoColorProfile* m_displayProfile;
    QCursor m_cursor;
    QPoint m_currentMousePosition;
    KisPaintInformation m_previousPaintInformation;
    KisPainter *m_painter;
    double m_dragDist;
    const KoCompositeOp *m_compositeOp;
    QPoint m_lastPosition;
};

#endif // KIS_SCRATCH_PAD_H
