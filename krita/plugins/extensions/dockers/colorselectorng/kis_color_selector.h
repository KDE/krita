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

#ifndef KIS_COLSELNG_COLOR_SELECTOR_H
#define KIS_COLSELNG_COLOR_SELECTOR_H

#include "kis_color_selector_base.h"

#include <QColor>
class KisColorSelectorTriangle;
class KisColorSelectorRing;
class KisColorSelectorComponent;
class KisColorSelectorSimple;

class KisColorSelector : public KisColorSelectorBase
{
public:
    enum Type {Ring, Square, Wheel, Triangle, Slider};
    enum Parameters {H, S, V, L, SL, SV, SH, VH, LH};
    struct Configuration {
        Type mainType;
        Type subType;
        Parameters mainTypeParameter;
        Parameters subTypeParameter;
        Configuration(Type mainT = Square,
                              Type subT = Slider,
                              Parameters mainTP = SV,
                              Parameters subTP = H)
                                  : mainType(mainT),
                                  subType(subT),
                                  mainTypeParameter(mainTP),
                                  subTypeParameter(subTP)
        {}
    };

//    enum MainType {Ring, Square, Wheel};
//    enum SubType {Triangle, Square, Slider};
//    enum MainTypeParameter {SL, SV, SH, VH, LH, VSV/*experimental*/};
//    enum SubTypeParameter {H, S, V, L};

    KisColorSelector(QWidget* parent = 0, Configuration conf = Configuration());
    KisColorSelectorBase* createPopup() const;

    void setConfiguration(Configuration conf);
    Configuration configuration() const;
protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent *);
private:
    void mouseEvent(QMouseEvent* e);

    KisColorSelectorRing* m_ring;
    KisColorSelectorComponent* m_triangle;
    KisColorSelectorSimple* m_slider;
    KisColorSelectorSimple* m_square;

    Configuration m_configuration;

    QColor m_lastColor;
};

#endif // KIS_COLSELNG_COLOR_SELECTOR_H
