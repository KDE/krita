/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
