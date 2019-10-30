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
#include <QPointer>
#include <kis_canvas2.h>
#include "kis_acs_types.h"
#include "kis_signal_compressor_with_param.h"


class KoColor;
class QTimer;
class KoColorSpace;
class KisCanvas2;
class KisColorPreviewPopup;
class KisDisplayColorConverter;
class KoGamutMask;


/// Base class for all color selectors, that should support color management and zooming.
class KisColorSelectorBase : public QWidget
{
Q_OBJECT
public:
    enum Move {MoveToMousePosition, DontMove};
    explicit KisColorSelectorBase(QWidget *parent = 0);
    ~KisColorSelectorBase() override;

    void setPopupBehaviour(bool onMouseOver, bool onMouseClick);
    void setColorSpace(const KoColorSpace* colorSpace);
    virtual void setCanvas(KisCanvas2* canvas);
    virtual void unsetCanvas();
    const KoColorSpace* colorSpace() const;

    KisDisplayColorConverter* converter() const;

    void tryHideAllPopups();

public:
    void updateColor(const KoColor &color, Acs::ColorRole role, bool needsExplicitColorReset);
    void updateColorPreview(const KoColor &color);
    void showColorPreview();
    void updateBaseColorPreview(const KoColor &color);
    void updatePreviousColorPreview();

    virtual void setColor(const KoColor& color);

public Q_SLOTS:
    /**
     * Flushes caches and redraws the selectors
     */
    virtual void reset();

    virtual void updateSettings();
    virtual void showPopup(Move move=MoveToMousePosition);

public:
    void enterEvent(QEvent *e) override;
    void leaveEvent(QEvent *e) override;

    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;

protected:
    void keyPressEvent(QKeyEvent *) override;
    virtual KisColorSelectorBase* createPopup() const = 0;
    void dragEnterEvent(QDragEnterEvent *) override;
    void dropEvent(QDropEvent *) override;
    void setHidingTime(int time);
    bool isPopup() const { return m_isPopup; }
    void mouseMoveEvent(QMouseEvent *event) override;
    void changeEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void requestUpdateColorAndPreview(const KoColor &color, Acs::ColorRole role);

private:
    void commitColor(const KoColor& koColor, Acs::ColorRole role);


protected Q_SLOTS:
    void hidePopup();

    /// if you overwrite this, keep in mind, that you should set the color only, if m_colorUpdateAllowed is true
    virtual void canvasResourceChanged(int key, const QVariant& v);

    void updateLastUsedColorPreview(const KoColor &color);

public:
    // This is a private interface for signal compressor, don't use it.
    // Use requestUpdateColorAndPreview() instead
    void slotUpdateColorAndPreview(QPair<KoColor, Acs::ColorRole> color);

private:
    void lazyCreatePopup();

protected:
    QPointer<KisCanvas2> m_canvas;
    KisColorSelectorBase* m_popup;
    QWidget* m_parent;
    bool m_colorUpdateAllowed;

    // This boolean here is to check if the color selector is updating the resource, so it won't update itself when the resource is updated.
    bool m_colorUpdateSelf;

private:
    QTimer* m_hideTimer;
    bool m_popupOnMouseOver;
    bool m_popupOnMouseClick;
    mutable const KoColorSpace* m_colorSpace;
    bool m_isPopup; //this instance is a popup
    bool m_hideOnMouseClick;
    KisColorPreviewPopup* m_colorPreviewPopup;

    typedef KisSignalCompressorWithParam<QPair<KoColor, Acs::ColorRole>> ColorCompressorType;
    QScopedPointer<ColorCompressorType> m_updateColorCompressor;
};

#endif
