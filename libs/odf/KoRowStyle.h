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

#ifndef KOSTYLE_H
#define KOSTYLE_H

#include "KoStyle.h"

/**
 * A \class KoRowStyle represents a style of a row to be applied to one or more row.
 * 
 * As all the styles it can be shared
 */

class KoRowStyle : public KoStyle
{
public:
    KoRowStyle();
    ~KoRowStyle();

    void setBackgroundColor(QColor color);
    QColor backgroundColor() const;

    enum HeightType{
        MinimumHeight,
        ExactHeight,
        OptimalHeight
    };
    void setHeight(qreal height);
    void setHeightType(HeightType type);
    qreal height() const;

    void setBreakBefore(bool breakBefore);
    bool breakBefore() const;

    void setBreakAfter(bool breakAfter);
    bool breakAfter() const;

    void setKeepTogether(bool keepTogether);
    bool keepTogether() const;

    void setBackgroundImage(Image image);
    Image backgroundImage() const;

private:
    QColor m_backgroundColor;
    Image* m_image;

    qreal m_height;
    HeightType m_heightType;
    bool m_breakAfter;
    bool m_breakBhefore;
    bool m_keepTogether;
};

#endif
