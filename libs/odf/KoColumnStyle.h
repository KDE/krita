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

#ifndef KOCOLUMNSTYLE_H
#define KOCOLUMNSTYLE_H

#include "KoStyle.h"
#include "koodf_export.h"

/**
 * A \class KoColumnStyle represents a style to be applied to one or more columns.
 * 
 * As all the styles it can be shared
 */

class KOODF_EXPORT KoColumnStyle : public KoStyle
{
    KoColumnStyle();
public:
    KOSTYLE_DECLARE_SHARED_POINTER(KoColumnStyle)

    ~KoColumnStyle();

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

    enum WidthType {
        MinimumWidth,
        ExactWidth,
        OptimalWidth
    };
    void setWidth(qreal width);
    qreal width() const;
    void setWidthType(WidthType type);
    WidthType widthType() const;

protected:
    virtual void prepareStyle(KoGenStyle& style) const;
    virtual QString defaultPrefix() const;
    virtual KoGenStyle::Type styleType() const;
    virtual KoGenStyle::Type automaticstyleType() const;
    virtual const char* styleFamilyName() const;

private:
    BreakType m_breakAfter;
    BreakType m_breakBefore;
    qreal m_width;
    WidthType m_widthType;
};

#endif
