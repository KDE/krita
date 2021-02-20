/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_CONVOLUTION_FILTER_H_
#define _KIS_CONVOLUTION_FILTER_H_

#include "filter/kis_filter.h"
#include "filter/kis_filter_registry.h"
#include "filter/kis_filter_configuration.h"
#include "kis_convolution_painter.h"

class KisConvolutionFilter : public KisFilter
{

public:
    KisConvolutionFilter(const KoID& id, const KoID & category, const QString & entry);
public:
    void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfigurationSP config,
                     KoUpdater* progressUpdater) const override;

    QRect neededRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const override;
    QRect changedRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const override;

protected:
    void setIgnoreAlpha(bool v);

protected:
    KisConvolutionKernelSP m_matrix;
    bool m_ignoreAlpha;
};

#endif
