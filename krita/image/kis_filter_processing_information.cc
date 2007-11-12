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

#include "kis_filter_processing_information.h"

#include "kis_paint_device.h"
#include "kis_selection.h"

struct KisFilterConstantProcessingInformation::Private
{
    Private() {};
    KisPaintDeviceSP device;
    const KisSelectionSP selection;
    QPoint topLeft;
};

KisFilterConstantProcessingInformation::KisFilterConstantProcessingInformation(const KisPaintDeviceSP &device, const QPoint& topLeft, const KisSelectionSP selection) : d(new Private)
{
    d->device = device;
    d->selection = selection;
    d->topLeft = topLeft;
}

const KisPaintDeviceSP KisFilterConstantProcessingInformation::paintDevice() const
{
    return d->device;
}

const KisSelectionSP KisFilterConstantProcessingInformation::selection() const
{
    return d->selection;
}

const QPoint& KisFilterConstantProcessingInformation::topLeft() const
{
    return d->topLeft;
}

struct KisFilterProcessingInformation::Private
{
    KisPaintDeviceSP device;
};

KisFilterProcessingInformation::KisFilterProcessingInformation(KisPaintDeviceSP device, const QPoint& topLeft, const KisSelectionSP selection) : KisFilterConstantProcessingInformation(device, topLeft, selection), d(new Private)
{
    d->device = device;
}

KisPaintDeviceSP KisFilterProcessingInformation::paintDevice()
{
    return d->device;
}
