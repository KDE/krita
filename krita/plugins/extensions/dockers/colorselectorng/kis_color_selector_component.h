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

#include <QWidget>

class KoColorSpace;
class KisColorSelectorBase;

class KisColorSelectorComponent : public QWidget
{
    Q_OBJECT
public:
    explicit KisColorSelectorComponent(KisColorSelectorBase* parent);
protected:
    const KoColorSpace* colorSpace() const;
    virtual void mousePressEvent(QMouseEvent *) = 0;
    
private:
    KisColorSelectorBase* m_parent;

};

#endif // KIS_COLOR_SELECTOR_COMPONENT_H
