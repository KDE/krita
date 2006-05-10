/*
 * This file is part of the KDE project
 *  Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>
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

#ifndef KIS_IMAGE_IFACE_H
#define KIS_IMAGE_IFACE_H

#include <dcopref.h>
#include <dcopobject.h>


#include <QString>

class KisImage;
class KisPaintDeviceIface;

class KisImageIface : virtual public DCOPObject
{
    K_DCOP
public:
    KisImageIface( KisImage *img_ );
k_dcop:

    int height() const;
    int width() const;

    void setName(const QString& name);

    void rotateCCW();
    void rotateCW();
    void rotate180();
    void rotate(double angle);

    /**
     * Get the active painting device.
     */
    DCOPRef activeDevice();

    /**
     * Get the colorspace of this image
     */
    DCOPRef colorSpace() const;


private:

    KisImage *m_img;
};

#endif
