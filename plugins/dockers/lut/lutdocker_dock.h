/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef LUT_DOCKER_DOCK_H
#define LUT_DOCKER_DOCK_H

#include <QWidget>
#include <QLabel>
#include <QPointer>
#include <QDockWidget>

#include <KoDockFactoryBase.h>
#include <KoCanvasObserverBase.h>

#include <kis_types.h>
#include <kis_canvas2.h>

#include "ui_wdglut.h"

#include <OpenColorIO/OpenColorIO.h>
#include "kis_signal_compressor_with_param.h"


namespace OCIO = OCIO_NAMESPACE;

class BlackWhitePointChooser;

#include "ocio_display_filter.h"
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

