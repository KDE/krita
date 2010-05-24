/* This file is part of the KDE project
 *
 * Copyright (C) 2009 Inge wallin <inge@lysator.liu.se>
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
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


#ifndef KOBORDER_H
#define KOBORDER_H

#include "koodf_export.h"

#include <QColor>
#include <QSharedData>

#include "KoXmlReaderForward.h"

class KoGenStyle;
class KoBorderPrivate;


/**
 * A container for all properties of a generic border as defined by ODF.
 *
 * A border is used in at least the following contexts:
 *  - paragraph
 *  - page
 *  - table
 *  - table cell
 *
 */

class KOODF_EXPORT KoBorder
{
public:

    /// The type of border.  Note that some of the border types are legacies from the old KWord format.
    enum BorderStyle {
        BorderNone, ///< no border. This value forces the computed value of 'border-width' to be '0'.
        BorderDotted,   ///< The border is a series of dots.
        BorderDashed,   ///< The border is a series of short line segments.
        BorderSolid,    ///< The border is a single line segment.
        BorderDouble,   ///< The border is two solid lines. The sum of the two lines and the space between them equals the value of 'border-width'.
        BorderGroove,   ///< The border looks as though it were carved into the canvas. (old kword type)
        BorderRidge,    ///< The opposite of 'groove': the border looks as though it were coming out of the canvas. (old kword type)
        BorderInset,    ///< The border makes the entire box look as though it were embedded in the canvas. (old kword type)
        BorderOutset,   ///< The opposite of 'inset': the border makes the entire box look as though it were coming out of the canvas. (old kword type)

        // kword legacy
        BorderDashDotPattern,
        BorderDashDotDotPattern
    };

    /// Holds data about one border line.
    struct BorderData {
        BorderData();
        BorderStyle  style; ///< The border style. (see KoBorder::BorderStyle)
        qreal width; ///< The thickness of the border, or 0 if there is no border
        QColor color; ///< The border Color
        /// In case of style being 'double' the thickness of the inner border line
        qreal innerWidth;
        /// In case of style being 'double' the space between the inner and outer border lines
        qreal spacing;
    };


    /// Constructor
    KoBorder();
    KoBorder(const KoBorder &kb);

    /// Destructor
    ~KoBorder();

    /// Assignment
    KoBorder &operator=(const KoBorder &other);
    /// Compare the border with the other one
    bool operator==(const KoBorder &other) const;
    bool operator!=(const KoBorder &other) const { return !operator==(other); }

    void setLeftBorderStyle(BorderStyle style);
    BorderStyle leftBorderStyle() const;
    void setLeftBorderColor(const QColor &color);
    QColor leftBorderColor() const;
    void setLeftBorderWidth(qreal width);
    qreal leftBorderWidth() const;
    void setLeftInnerBorderWidth(qreal width);
    qreal leftInnerBorderWidth() const;
    void setLeftBorderSpacing(qreal width);
    qreal leftBorderSpacing() const;

    void setTopBorderStyle(BorderStyle style);
    BorderStyle topBorderStyle() const;
    void setTopBorderColor(const QColor &color);
    QColor topBorderColor() const;
    void setTopBorderWidth(qreal width);
    qreal topBorderWidth() const;
    void setTopInnerBorderWidth(qreal width);
    qreal topInnerBorderWidth() const;
    void setTopBorderSpacing(qreal width);
    qreal topBorderSpacing() const;

    void setRightBorderStyle(BorderStyle style);
    BorderStyle rightBorderStyle() const;
    void setRightBorderColor(const QColor &color);
    QColor rightBorderColor() const;
    void setRightBorderWidth(qreal width);
    qreal rightBorderWidth() const;
    void setRightInnerBorderWidth(qreal width);
    qreal rightInnerBorderWidth() const;
    void setRightBorderSpacing(qreal width);
    qreal rightBorderSpacing() const;

    void setBottomBorderStyle(BorderStyle style);
    BorderStyle bottomBorderStyle() const;
    void setBottomBorderColor(const QColor &color);
    QColor bottomBorderColor() const;
    void setBottomBorderWidth(qreal width);
    qreal bottomBorderWidth() const;
    void setBottomInnerBorderWidth(qreal width);
    qreal bottomInnerBorderWidth() const;
    void setBottomBorderSpacing(qreal width);
    qreal bottomBorderSpacing() const;

    BorderData leftBorderData() const;
    BorderData topBorderData() const;
    BorderData rightBorderData() const;
    BorderData bottomBorderData() const;


    /**
     * Load the style from the element
     *
     * @param style  the element containing the style to read from
     */
    void loadOdf(const KoXmlElement &style);

    void saveOdf(KoGenStyle &style) const;


    // Some public functions used in other places where borders are handled.
    // Example: KoParagraphStyle
    // FIXME: These places should be made to use KoBorder instead.
    static BorderStyle odfBorderStyle(const QString &borderstyle);
    static QString odfBorderStyleString(BorderStyle borderstyle);

private:
    QSharedDataPointer<KoBorderPrivate> d;
};

#endif
