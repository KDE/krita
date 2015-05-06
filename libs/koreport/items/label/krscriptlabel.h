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
#ifndef KRSCRIPTLABEL_H
#define KRSCRIPTLABEL_H

#include <QObject>
#include "KoReportItemLabel.h"

/**
 @author Adam Pigg <adam@piggz.co.uk>
*/
namespace Scripting
{

/**
 @brief Label script interface

 The user facing interface for scripting report labels

 @author Adam Pigg <adam@piggz.co.uk>.
*/

class Label : public QObject
{
    Q_OBJECT
public:
    explicit Label(KoReportItemLabel *);

    ~Label();

public Q_SLOTS:

    //! @returns the caption (text) for the label
    QString caption() const;

    //! Sets the caption (text) of the label to the given string
    void setCaption(const QString&);

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
    KoReportItemLabel *m_label;
};
}

#endif
