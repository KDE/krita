/* 
 * Copyright (C) 2015 by Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * Based on the Digikam CIE Tongue widget
 * Copyright (C) 2006-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Any source code are inspired from lprof project and
 * Copyright (C) 1998-2001 Marti Maria
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 **/

#ifndef KIS_TONECURVEWIDGET_H
#define KIS_TONECURVEWIDGET_H
 
#include <QWidget>
#include <QColor>
#include <QPaintEvent>

#include <kritaui_export.h>
 
class KRITAUI_EXPORT KisToneCurveWidget : public QWidget
{
    Q_OBJECT
 
public:
 
    KisToneCurveWidget(QWidget *parent=0);
    ~KisToneCurveWidget() override;

    void setGreyscaleCurve(QPolygonF poly);
    void setRGBCurve(QPolygonF red, QPolygonF green, QPolygonF blue);
    void setCMYKCurve(QPolygonF cyan, QPolygonF magenta, QPolygonF yellow, QPolygonF key);
    void setProfileDataAvailable(bool dataAvailable);
protected:
 
    int  grids(double val) const;
    void drawGrid();
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent*) override;
 
private:
 
    void updatePixmap();
    void mapPoint(QPointF & xy);
    void biasedLine(int x1, int y1, int x2, int y2);
    void biasedText(int x, int y, const QString& txt);
private :
 
    class Private;
    Private* const d;
};

#endif /* KISTONECURVEWIDGET_H */
