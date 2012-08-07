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

#ifndef LUT_DOCKER_DOCK_H
#define LUT_DOCKER_DOCK_H

#include <QWidget>
#include <QLabel>

#include <QDockWidget>

#include <KoDockFactoryBase.h>
#include <KoCanvasObserverBase.h>

#include <kis_types.h>

#include "ui_wdglut.h"

#include <OpenColorIO/OpenColorIO.h>


namespace OCIO = OCIO_NAMESPACE;


class KisCanvas2;
class KisDoubleWidget;
class KoZoomAdapter;
class KoColorSpace;
class SqueezedComboBox;
class QCheckBox;
class KComboBox;
class QToolButton;

#include "ocio_display_filter.h"

/**
 * Image overview docker
 *
 * _Should_ provide an image thumbnail with a pan rect and a zoom slider, as well
 * as some pertinent information and the exposure slider. Apart from the exposure
 * slider, this has been broken since 2006 :-(
 */
class LutDockerDock : public QDockWidget, public KoCanvasObserverBase, public Ui_WdgLut
{
    Q_OBJECT

public:

    LutDockerDock(OCIO::ConstConfigRcPtr config);
    ~LutDockerDock();

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

    void updateWidgets();
    void selectOcioConfiguration();
    void resetOcioConfiguration();
    void refillComboboxes();
    void refillViewCombobox();
    void selectLut();
    void clearLut();

private:

    KisCanvas2 *m_canvas;
    OCIO::ConstConfigRcPtr m_ocioConfig;
    OcioDisplayFilter *m_displayFilter;

    bool m_draggingSlider;
    bool m_updateDisplay;


};


#endif // LUT_DOCKER_DOCK_H

