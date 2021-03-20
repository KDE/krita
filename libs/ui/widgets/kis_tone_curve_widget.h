/* 
 * SPDX-FileCopyrightText: 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * Based on the Digikam CIE Tongue widget
 * SPDX-FileCopyrightText: 2006-2013 Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Any source code are inspired from lprof project and
 * SPDX-FileCopyrightText: 1998-2001 Marti Maria
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
