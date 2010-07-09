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
class QPaintEvent;

class KisColorSelectorComponent : public QObject
{
    Q_OBJECT
public:
    explicit KisColorSelectorComponent(KisColorSelectorBase* parent);
signals:
    void update();
protected:
    const KoColorSpace* colorSpace() const;
    /// the component must not react on middle click (used for zooming)
    virtual void mousePressEvent(QMouseEvent *) = 0;
    /// paint component using given painter
    virtual void paintEvent(QPaintEvent *, QPainter*) = 0;

    int width() const;
    int height() const;
    
private:
    KisColorSelectorBase* m_parent;
};

#endif // KIS_COLOR_SELECTOR_COMPONENT_H
