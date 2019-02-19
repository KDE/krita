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
#include <QColor>

#include <resources/KoGamutMask.h>

#include "kis_color_selector.h"

class KoColorSpace;

class QPainter;


class KisColorSelectorComponent : public QObject
{
    Q_OBJECT
public:
    typedef KisColorSelectorConfiguration::Parameters Parameter;
    typedef KisColorSelectorConfiguration::Type Type;

    explicit KisColorSelectorComponent(KisColorSelector* parent);
    void setGeometry(int x, int y, int width, int height);
    void paintEvent(QPainter*);

    /// saves the mouse position, so that a blip can be created.
    virtual void mouseEvent(int x, int y);

    /// return the color, that was selected by calling mouseEvent
    KoColor currentColor();

    int width() const;
    int height() const;

    /// setConfiguration can be ignored (for instance ring and triangle, as they can have only one config)
    void setConfiguration(Parameter param, Type type);

    /// set the color, blibs etc
    virtual void setColor(const KoColor& color);

    /// force subsequent redraw of the component
    void setDirty();

    /// returns true, if this component wants to grab the mouse (normally true, if containsPoint returns true)
    bool wantsGrab(int x, int y) {return containsPointInComponentCoords(x-m_x, y-m_y);}

    void setGamutMask(KoGamutMask* gamutMask);
    void unsetGamutMask();
    void updateGamutMaskPreview();
    void toggleGamutMask(bool state);

public Q_SLOTS:
    /// set hue, saturation, value or/and lightness
    /// unused parameters should be set to -1
    void setParam(qreal hue, qreal hsvSaturation, qreal value, qreal hslSaturation, qreal lightness, qreal hsiSaturation, qreal intensity, qreal hsySaturation, qreal luma);
Q_SIGNALS:
    /// request for repaint, for instance, if the hue changes.
    void update();
    /// -1, if unaffected
    void paramChanged(qreal hue, qreal hsvSaturation, qreal value, qreal hslSaturation, qreal lightness, qreal hsiSaturation, qreal intensity, qreal hsySaturation, qreal luma);
protected:
    const KoColorSpace* colorSpace() const;
    /// returns true, if ether the colour space, the size or the parameters have changed since the last paint event
    bool isDirty() const;

    /// this method must be overloaded to return the colour at position x/y and draw a marker on that position
    virtual KoColor selectColor(int x, int y) = 0;

    /// paint component using given painter
    /// the component should respect width() and height() (eg. scale to width and height), but doesn't
    /// have to care about x/y coordinates (top left corner)
    virtual void paint(QPainter*) = 0;

    /// a subclass can implement this method, the default returns true if the coordinates are in the component rect
    /// values for the subclasses are provided in component coordinates, eg (0,0) is top left of component
    virtual bool containsPointInComponentCoords(int x, int y) const;

    /// a subclass can implement this method to note that the point, although it is in
    /// containsPointInComponentCoords area, still cannot be selected as a color (e.g.
    /// it is masked out). Default implementation always returns true.
    virtual bool allowsColorSelectionAtPoint(const QPoint &pt) const;

    // Workaround for Bug 287001
    void setLastMousePosition(int x, int y);

    qreal m_hue;
    qreal m_hsvSaturation;
    qreal m_value;
    qreal m_hslSaturation;
    qreal m_lightness;
    qreal m_hsiSaturation;
    qreal m_intensity;
    qreal m_hsySaturation;
    qreal m_luma;
    Parameter m_parameter;
    Type m_type;
    KisColorSelector* m_parent;
    bool m_gamutMaskOn;
    KoGamutMask* m_currentGamutMask;
    bool m_maskPreviewActive;
    qreal m_lastX;
    qreal m_lastY;
    int m_x;
    int m_y;
private:
    int m_width;
    int m_height;
    bool m_dirty;
    const KoColorSpace* m_lastColorSpace;
    KoColor m_lastSelectedColor;
};

#endif // KIS_COLOR_SELECTOR_COMPONENT_H
