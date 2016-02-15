/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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
