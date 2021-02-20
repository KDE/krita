/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_exposure_gamma_correction_interface.h"

#include <QGlobalStatic>

Q_GLOBAL_STATIC(KisDumbExposureGammaCorrectionInterface, s_instance)

KisExposureGammaCorrectionInterface::~KisExposureGammaCorrectionInterface()
{
}

KisDumbExposureGammaCorrectionInterface*
KisDumbExposureGammaCorrectionInterface::instance()
{
    return s_instance;
}

bool KisDumbExposureGammaCorrectionInterface::canChangeExposureAndGamma() const
{
    return false;
}

qreal KisDumbExposureGammaCorrectionInterface::currentExposure() const
{
    return 0.0;
}

void KisDumbExposureGammaCorrectionInterface::setCurrentExposure(qreal value)
{
    Q_UNUSED(value);
}

qreal KisDumbExposureGammaCorrectionInterface::currentGamma() const
{
    return 1.0;
}

void KisDumbExposureGammaCorrectionInterface::setCurrentGamma(qreal value)
{
    Q_UNUSED(value);
}
