/*
 *  SPDX-FileCopyrightText: 2024 Halla Rempt <halla@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISQSTRINGLISTFWD_H
#define KISQSTRINGLISTFWD_H

#include <QtGlobal>

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
class QStringList;
#else
class QString;
class QByteArray;
template <typename T> class QList;
template<typename T> using QVector = QList<T>;
using QStringList = QList<QString>;
using QByteArrayList = QList<QByteArray>;
#endif

#endif // KISQSTRINGLISTFWD_H
