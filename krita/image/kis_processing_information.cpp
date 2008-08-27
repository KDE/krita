/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_processing_information.h"
#include "kis_paint_device.h"
#include "kis_selection.h"

struct KisConstProcessingInformation::Private {
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

struct KisProcessingInformation::Private {
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
