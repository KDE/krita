/*
 *  Copyright (c) 2010 Carlos Licea <carlos@kdab.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KOTABLESTYLE_H
#define KOTABLESTYLE_H

#include "KoStyle.h"

class KoTableStyle : public KoStyle
{
public:
    KoTableStyle();
    ~TableStyle();

    void setBackgroundColor(QColor color);
    QColor backgroundColor() const;

    void setBreakBefore(bool breakBefore);
    bool breakBefore() const;

    void setBreakAfter(bool breakAfter);
    bool breakAfter() const;

    void setAllowBreakBetweenRows(bool allow);
    bool allowBreakBetweenRows() const;

    void setMargins(qreal left, qreal top, qreal right, qreal bottom);
    void setLeftMargin(qreal left);
    void setTopMargin(qreal top);
    void setRightMargin(qreal right);
    void setBottomMargin(qreal bottom);

    enum WidthUnit {
        Percentage,
        Points
    };
    void setWidth(qreal width, WidthUnit unit = Points);
    qreal width() const;
    WidthUnit widthUnit() const;

    enum HorizontalAlign {
        Center,
        Left,
        Margins,
        Right
    };
    void setHorizontalAlign(HorizontalAlign align);
    HorizontalAlign horizontalAlign() const;

    enum BorderModel {
        Collapsing,
        Separating
    };
    void setBorderModel(BorderModel bordelModel);
    BorderModel borderModel() const;

    void setDisplay(bool display);
    bool display() const;

//     Image backgroundImage() const;
//     void setBackgroundImage(Image image);

protected:
    virtual void prepareStyle(KoGenStyle& style) const;

private:
    QColor m_backgroundColor;
    bool m_breakAfter;
    bool m_breakBefore;
    bool m_allowBreakBetweenRows;

    qreal m_leftMargin;
    qreal m_topMargin;
    qreal m_rightMargin;
    qreal m_bottomMargin;

    qreal width;
    WidthUnit unit;

    HorizontalAlign m_horizontalAlign;
    BorderModel m_borderModel;

    bool m_display;
    //TODO style:page-number
    //TODO style:shadow
    //TODO style:writing-mode
    //
};

#endif
