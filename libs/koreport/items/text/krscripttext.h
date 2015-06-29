/*
 * Kexi Report Plugin
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SCRIPTINGKRSCRIPTTEXT_H
#define SCRIPTINGKRSCRIPTTEXT_H

#include <QObject>
#include "KoReportItemText.h"

namespace Scripting
{

/**
 @brief Text item script interface

 The user facing interface for scripting report text items
*/
class Text : public QObject
{
    Q_OBJECT
public:
    explicit Text(KoReportItemText*);

    ~Text();
public Q_SLOTS:

    //! @returns the source (column) that the field gets its data from
    QString source() const;

    //! Sets the source (column) for the field.
    //! @param src new source for the item data
    void setSource(const QString& src);

    //! @return the horizontal alignment as an integer
    //! Valid values are left: -1, center: 0, right; 1
    int horizontalAlignment() const;

    //! Sets the horizontal alignment
    //! Valid values for alignment are left: -1, center: 0, right; 1
    //! @param align new horizontal alignment
    void setHorizonalAlignment(int align);

    //! @return the vertical alignment
    //! Valid values are top: -1, middle: 0, bottom: 1
    int verticalAlignment() const;

    //! Sets the vertical alignment
    //! Valid values for aligmnt are top: -1, middle: 0, bottom: 1
    //! @param align new vertical alignment
    void setVerticalAlignment(int align);

    //! @return the background color of the lable
    QColor backgroundColor() const;

    //! Set the background color of the label to the given color
    //! @param color new background color as a QColor
    void setBackgroundColor(const QColor& color);

    //! @return the foreground (text) color of the label
    QColor foregroundColor() const;

    //! Sets the foreground (text) color of the label to the given color
    //! @param color new text color as a QColor
    void setForegroundColor(const QColor& color);

    //! @return the opacity of the label
    int backgroundOpacity() const;

    //! Sets the background opacity of the label
    //! Valid values are in the range 0-100
    //! @param opacity new opacity as a percentage
    void setBackgroundOpacity(int opactity);

    //! @return the border line color of the label
    QColor lineColor() const;

    //! Sets the border line color of the label to the given color
    //! @param color new line color
    void setLineColor(const QColor& color);

    //! @return the border line weight (thickness) of the label
    int lineWeight() const;

    //! Sets the border line weight (thickness) of the label
    //! @param weight in points
    void setLineWeight(int weight);

    //! @return the border line style of the label.  Values are from Qt::Penstyle range 0-5
    int lineStyle() const;

    //! Sets the border line style of the label to the given style in the range 0-5
    //! @param style integer representation of Qt::Penstyle
    void setLineStyle(int style);

    //! @returns the position of the label in points
    QPointF position() const;

    //! Sets the position of the label to the given point coordinates
    //! @param pos new position for item as QPointF
    void setPosition(const QPointF& pos);

    //! @returns the size of the label in points
    QSizeF size() const;

    //! Sets the size of the label to the given size in points
    //! @param size new size for item as QSizeF
    void setSize(const QSizeF& size);

    //! Loads text from the given file into the item
    //! @param path file to load from
    void loadFromFile(const QString& path);
private:
    KoReportItemText *m_text;
};

}

#endif
