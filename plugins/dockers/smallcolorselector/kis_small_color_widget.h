/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_SMALL_COLOR_WIDGET_H_
#define _KIS_SMALL_COLOR_WIDGET_H_

#include <QOpenGLWidget>

class KisSmallColorWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    KisSmallColorWidget(QWidget* parent);
    ~KisSmallColorWidget() override;
public:
    void paintEvent(QPaintEvent * event) override;
    void resizeEvent(QResizeEvent * event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void mouseMoveEvent(QMouseEvent * event) override;

    QSize sizeHint() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;

public:
    int hue() const;
    int value() const;
    int saturation() const;
    QColor color() const;
public Q_SLOTS:
    void setHue(int h);
    void setHSV(int h, int s, int v);
    void setQColor(const QColor&);
Q_SIGNALS:
    void colorChanged(const QColor&);
private:
    void tellColorChanged();
    void updateParameters(const QSize &size);
    void generateRubber();
    void generateSquare();
    void selectColorAt(int _x, int _y);
private:
    struct Private;
    Private* const d;
};

#endif
