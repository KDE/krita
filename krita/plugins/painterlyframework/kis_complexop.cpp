/*
 *  Copyright (c) 2007 Emanuele Tamponi (emanuele@valinor.it)
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

#include "kis_complexop.h"

#include <KLocale>
#include <kis_image.h>
#include <kis_painter.h>
#include <kis_paint_information.h>
#include <kis_paintop_settings.h>
#include <QString>

KisComplexOp::KisComplexOp(KisPainter *painter)
    : super(painter)
{
}

KisComplexOp::~KisComplexOp()
{

}

void KisComplexOp::paintAt(const KisPaintInformation &info)
{
    Q_UNUSED(info)
}


KisPaintOp *KisComplexOpFactory::createOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisImageSP image)
{
    Q_UNUSED(settings)
    Q_UNUSED(image)

    KisPaintOp *op = new KisComplexOp(painter);
    Q_CHECK_PTR(op);

    return op;
}

QString KisComplexOpFactory::id() const
{
    return "paintcomplex";
}

QString KisComplexOpFactory::name() const
{
    return i18n("Complex brush");
}
