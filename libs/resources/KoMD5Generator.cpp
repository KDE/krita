/*
 * Copyright (c) 2015 Stefano Bonicatti <smjert@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#include "KoMD5Generator.h"

#include <QIODevice>
#include <QFile>
#include <QCryptographicHash>

KoMD5Generator::KoMD5Generator()
{

}

KoMD5Generator::~KoMD5Generator()
{

}

QByteArray KoMD5Generator::generateHash(const QByteArray &array)
{
    if (!array.isEmpty()) {
        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData(array);
        return md5.result();
    }

    return array;
}

QByteArray KoMD5Generator::generateHash(const QString &filename)
{
    QByteArray result;

    QFile f(filename);
    if (f.exists() && f.open(QIODevice::ReadOnly)) {
        QByteArray ba = f.readAll();
        result = generateHash(ba);
    }

    return result;
}
