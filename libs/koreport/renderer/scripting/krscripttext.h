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
#include <krtextdata.h>

namespace Scripting
{

/**
 @author Adam Pigg <adam@piggz.co.uk>
*/
class Text : public QObject
{
    Q_OBJECT
public:
    Text(KRTextData*);

    ~Text();
public slots:

    /**Returns the source (column) that the field gets its data from*/
    QString source();
    /**Sets the source (column) for the field*/
    void setSource(const QString&);

    /**Gets/sets the horizontal alignment, -1 Left, 0 Center, +1 Right*/
    int horizontalAlignment();
    void setHorizonalAlignment(int);

    /**Gets/sets the vertical alignment, -1 Top, 0 Middle, +1 Bottom*/
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

    /**Gets/sets the line style.  Valid values are those from Qt::PenStyle (0-5)*/
    int lineStyle();
    void setLineStyle(int);

    QPointF position();
    void setPosition(const QPointF&);

    QSizeF size();
    void setSize(const QSizeF&);

    void loadFromFile(const QString&);
private:
    KRTextData *m_text;
};

}

#endif
