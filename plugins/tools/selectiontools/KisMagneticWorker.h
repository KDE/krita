/*
 *  Copyright (c) 2019 Kuntal Majumder <hellozee@disroot.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISMAGNETICWORKER_H
#define KISMAGNETICWORKER_H

#include <kis_paint_device.h>
#include <kritaselectiontools_export.h>

class KRITASELECTIONTOOLS_EXPORT KisMagneticWorker{
    public:
        KisMagneticWorker();
        KisMagneticWorker(KisPaintDeviceSP dev, const QRect &rect);
        void run(KisPaintDeviceSP dev, const QRect& rect);
        void computeEdge(QPoint start, QPoint end);

    private:
        KisPaintDeviceSP m_dev;
        const QRect m_rect;
};

#endif
