/*
 * This file is part of Krita
 *
 * Copyright (c) 2016 Spencer Brown <sbrown655@gmail.com>
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

#pragma once

#include "KoColorTransformation.h"
#include "KoAbstractGradient.h"

class KritaGradientMapColorTransformation : public KoColorTransformation
{
public:
    KritaGradientMapColorTransformation(const KoAbstractGradient * gradient, const KoColorSpace* cs);
    virtual void transform(const quint8* src, quint8* dst, qint32 nPixels) const;
private:
    const KoAbstractGradient* m_gradient;
    const KoColorSpace* m_colorSpace;
    const size_t m_psize;
};

