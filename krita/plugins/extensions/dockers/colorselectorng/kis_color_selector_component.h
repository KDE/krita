/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 */

#ifndef KIS_COLOR_SELECTOR_COMPONENT_H
#define KIS_COLOR_SELECTOR_COMPONENT_H

#include <QObject>

class KoColorSpace;
class KisColorSelectorBase;

class QMouseEvent;
class QPainter;
class QResizeEvent;
class QColor;

class KisColorSelectorComponent : public QObject
{
    Q_OBJECT
public:
    explicit KisColorSelectorComponent(KisColorSelectorBase* parent);
    void setGeometry(int x, int y, int width, int height);
    void paintEvent(QPainter*);
    void mouseEvent(int x, int y);
    virtual QColor currentColor();
    int width() const;
    int height() const;
public slots:
    /// can be used to set the "third" parameter.
    /// for instance another component selects the hue, which is passed here, and this one selects saturation and value
    void setParam(qreal p);
    /// can be used to set the "second" and "third" parameter.
    /// for instance another component selects hue and saturation, which is passed here, and this one selects value
    void setParam(qreal p1, qreal p2);
signals:
    /// request for repaint, for instance, if the hue changes.
    void update();
    void paramChanged(qreal);
    void paramChanged(qreal, qreal);
protected:
    const KoColorSpace* colorSpace() const;
    virtual void selectColor(int x, int y) = 0;

    /// paint component using given painter
    /// the component should respect width() and height() (eg. scale to width and height), but doesn't
    /// have to care about x/y coordinates (top left corner)
    virtual void paint(QPainter*) = 0;

    qreal parameter1() const;
    qreal parameter2() const;
private:
    KisColorSelectorBase* m_parent;
    int m_width;
    int m_height;
    int m_x;
    int m_y;
    qreal m_param1;
    qreal m_param2;
};

#endif // KIS_COLOR_SELECTOR_COMPONENT_H
