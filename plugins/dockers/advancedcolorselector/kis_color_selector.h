/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
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

#ifndef KIS_COLOR_SELECTOR_H
#define KIS_COLOR_SELECTOR_H

#include "kis_color_selector_base.h"
#include <KisColorSelectorConfiguration.h>


class KisColorSelectorRing;
class KisColorSelectorComponent;
class KisColorSelectorSimple;
class KisColorSelectorWheel;
class QPushButton;
class KisSignalCompressor;

class KisColorSelector : public KisColorSelectorBase
{
    Q_OBJECT
public:

    KisColorSelector(KisColorSelectorConfiguration conf, QWidget* parent = 0);
    KisColorSelector(QWidget* parent=0);
    KisColorSelectorBase* createPopup() const override;

    void setConfiguration(KisColorSelectorConfiguration conf);
    KisColorSelectorConfiguration configuration() const;
    void setColor(const KoColor &color) override;

    /// update icons when a theme update happens
    void updateIcons();

    void hasAtLeastOneDocument(bool value);

public Q_SLOTS:
    void reset() override;
    void updateSettings() override;
    void slotGamutMaskSet(KoGamutMask* gamutMask);
    void slotGamutMaskUnset();
    void slotGamutMaskPreviewUpdate();
    void slotGamutMaskToggle(bool state);

Q_SIGNALS:
    void settingsButtonClicked();

protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    bool displaySettingsButton();

private:
    void mouseEvent(QMouseEvent* e);
    void init();

    KisColorSelectorRing* m_ring;
    KisColorSelectorComponent* m_triangle;
    KisColorSelectorSimple* m_slider;
    KisColorSelectorSimple* m_square;
    KisColorSelectorWheel* m_wheel;
    QPushButton* m_button;
    KisColorSelectorComponent* m_mainComponent;
    KisColorSelectorComponent* m_subComponent;
    KisColorSelectorComponent* m_grabbingComponent;

    KisSignalCompressor *m_signalCompressor;

    KisColorSelectorConfiguration m_configuration;

    KoColor m_lastRealColor;
    KoColor m_currentRealColor;
    bool m_blipDisplay;
    Acs::ColorRole m_lastColorRole;


    /// if Krita starts with a reference to this component that is attached to a canvas, it will call setCanvas()
    /// that check will be what ultimately decides whether this component will look enabled or disabled
    /// This color selector is sometimes not attached to the canvas, so we shouldn't disable it in that situation
    /// One instance of that is when you select the color wheel type from the settings.
    bool m_hasAtLeastOneDocumentOpen = true;

public:
    void setDisplayBlip(bool disp) {m_blipDisplay = disp;}
    bool displayBlip() const {return m_blipDisplay;}
};

#endif // KIS_COLSELNG_COLOR_SELECTOR_H
