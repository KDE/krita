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

#ifndef __KIS_ACS_TYPES_H
#define __KIS_ACS_TYPES_H

#include <QPoint>
#include <KoColor.h>


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
    KoColor pickColor(PaintDeviceSP device, const QPoint &pt) {
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

}



#endif /* __KIS_ACS_TYPES_H */
