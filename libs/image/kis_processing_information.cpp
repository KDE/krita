/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_processing_information.h"
#include "kis_paint_device.h"
#include "kis_selection.h"
#include "kis_image.h"

struct Q_DECL_HIDDEN KisConstProcessingInformation::Private {
    Private() : device(0), selection(0) {}
    KisPaintDeviceSP device;
    KisSelectionSP selection;
    QPoint topLeft;
};

KisConstProcessingInformation::KisConstProcessingInformation(const KisPaintDeviceSP device, const QPoint& topLeft, const KisSelectionSP selection) : d(new Private)
{
    d->device = device;
    d->selection = selection;
    d->topLeft = topLeft;
}

KisConstProcessingInformation::KisConstProcessingInformation(const KisConstProcessingInformation& _rhs) : d(new Private)
{
    *d = *_rhs.d;
}

KisConstProcessingInformation& KisConstProcessingInformation::operator=(const KisConstProcessingInformation & _rhs)
{
    *d = *_rhs.d;
    return *this;
}

KisConstProcessingInformation::~KisConstProcessingInformation()
{
    delete d;
}

const KisPaintDeviceSP KisConstProcessingInformation::paintDevice() const
{
    return d->device;
}

const KisSelectionSP KisConstProcessingInformation::selection() const
{
    return d->selection;
}

const QPoint& KisConstProcessingInformation::topLeft() const
{
    return d->topLeft;
}

struct Q_DECL_HIDDEN KisProcessingInformation::Private {
    KisPaintDeviceSP device;
};

KisProcessingInformation::KisProcessingInformation(KisPaintDeviceSP device, const QPoint& topLeft, const KisSelectionSP selection) : KisConstProcessingInformation(device, topLeft, selection), d(new Private)
{
    d->device = device;
}

KisProcessingInformation::KisProcessingInformation(const KisProcessingInformation& _rhs) : KisConstProcessingInformation(_rhs), d(new Private(*_rhs.d))
{
}

KisProcessingInformation& KisProcessingInformation::operator=(const KisProcessingInformation & _rhs)
{
    *d = *_rhs.d;
    KisConstProcessingInformation::operator=(_rhs);
    return *this;
}

KisProcessingInformation::~KisProcessingInformation()
{
    delete d;
}

KisPaintDeviceSP KisProcessingInformation::paintDevice()
{
    return d->device;
}
