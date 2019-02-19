/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISGLIMAGEF16_H
#define KISGLIMAGEF16_H

#include <QSharedDataPointer>
#include <half.h>
#include <boost/operators.hpp>

class QSize;

class KisGLImageF16 : public boost::equality_comparable<KisGLImageF16>
{
public:
    KisGLImageF16();
    KisGLImageF16(const QSize &size, bool clearPixels = false);
    KisGLImageF16(int width, int height, bool clearPixels = false);
    KisGLImageF16(const KisGLImageF16 &rhs);
    KisGLImageF16& operator=(const KisGLImageF16 &rhs);

    friend bool operator==(const KisGLImageF16 &lhs, const KisGLImageF16 &rhs);

    ~KisGLImageF16();

    void clearPixels();
    void resize(const QSize &size, bool clearPixels = false);

    const half* constData() const;
    half* data();

    QSize size() const;
    int width() const;
    int height() const;

    bool isNull() const;

private:
    struct Private;
    QSharedDataPointer<Private> m_d;
};

#endif // KISGLIMAGEF16_H
