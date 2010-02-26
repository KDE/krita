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

class KRLineData;

namespace Scripting
{

/**
 @author Adam Pigg <adam@piggz.co.uk>
*/
class Line : public QObject
{
    Q_OBJECT
public:
    Line(KRLineData *);

    ~Line();

public slots:
    /**
     * Return the start position of the line
     * @return start position
     */
    QPointF startPosition();

    /**
     * Set the start position of the line
     * @param StartPosition
     */
    void setStartPosition(const QPointF&);

    /**
     * Return the end position of the line
     * @return end position
     */
    QPointF endPosition();

    /**
     * Set the end position of the line
     * @param EndPosition
     */
    void setEndPosition(const QPointF&);

    /**
     * Return the color of the line
     * @return line color
     */
    QColor lineColor();

    /**
     * Sets the line color
     * @param LineColor
     */
    void setLineColor(const QColor&);

    /**
     * Return the weight (width) of the line
     * @return Weight
     */
    int lineWeight();

    /**
     * Set the weight (width) of the line
     * @param Weight
     */
    void setLineWeight(int);

    /**
     * Return the line style.  Valid values are those from Qt::PenStyle (0-5)
     * @return Style
     */
    int lineStyle();


    /**
     * Set the style of the line
     * @param Style From Qt::PenStyle (0-5)
     */
    void setLineStyle(int);

private:
    KRLineData *m_line;

};

}

#endif
