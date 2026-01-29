/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef LUT_DOCKER_DOCK_H
#define LUT_DOCKER_DOCK_H

#include "ui_wdglut.h"

#include <QWidget>
#include <QPointer>
#include <QDockWidget>

#include <KoDockFactoryBase.h>
#include <KoCanvasObserverBase.h>

#include <kis_types.h>
#include <kis_canvas2.h>
#include <kis_signal_compressor_with_param.h>

#include <OpenColorIO.h>

#include <config-ocio.h>

namespace OCIO = OCIO_NAMESPACE;

class BlackWhitePointChooser;

#include <config-ocio.h>

#include "ocio_display_filter_vfx2021.h"

#include "kis_exposure_gamma_correction_interface.h"


class LutDockerDock : public QDockWidget, public KoCanvasObserverBase, public Ui_WdgLut, public KisExposureGammaCorrectionInterface
{
    Q_OBJECT

public:

    LutDockerDock();
    ~LutDockerDock() override;
    QString observerName() override
    {
        return "LutDockerDock";
    }
    /// reimplemented from KoCanvasObserverBase
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

    bool canChangeExposureAndGamma() const override;
    qreal currentExposure() const override;
    void setCurrentExposure(qreal value) override;
    void setCurrentGamma(qreal value) override;
    qreal currentGamma() const override;

private Q_SLOTS:

    void slotImageColorSpaceChanged();

    void exposureValueChanged(double exposure);
    void gammaValueChanged(double exposure);

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

private:

    QWidget *m_page;

    QPointer<KisCanvas2> m_canvas;
    OCIO::ConstConfigRcPtr m_ocioConfig;
    QSharedPointer<KisDisplayFilter> m_displayFilter;

    QScopedPointer<KisSignalCompressorWithParam<qreal> > m_exposureCompressor;
    QScopedPointer<KisSignalCompressorWithParam<qreal> > m_gammaCompressor;

    BlackWhitePointChooser *m_bwPointChooser;
};


#endif // LUT_DOCKER_DOCK_H

