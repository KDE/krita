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
#ifndef KRSCRIPTFIELD_H
#define KRSCRIPTFIELD_H

#include <QObject>
#include "KoReportItemField.h"

namespace Scripting
{
/**
 @brief Field script interface

 The user facing interface for scripting report fields
*/

class Field : public QObject
{
    Q_OBJECT
public:
    explicit Field(KoReportItemField*);

    ~Field();

public Q_SLOTS:
    //! @returns the source (column) that the field gets its data from
    QString source() const;

    //! Sets the source (column) for the field.
    //! Valid values include a column name, fixed string if prefixed with '$'
    //! or a valid script expression if prefixed with a '='
    void setSource(const QString&);

        //! @return the horizontal alignment as an integer
    //! Valid values are left: -1, center: 0, right; 1
    int horizontalAlignment() const;

    //! Sets the horizontal alignment
    //! Valid values for alignment are left: -1, center: 0, right; 1
    void setHorizonalAlignment(int);

    //! @return the vertical alignment
    //! Valid values are top: -1, middle: 0, bottom: 1
    int verticalAlignment() const;

    //! Sets the vertical alignment
    //! Valid values for aligmnt are top: -1, middle: 0, bottom: 1
    void setVerticalAlignment(int);

    //! @return the background color of the lable
    QColor backgroundColor() const;

    //! Set the background color of the label to the given color
    void setBackgroundColor(const QColor&);

    //! @return the foreground (text) color of the label
    QColor foregroundColor() const;

    //! Sets the foreground (text) color of the label to the given color
    void setForegroundColor(const QColor&);

    //! @return the opacity of the label
    int backgroundOpacity() const;

    //! Sets the background opacity of the label
    //! Valid values are in the range 0-100
    void setBackgroundOpacity(int);

    //! @return the border line color of the label
    QColor lineColor() const;

    //! Sets the border line color of the label to the given color
    void setLineColor(const QColor&);

    //! @return the border line weight (thickness) of the label
    int lineWeight() const;

    //! Sets the border line weight (thickness) of the label
    void setLineWeight(int);

    //! @return the border line style of the label.  Values are from Qt::Penstyle range 0-5
    int lineStyle() const;

    //! Sets the border line style of the label to the given style in the range 0-5
    void setLineStyle(int);

    //! @returns the position of the label in points
    QPointF position() const;

    //! Sets the position of the label to the given point coordinates
    void setPosition(const QPointF&);

    //! @returns the size of the label in points
    QSizeF size() const;

    //! Sets the size of the label to the given size in points
    void setSize(const QSizeF&);

private:
    KoReportItemField *m_field;

};
}
#endif
