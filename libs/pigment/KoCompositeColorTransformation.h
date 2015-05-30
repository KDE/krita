/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KO_COMPOSITE_COLOR_TRANSFORMATION_H
#define __KO_COMPOSITE_COLOR_TRANSFORMATION_H

#include "KoColorTransformation.h"

#include <QScopedPointer>


class PIGMENTCMS_EXPORT KoCompositeColorTransformation : public KoColorTransformation
{
public:
    enum Mode {
        INPLACE = 0, /// transform pixels in place (in 'dst' buffer)
        BUFFERED /// transform using a temporary buffer (not implemented yet)
    };

public:
    KoCompositeColorTransformation(Mode mode);
    ~KoCompositeColorTransformation();

    void appendTransform(KoColorTransformation *transform);

    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KO_COMPOSITE_COLOR_TRANSFORMATION_H */
