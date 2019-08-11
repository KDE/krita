/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
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

#ifndef KIS_COLOR_PATCHES_H
#define KIS_COLOR_PATCHES_H

#include "kis_color_selector_base.h"

#include "KoColor.h"

class KoColor;


class KisColorPatches : public KisColorSelectorBase
{
Q_OBJECT
public:
    explicit KisColorPatches(QString configPrefix, QWidget *parent = 0);
    enum Direction { Horizontal, Vertical };

public Q_SLOTS:
    void updateSettings() override;

protected:
    void setColors(QList<KoColor> colors);
    QList<KoColor> colors() const {return m_colors;}

    void paintEvent(QPaintEvent *) override;
    void wheelEvent(QWheelEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    int patchCount() const;
    bool colorAt(const QPoint &, KoColor *result) const;

public:
    /// set buttons, that should be drawn additionally to the patches
    /// this class takes ownership of them and will delete them
    /// they will be resized to the patchsize
    void setAdditionalButtons(QList<QWidget*> buttonList);

private:
    int m_patchWidth;
    int m_patchHeight;
    int m_patchCount;
    QList<KoColor> m_colors;
    bool m_allowColorListChangeGuard;
    int m_scrollValue;

    Direction m_direction;
    bool m_allowScrolling;
    int m_numCols;
    int m_numRows;
    QList<QWidget*> m_buttonList;

    /// returns width of the patchfield, if there are only m_numRows allowed
    int widthOfAllPatches();
    /// returns height of the patchfield, if there are only m_numCols allowed
    int heightOfAllPatches();

    /// returns height, that is needed to display all patches with the given width
    int heightForWidth(int width) const override;
    /// returns width, that is needed to display all patches with the given height
    int widthForHeight(int height) const;

    /// returns count of colors and buttons
    int fieldCount() const;

    QString m_configPrefix;

    QPoint m_dragStartPos;
};

#endif
