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

#ifndef KOROWSTYLE_H
#define KOROWSTYLE_H

#include "KoStyle.h"
#include "koodf_export.h"

#include <QColor>

/**
 * A \class KoRowStyle represents a style of a row to be applied to one or more rows.
 * 
 * As all the styles it can be shared
 */

class KOODF_EXPORT KoRowStyle : public KoStyle
{
    KoRowStyle();

public:
    KOSTYLE_DECLARE_SHARED_POINTER(KoRowStyle)
    ~KoRowStyle();

    void setBackgroundColor(const QColor& color);
    QColor backgroundColor() const;

    enum HeightType{
        MinimumHeight,
        ExactHeight,
        OptimalHeight
    };
    void setHeight(qreal height);
    void setHeightType(HeightType type);
    qreal height() const;

    enum BreakType {
        NoBreak,
        AutoBreak,
        ColumnBreak,
        PageBreak
    };
    void setBreakBefore(BreakType breakBefore);
    BreakType breakBefore() const;

    void setBreakAfter(BreakType breakAfter);
    BreakType breakAfter() const;

    enum KeepTogetherType {
        DontKeepTogether,
        AutoKeepTogether,
        AlwaysKeeptogether
    };
    void setKeepTogether(KeepTogetherType keepTogether);
    KeepTogetherType keepTogether() const;

//     void setBackgroundImage(Image image);
//     Image backgroundImage() const;

protected:
    virtual KoGenStyle::Type automaticstyleType() const;
    virtual QString defaultPrefix() const;
    virtual void prepareStyle(KoGenStyle& style) const;
    virtual const char* styleFamilyName() const;
    virtual KoGenStyle::Type styleType() const;

private:
    QColor m_backgroundColor;
//     Image* m_image;

    qreal m_height;
    HeightType m_heightType;
    BreakType m_breakAfter;
    BreakType m_breakBefore;
    KeepTogetherType m_keepTogether;
};

#endif
