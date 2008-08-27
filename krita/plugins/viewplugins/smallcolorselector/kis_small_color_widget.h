/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_SMALL_COLOR_WIDGET_H_
#define _KIS_SMALL_COLOR_WIDGET_H_

#include <QWidget>

class KisSmallColorWidget : public QWidget
{
    Q_OBJECT
public:
    KisSmallColorWidget(QWidget* parent);
    ~KisSmallColorWidget();
public:
    void paintEvent(QPaintEvent * event);
    void resizeEvent(QResizeEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
public:
    int hue() const;
    int value() const;
    int saturation() const;
    QColor color() const;
public slots:
    void setHue(int h);
    void setHSV(int h, int s, int v);
    void setQColor(const QColor&);
signals:
    void colorChanged(const QColor&);
private:
    void tellColorChanged();
    void updateParameters();
    void generateRubber();
    void generateSquare();
    void selectColorAt(int _x, int _y);
private:
    struct Private;
    Private* const d;
};

#endif
