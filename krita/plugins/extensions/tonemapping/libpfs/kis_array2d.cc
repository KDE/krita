/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "kis_array2d.h"
#include <QRect>

#include "kis_generic_colorspace.h"
#include "kis_paint_device.h"
#include "kis_random_accessor.h"

using namespace pfs;

struct Array2DImpl::Private {
    int sx, sy, cols, rows, index;
    KisPaintDeviceSP device;
    KisRandomAccessor* randomAccessor;
    KisGenericColorSpace<float, 1>  * colorSpace;
};

Array2DImpl::Array2DImpl(int cols, int rows) : d(new Private)
{
    d->colorSpace = new KisGenericColorSpace<float, 1>();
    init(0, 0, cols, rows, 0, new KisPaintDevice(d->colorSpace));
}

Array2DImpl::Array2DImpl(QRect r, int index, KisPaintDeviceSP device) : d(new Private)
{
    d->colorSpace = 0;
    init(r.x(), r.y(), r.width(), r.height(), index, device);
}

void Array2DImpl::init(int sx, int sy, int cols, int rows, int index, KisPaintDeviceSP device)
{
    Q_ASSERT(device);
    d->sx = sx;
    d->sy = sy;
    d->cols = cols;
    d->rows = rows;
    d->index = index;
    d->device = device;
    d->randomAccessor = new KisRandomAccessor(d->device->createRandomAccessor(0, 0));
}

Array2DImpl::~Array2DImpl()
{
    delete d->randomAccessor;
    delete d->colorSpace;
    delete d;
}

int Array2DImpl::getCols() const
{
    return d->cols;
}

int Array2DImpl::getRows() const
{
    return d->rows;
}

int Array2DImpl::colToDevice(int col) const
{
    Q_ASSERT(col >= 0 && col < d->cols);
    return col + d->sx;
}

int Array2DImpl::rowToDevice(int row) const
{
    Q_ASSERT(row >= 0 && row < d->rows);
    return row + d->sy;
}

float& Array2DImpl::operator()(int col, int row)
{
    d->randomAccessor->moveTo(colToDevice(col), rowToDevice(row));
    return *(reinterpret_cast<float*>(d->randomAccessor->rawData()) + d->index);
}

const float& Array2DImpl::operator()(int col, int row) const
{
    d->randomAccessor->moveTo(colToDevice(col), rowToDevice(row));
    return *(reinterpret_cast<const float*>(d->randomAccessor->oldRawData()) + d->index);
}

float& Array2DImpl::operator()(int index)
{
    int col = index % d->cols;
    int row = index / d->rows;
    d->randomAccessor->moveTo(colToDevice(col), rowToDevice(row));
    return *(reinterpret_cast<float*>(d->randomAccessor->rawData()) + d->index);
}

const float& Array2DImpl::operator()(int index) const
{
    int col = index % d->cols;
    int row = index / d->rows;
    d->randomAccessor->moveTo(colToDevice(col), rowToDevice(row));
    return *(reinterpret_cast<const float*>(d->randomAccessor->oldRawData()) + d->index);
}

KisPaintDeviceSP Array2DImpl::device()
{
    return d->device;
}
