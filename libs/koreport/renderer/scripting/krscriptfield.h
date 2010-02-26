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
#include <krfielddata.h>

/**
 @author Adam Pigg <adam@piggz.co.uk>
*/
namespace Scripting
{
class Field : public QObject
{
    Q_OBJECT
public:
    Field(KRFieldData*);

    ~Field();

public slots:
    /**Returns the source (column) that the field gets its data from*/
    QString source();
    /**Sets the source (column) for the field*/
    void setSource(const QString&);

    int horizontalAlignment();
    void setHorizonalAlignment(int);

    int verticalAlignment();
    void setVerticalAlignment(int);

    QColor backgroundColor();
    void setBackgroundColor(const QColor&);

    QColor foregroundColor();
    void setForegroundColor(const QColor&);

    int backgroundOpacity();
    void setBackgroundOpacity(int);

    QColor lineColor();
    void setLineColor(const QColor&);

    int lineWeight();
    void setLineWeight(int);

    int lineStyle();
    void setLineStyle(int);

    QPointF position();
    void setPosition(const QPointF&);

    QSizeF size();
    void setSize(const QSizeF&);
private:
    KRFieldData *m_field;

};
}
#endif
