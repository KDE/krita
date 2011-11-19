/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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
#include "kis_selection_default_bounds.h"


struct KisSelectionDefaultBounds::Private
{
    KisPaintDeviceSP parentDevice;
};

KisSelectionDefaultBounds::KisSelectionDefaultBounds(KisPaintDeviceSP parentDevice, KisImageWSP image)
    : KisDefaultBounds(image),
      m_d(new Private())
{
    m_d->parentDevice = parentDevice;
}

KisSelectionDefaultBounds::~KisSelectionDefaultBounds()
{
    delete m_d;
}

QRect KisSelectionDefaultBounds::bounds() const
{
    QRect additionalRect = m_d->parentDevice ? m_d->parentDevice->exactBounds() : QRect();
    return additionalRect | KisDefaultBounds::bounds();
}
