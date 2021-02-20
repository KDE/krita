/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ACS_TYPES_H
#define __KIS_ACS_TYPES_H

#include <QPoint>
#include <KoColor.h>
#include "kis_iterator_ng.h"


namespace Acs {
    enum ColorRole {Foreground, Background};

    inline ColorRole buttonToRole(Qt::MouseButton button) {
        return button == Qt::LeftButton ? Acs::Foreground : Acs::Background;
    }

    inline ColorRole buttonsToRole(Qt::MouseButton button, Qt::MouseButtons buttons) {
        return button == Qt::LeftButton || buttons & Qt::LeftButton ? Acs::Foreground : Acs::Background;
    }

    template <class ResourceProvider>
    void setCurrentColor(ResourceProvider *provider, ColorRole role, const KoColor &color) {
        if (role == Acs::Foreground) {
            provider->setFGColor(color);
        } else {
            provider->setBGColor(color);
        }
    }

    template <class ResourceProvider>
    KoColor currentColor(ResourceProvider *provider, ColorRole role) {
        return role == Acs::Foreground ? provider->fgColor() : provider->bgColor();
    }

    template <class PaintDeviceSP>
    KoColor sampleColor(PaintDeviceSP device, const QPoint &pt) {
        KoColor color;
        if (device) {
            (void) device->pixel(pt.x(), pt.y(), &color);
        }
        return color;
    }

    template <class PaintDeviceSP>
    void setColor(PaintDeviceSP device, const QPoint &pt, const KoColor &color) {
        (void) device->setPixel(pt.x(), pt.y(), color);
    }

    template<class Iterator>
    void setColorWithIterator(Iterator &it, const KoColor &color, const int pixelSize) {
        memcpy(it.rawData(), color.data(), pixelSize);
    }

}



#endif /* __KIS_ACS_TYPES_H */
