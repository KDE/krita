/*
 * SPDX-FileCopyrightText: 2015 Stefano Bonicatti <smjert@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/
#include "KoMD5Generator.h"

#include <QIODevice>
#include <QFile>
#include <QCryptographicHash>

QString KoMD5Generator::generateHash(const QByteArray &array)
{
    QString result;

    if (!array.isEmpty()) {
        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData(array);
        result = md5.result().toHex();
    }

    return result;
}

QString KoMD5Generator::generateHash(const QString &filename)
{
    QString result;

    QFile f(filename);
    if (f.exists() && f.open(QIODevice::ReadOnly)) {
        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData(&f);
        result = md5.result().toHex();
    }

    return result;
}
