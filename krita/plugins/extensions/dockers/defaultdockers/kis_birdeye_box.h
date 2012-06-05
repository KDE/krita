/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_BIRDEYE_BOX_H
#define KIS_BIRDEYE_BOX_H

#include <QWidget>
#include <QLabel>

#include <QDockWidget>

#include <KoDockFactoryBase.h>
#include <KoCanvasObserverBase.h>

#include <kis_types.h>

class KisCanvas2;
class KoBirdEyePanel;
class KisDoubleWidget;
class KoZoomAdapter;
class KoColorSpace;
class SqueezedComboBox;
class QCheckBox;
class KComboBox;
/**
 * Image overview docker
 *
 * _Should_ provide an image thumbnail with a pan rect and a zoom slider, as well
 * as some pertinent information and the exposure slider. Apart from the exposure
 * slider, this has been broken since 2006 :-(
 */
class KisBirdEyeBox : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT

public:

    KisBirdEyeBox();
    ~KisBirdEyeBox();

    /// reimplemented from KoCanvasObserverBase
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas() { m_canvas = 0; }

private slots:

    void slotImageColorSpaceChanged();
    void exposureValueChanged(double exposure);
    void exposureSliderPressed();
    void exposureSliderReleased();
    void gammaValueChanged(double exposure);
    void gammaSliderPressed();
    void gammaSliderReleased();
    void updateDisplaySettings();


private:

    KisCanvas2 *m_canvas;

    KoBirdEyePanel *m_birdEyePanel;
    KisDoubleWidget *m_exposureDoubleWidget;
    KisDoubleWidget *m_gammaDoubleWidget;
    SqueezedComboBox *m_cmbDisplayProfile;
    KComboBox *m_cmbMonitorIntent;
    QCheckBox *m_chkBlackPoint;

    bool m_draggingSlider;
};


class KisBirdEyeBoxFactory : public KoDockFactoryBase
{
public:
    KisBirdEyeBoxFactory() {}

    virtual QString id() const {
        return QString("KisBirdeyeBox");
    }

    virtual QDockWidget* createDockWidget() {
        KisBirdEyeBox * dockWidget = new KisBirdEyeBox();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const {
        return DockMinimized;
    }
};

#endif // KIS_BIRDEYE_BOX_H
