/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
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

#ifndef KIS_COLSELNG_COMMON_COLORS_H
#define KIS_COLSELNG_COMMON_COLORS_H

#include "kis_color_patches.h"

class KisCanvas2;

class KisCommonColors : public KisColorPatches
{
Q_OBJECT
public:
    explicit KisCommonColors(QWidget *parent = 0);
//    int heightForWidth(int) const;
//    QSize sizeHint() const;
    void setCanvas(KisCanvas2 *canvas);
private:
    QList<QColor> extractColors();
    QList<QRgb> getColors();

    int m_numColors;
    QList<QColor> m_extractedColors;
    KisCanvas2* m_canvas;
public slots:
    void recalculate();
};

#endif // KIS_COLSELNG_COMMON_COLORS_H
