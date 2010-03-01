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

#include <KoColorProfile.h>
#include <KoColor.h>

#include <kis_paintop_preset.h>
#include <kis_types.h>

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

    void setPaintColor(const KoColor& foregroundColor);
    void setPreset(KisPaintOpPresetSP preset);
    void setBackgroundColor(const QColor& backgroundColor);
    void setBackgroundTile(const QImage& tile);
    void setColorSpace(const KoColorSpace* colorSpace);
    void setDisplayProfile(const KoColorProfile* colorProfile);
    void clear();

protected:

    virtual void contextMenuEvent ( QContextMenuEvent * event );
    virtual void keyPressEvent ( QKeyEvent * event );
    virtual void keyReleaseEvent ( QKeyEvent * event );
    virtual void mouseDoubleClickEvent ( QMouseEvent * event );
    virtual void mouseMoveEvent ( QMouseEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseReleaseEvent ( QMouseEvent * event );
    virtual void paintEvent ( QPaintEvent * event );
    virtual void resizeEvent ( QResizeEvent * event );
    virtual void tabletEvent ( QTabletEvent * event );
    virtual void wheelEvent ( QWheelEvent * event );

private:

    enum Mode {
        PAINTING,
        HOVERING,
        PANNING
    };

    QPoint m_offset;
    const KoColorSpace* m_colorSpace;
    KoColor m_foregroundColor;
    QImage m_backgroundTile;
    QColor m_backgroundColor;
    Mode m_toolMode;
    KisPaintDeviceSP m_paintDevice;
    KisPaintOpPresetSP m_preset;
    BackgroundMode m_backgroundMode;
    const KoColorProfile* m_displayProfile;
};

#endif // KIS_SCRATCH_PAD_H
