/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_CACHED_PAINT_DEVICE_H
#define __KIS_CACHED_PAINT_DEVICE_H

#include "kis_lockless_stack.h"
#include "kis_paint_device.h"
#include "kis_selection.h"
#include "KoColorSpace.h"
#include "KoColor.h"

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

    KisPaintDeviceSP getDevice(KisPaintDeviceSP prototype, const KoColorSpace *colorSpace) {
        KisPaintDeviceSP device;

        if(!m_stack.pop(device)) {
            device = new KisPaintDevice(colorSpace);
        } else {
            device->convertTo(colorSpace);
        }

        device->setDefaultPixel(KoColor(colorSpace));
        device->setDefaultBounds(prototype->defaultBounds());
        device->setX(prototype->x());
        device->setY(prototype->y());

        return device;
    }

    void putDevice(KisPaintDeviceSP device) {
        device->clear();
        device->setDefaultBounds(new KisDefaultBounds());
        m_stack.push(device);
    }

    bool isEmpty() const {
        return m_stack.isEmpty();
    }

    struct Guard {
        Guard(KisPaintDeviceSP prototype, KisCachedPaintDevice &parent)
            : m_parent(parent)
        {
            m_device = m_parent.getDevice(prototype);
        }

        Guard(KisPaintDeviceSP prototype, const KoColorSpace *cs, KisCachedPaintDevice &parent)
            : m_parent(parent)
        {
            m_device = m_parent.getDevice(prototype, cs);
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

    bool isEmpty() const {
        return m_stack.isEmpty();
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
