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

#ifndef KOTBLSTYLE_H
#define KOTBLSTYLE_H

#include "KoStyle.h"
#include "koodf_export.h"

#include <QColor>

/**
 * A \class KoTblStyle represents a style for a Table in
 * a ODF document.
 *
 * As all the styles it can be shared.
 **/

class KOODF_EXPORT KoTblStyle : public KoStyle
{
    KoTblStyle();

public:
    KOSTYLE_DECLARE_SHARED_POINTER(KoTblStyle)

    ~KoTblStyle();

    void setBackgroundColor(const QColor& color);
    QColor backgroundColor() const;

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

    void setAllowBreakBetweenRows(bool allow);
    bool allowBreakBetweenRows() const;

    void setMasterPageName(const QString& name);
    void setMasterPageName(const char* name);
    QString masterPageName() const;

    void setLeftMargin(qreal left);
    qreal leftMargin() const;

    void setTopMargin(qreal top);
    qreal topMargin() const;

    void setRightMargin(qreal right);
    qreal rightMargin() const;

    void setBottomMargin(qreal bottom);
    qreal bottomMargin() const;

    enum WidthUnit {
        PercentageUnit,
        PointsUnit
    };
    void setWidth(qreal width, WidthUnit unit = PointsUnit);
    qreal width() const;
    WidthUnit widthUnit() const;

    enum HorizontalAlign {
        CenterAlign,
        LeftAlign,
        MarginsAlign,
        RightAlign
    };
    void setHorizontalAlign(HorizontalAlign align);
    HorizontalAlign horizontalAlign() const;

    enum BorderModel {
        CollapsingModel,
        SeparatingModel
    };
    void setBorderModel(BorderModel bordelModel);
    BorderModel borderModel() const;

    void setDisplay(bool display);
    bool display() const;

    enum KeepWithNext {
        AutoKeepWithNext,
        AlwaysKeepWithNext
    };
    void setKeepWithNext(KeepWithNext keepWithNext);
    KeepWithNext keepWithNext() const;

    enum WritingMode {
        LrTbWritingMode,
        RlTbWritingMode,
        TbRlWritingMode,
        TbLrWritingMode,
        LrWritingMode,
        RlWritingMode,
        TbWritingMode,
        PageWritingMode
    };
    void setWritingMode(WritingMode writingMode);
    WritingMode writingMode() const;

protected:
    virtual void prepareStyle(KoGenStyle& style) const;
    virtual KoGenStyle::Type automaticstyleType() const;
    virtual KoGenStyle::Type styleType() const;
    virtual const char* styleFamilyName() const;
    virtual QString defaultPrefix() const;

private:
    QColor m_backgroundColor;
    BreakType m_breakAfter;
    BreakType m_breakBefore;
    bool m_allowBreakBetweenRows;

    QString m_masterPageName;

    qreal m_leftMargin;
    qreal m_topMargin;
    qreal m_rightMargin;
    qreal m_bottomMargin;

    qreal m_width;
    WidthUnit m_widthUnit;

    HorizontalAlign m_horizontalAlign;
    BorderModel m_borderModel;
    KeepWithNext m_keepWithNext;
    WritingMode m_writingMode;

    bool m_display;
    //TODO style:page-number
    //TODO style:shadow
    //TODO style:background-image
};

#endif
