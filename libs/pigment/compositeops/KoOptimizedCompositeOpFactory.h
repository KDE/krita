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

#ifndef KOOPTIMIZEDCOMPOSITEOPFACTORY_H
#define KOOPTIMIZEDCOMPOSITEOPFACTORY_H

#include "pigment_export.h"

class KoCompositeOp;
class KoColorSpace;

/**
 * The creation of the optimized composite ops is moved into a separate
 * objects module for two reasons:
 *
 * 1) They are not templated, that is they do not need inlining into
 *    the user's code.
 * 2) This removes compilation dependencies.
 * 3) (most important!) When the object module is shared with a colorspace
 *    class, which is quite huge itself, GCC layouts the code somehow badly
 *    that causes 60% performance degradation.
 */

class PIGMENTCMS_EXPORT KoOptimizedCompositeOpFactory
{
public:
    static KoCompositeOp* createAlphaDarkenOp32(const KoColorSpace *cs);
    static KoCompositeOp* createOverOp32(const KoColorSpace *cs);
};

#endif /* KOOPTIMIZEDCOMPOSITEOPFACTORY_H */
