/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef LUT_DOCKER_DOCK_H
#define LUT_DOCKER_DOCK_H

#include "ui_wdglut.h"

#include <QWidget>
#include <QLabel>
#include <QPointer>
#include <QDockWidget>

#include <KoDockFactoryBase.h>
#include <KoCanvasObserverBase.h>

#include <kis_types.h>
#include <kis_canvas2.h>
#include <kis_signal_compressor_with_param.h>

#include <OpenColorIO.h>

#include <config-ocio.h>

#ifndef HAVE_OCIO_V2
#define OCIO_VERSION_FULL_STR OCIO_VERSION
#endif

namespace OCIO = OCIO_NAMESPACE;

class BlackWhitePointChooser;

#include <config-ocio.h>

#ifdef HAVE_OCIO_V2
#include "ocio_display_filter_vfx2021.h"
#else
#include "ocio_display_filter_vfx2020.h"
#endif

#include "kis_exposure_gamma_correction_interface.h"


class LutDockerDock : public QDockWidget, public KoCanvasObserverBase, public Ui_WdgLut, public KisExposureGammaCorrectionInterface
{
    Q_OBJECT

public:

    LutDockerDock();
    ~LutDockerDock();
    QString observerName() { return "LutDockerDock"; }
    /// reimplemented from KoCanvasObserverBase
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas();

    bool canChangeExposureAndGamma() const;
    qreal currentExposure() const;
    void setCurrentExposure(qreal value);
    qreal currentGamma() const;
    void setCurrentGamma(qreal value);

private Q_SLOTS:

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

    QPointer<KisCanvas2> m_canvas;
    OCIO::ConstConfigRcPtr m_ocioConfig;
    QSharedPointer<KisDisplayFilter> m_displayFilter;

    bool m_draggingSlider;

    QScopedPointer<KisSignalCompressorWithParam<qreal> > m_exposureCompressor;
    QScopedPointer<KisSignalCompressorWithParam<qreal> > m_gammaCompressor;

    BlackWhitePointChooser *m_bwPointChooser;
};


#endif // LUT_DOCKER_DOCK_H

