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
#include "kis_signal_compressor_with_param.h"


namespace OCIO = OCIO_NAMESPACE;


class KisCanvas2;
class KisDoubleWidget;
class KoZoomAdapter;
class SqueezedComboBox;
class QCheckBox;
class QToolButton;
class BlackWhitePointChooser;

#include "ocio_display_filter.h"
#include "kis_exposure_gamma_correction_interface.h"


class LutDockerDock : public QDockWidget, public KoCanvasObserverBase, public Ui_WdgLut, public KisExposureGammaCorrectionInterface
{
    Q_OBJECT

public:

    LutDockerDock();
    ~LutDockerDock();

    /// reimplemented from KoCanvasObserverBase
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas() { m_canvas = 0; }

    bool canChangeExposureAndGamma() const;
    qreal currentExposure() const;
    void setCurrentExposure(qreal value);
    qreal currentGamma() const;
    void setCurrentGamma(qreal value);

private slots:

    void slotImageColorSpaceChanged();

    void exposureValueChanged(double exposure);
    void exposureSliderPressed();
    void exposureSliderReleased();

    void gammaValueChanged(double exposure);
    void gammaSliderPressed();
    void gammaSliderReleased();

    void updateDisplaySettings();

    void slotColorManagementModeChanged();

    void writeControls();
    void selectOcioConfiguration();
    void resetOcioConfiguration();
    void refillViewCombobox();
    void selectLut();
    void clearLut();

    void slotShowBWConfiguration();

    void slotUpdateIcons();

private:
    void enableControls();
    void refillControls();

    void setCurrentExposureImpl(qreal value);
    void setCurrentGammaImpl(qreal value);

private:

    QWidget *m_page;

    KisCanvas2 *m_canvas;
    OCIO::ConstConfigRcPtr m_ocioConfig;
    OcioDisplayFilterSP m_displayFilter;

    bool m_draggingSlider;

    QScopedPointer<KisSignalCompressorWithParam<qreal> > m_exposureCompressor;
    QScopedPointer<KisSignalCompressorWithParam<qreal> > m_gammaCompressor;

    BlackWhitePointChooser *m_bwPointChooser;
};


#endif // LUT_DOCKER_DOCK_H

