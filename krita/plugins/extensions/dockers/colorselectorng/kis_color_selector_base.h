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

#ifndef KIS_COLOR_SELECTOR_BASE_H
#define KIS_COLOR_SELECTOR_BASE_H

#include <QWidget>
#include <QRgb>

class QColor;
class KoColor;
class QTimer;
class KoColorSpace;
class KisCanvas2;
class KisCanvas2;

/// Base class for all color selectors, that should support color management and zooming.
class KisColorSelectorBase : public QWidget
{
Q_OBJECT
public:
    explicit KisColorSelectorBase(QWidget *parent = 0);
    ~KisColorSelectorBase();

    void setPopupBehaviour(bool onMouseOver, bool onMouseClick);
    void setColorSpace(const KoColorSpace* colorSpace);
    virtual void setCanvas(KisCanvas2* canvas);
    const KoColorSpace* colorSpace() const;
    enum ColorRole {Foreground, Background};

public slots:
    virtual void updateSettings();
    virtual void setColor(const QColor& color);
    virtual void showPopup();
    /// commits a color to the resource manager
    void commitColor(const KoColor& koColor, ColorRole role);

public:
    void mousePressEvent(QMouseEvent *);

    /// finds a QColor, that will be ref.toQColor(), if converting it to the color space of ref
    QColor findGeneratingColor(const KoColor& ref) const;

protected:
    void mouseMoveEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *);
    virtual KisColorSelectorBase* createPopup() const = 0;
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

protected slots:
    void hidePopup();

    /// if you overwrite this, keep in mind, that you should set the colour only, if m_colorUpdateAllowed is true
    virtual void resourceChanged(int key, const QVariant& v);

protected:
    KisCanvas2* m_canvas;
    KisColorSelectorBase* m_popup;
    QWidget* m_parent;
    bool m_colorUpdateAllowed;

private:
    int m_hideDistance;
    QTimer* m_timer;
    bool m_popupOnMouseOver;
    bool m_popupOnMouseClick;
    mutable const KoColorSpace* m_colorSpace;
    bool m_isPopup; //this instance is a popup
};

#endif
