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

#ifndef __KIS_EXPOSURE_GAMMA_CORRECTION_INTERFACE_H
#define __KIS_EXPOSURE_GAMMA_CORRECTION_INTERFACE_H

#include <QtGlobal>
#include <kritaui_export.h>

/**
 * A special interface for OCIO filter providing functionality for the
 * main UI module. See Dependency Inversion Principle for more.
 */
struct KRITAUI_EXPORT KisExposureGammaCorrectionInterface {
    virtual ~KisExposureGammaCorrectionInterface();
    virtual bool canChangeExposureAndGamma() const = 0;
    virtual qreal currentExposure() const = 0;
    virtual void setCurrentExposure(qreal value) = 0;
    virtual qreal currentGamma() const = 0;
    virtual void setCurrentGamma(qreal value) = 0;
};

struct KRITAUI_EXPORT KisDumbExposureGammaCorrectionInterface : public KisExposureGammaCorrectionInterface
{
    static KisDumbExposureGammaCorrectionInterface* instance();

    bool canChangeExposureAndGamma() const override;
    qreal currentExposure() const override;
    void setCurrentExposure(qreal value) override;
    qreal currentGamma() const override;
    void setCurrentGamma(qreal value) override;
};

#endif /* __KIS_EXPOSURE_GAMMA_CORRECTION_INTERFACE_H */
