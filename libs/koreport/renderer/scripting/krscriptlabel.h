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
#include <krlabeldata.h>

/**
 @author Adam Pigg <adam@piggz.co.uk>
*/
namespace Scripting
{
class Label : public QObject
{
    Q_OBJECT
public:
    Label(KRLabelData *);

    ~Label();

public slots:
    QString caption();
    void setCaption(const QString&);

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

private:
    KRLabelData *m_label;
};
}

#endif
