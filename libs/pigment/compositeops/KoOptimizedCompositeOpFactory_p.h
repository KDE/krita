/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KOOPTIMIZEDCOMPOSITEOPFACTORY_P_H
#define KOOPTIMIZEDCOMPOSITEOPFACTORY_P_H

#include "pigment_export.h"

class KoCompositeOp;
class KoColorSpace;

/**
 * The creation of the legacy composite ops is moved to a separate
 * object file. Putting all the implementations together makes the
 * system run 1.4 times slower. I do not know the reason of it,
 * looks like some layout/code locality problem (DK)
 */

class PIGMENTCMS_EXPORT KoOptimizedCompositeOpFactoryPrivate
{
public:
    static KoCompositeOp* createLegacyAlphaDarkenOp32(const KoColorSpace *cs);
    static KoCompositeOp* createLegacyOverOp32(const KoColorSpace *cs);
};

#endif /* KOOPTIMIZEDCOMPOSITEOPFACTORY_P_H */
