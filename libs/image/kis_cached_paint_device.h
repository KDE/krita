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

#ifndef __KIS_CACHED_PAINT_DEVICE_H
#define __KIS_CACHED_PAINT_DEVICE_H

#include "tiles3/kis_lockless_stack.h"
#include "kis_paint_device.h"
#include "kis_selection.h"

class KisCachedPaintDevice
{
public:
    KisPaintDeviceSP getDevice(KisPaintDeviceSP prototype) {
        KisPaintDeviceSP device;

        if(!m_stack.pop(device)) {
            device = new KisPaintDevice(prototype->colorSpace());
        }

        device->prepareClone(prototype);
        return device;
    }

    void putDevice(KisPaintDeviceSP device) {
        device->clear();
        device->setDefaultBounds(new KisDefaultBounds());
        m_stack.push(device);
    }

    struct Guard {
        Guard(KisPaintDeviceSP prototype, KisCachedPaintDevice &parent)
            : m_parent(parent)
        {
            m_device = m_parent.getDevice(prototype);
        }

        ~Guard() {
            m_parent.putDevice(m_device);
        }

        KisPaintDeviceSP device() const {
            return m_device;
        }

        private:
        KisCachedPaintDevice &m_parent;
        KisPaintDeviceSP m_device;
    };

private:
    KisLocklessStack<KisPaintDeviceSP> m_stack;
};

class KisCachedSelection
{
public:
    KisSelectionSP getSelection() {
        KisSelectionSP selection;

        if(!m_stack.pop(selection)) {
            selection = new KisSelection(new KisSelectionEmptyBounds(0));
        }

        return selection;
    }

    void putSelection(KisSelectionSP selection) {
        selection->clear();
        selection->setDefaultBounds(new KisSelectionEmptyBounds(0));
        selection->pixelSelection()->moveTo(QPoint());
        m_stack.push(selection);
    }

    struct Guard {
        Guard(KisCachedSelection &parent)
            : m_parent(parent)
        {
            m_selection = m_parent.getSelection();
        }

        ~Guard() {
            m_parent.putSelection(m_selection);
        }

        KisSelectionSP selection() const {
            return m_selection;
        }

        private:
        KisCachedSelection &m_parent;
        KisSelectionSP m_selection;
    };

private:
    KisLocklessStack<KisSelectionSP> m_stack;
};

#endif /* __KIS_CACHED_PAINT_DEVICE_H */
