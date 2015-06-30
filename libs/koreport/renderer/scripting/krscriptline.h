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
#ifndef SCRIPTINGKRSCRIPTLINE_H
#define SCRIPTINGKRSCRIPTLINE_H

#include <QObject>
#include <QPointF>
#include <QColor>

class KoReportItemLine;

namespace Scripting
{

/**
 @brief Line item script interface

 The user facing interface for scripting report line items
*/
class Line : public QObject
{
    Q_OBJECT
public:
    explicit Line(KoReportItemLine *);

    ~Line();

public Q_SLOTS:
    /**
     * Return the start position of the line
     * @return start position
     */
    QPointF startPosition() const;

    /**
     * Set the start position of the line
     * @param pos new start poisition in points
     */
    void setStartPosition(const QPointF& pos);

    /**
     * Return the end position of the line
     * @return end position as QPointF
     */
    QPointF endPosition() const;

    /**
     * Set the end position of the line
     * @param pos new end position in points
     */
    void setEndPosition(const QPointF& pos);

    /**
     * Return the color of the line
     * @return line color
     */
    QColor lineColor() const;

    /**
     * Sets the line color
     * @param color new line color
     */
    void setLineColor(const QColor& color);

    /**
     * Return the weight (width) of the line
     * @return weight (width) in points
     */
    int lineWeight() const;

    /**
     * Set the weight (width) of the line
     * @param weight new line weight
     */
    void setLineWeight(int weight);

    /**
     * Return the line style.  Valid values are those from Qt::PenStyle (0-5)
     * @return line style in integer form
     */
    int lineStyle() const;


    /**
     * Set the style of the line
     * @param Style From Qt::PenStyle (0-5)
     */
    void setLineStyle(int);

private:
    KoReportItemLine *m_line;

};

}

#endif
