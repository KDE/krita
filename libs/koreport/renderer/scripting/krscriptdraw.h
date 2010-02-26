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
#ifndef KRSCRIPTDRAW_H
#define KRSCRIPTDRAW_H

#include <QObject>
#include <QPointF>

class OROPage;

/**
 @author Adam Pigg <adam@piggz.co.uk>
*/
class KRScriptDraw : public QObject
{
    Q_OBJECT
public:
    KRScriptDraw(QObject *parent = 0);

    ~KRScriptDraw();
    void setPage(OROPage*);
    void setOffset(QPointF);
public slots:
    /**
    Draw a rectangle
    \param x X posistion
    \param y Y position
    \param w Width
    \param h Height
    \param lc Line Color
    \param fc Fill Color
    \param lw Line Width
    \param o Opacity (0=transparent, 100=opaque)
    */
    void rectangle(qreal, qreal, qreal, qreal, const QString&, const QString&, qreal, int);

    /**
    Draw an ellipse
    \param x X posistion
    \param y Y position
    \param w Width
    \param h Height
    \param lc Line Color
    \param fc Fill Color
    \param lw Line Width
    \param o Opacity (0=transparent, 100=opaque)
     */
    void ellipse(qreal, qreal, qreal, qreal, const QString&, const QString&, qreal, int);

    /**
    Draw a line
    \param x1 Start X position
    \param y1 Start Y Position
    \param x2 End X position
    \param y2 End Y position
    \param lc Line Color
    */
    void line(qreal, qreal, qreal, qreal, const QString&);

    /**
    Draw some text
    \param x X Position
    \param y Y Position
    \param txt The text
    \param fnt The font
    \param pt Point size
    \param fc Foreground color
    \param bc Background color
    \param lc Line color
    \param lw Line width
    \param o Opacity (0=transparent, 100=opaque)
    */
    void text(qreal, qreal, const QString &, const QString &fnt = "Helvetica", int pt = 12, const QString &fc = "#000000", const QString &bc = "#ffffff", const QString &lc = "#ffffff", qreal lw = 0, int o = 0);
private:
    OROPage *m_curPage;
    QPointF m_curOffset;

};

#endif
