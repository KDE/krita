/* This file is part of the KDE project
 * Copyright 2011 (C) Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef _KIS_KRA_UTILS_
#define _KIS_KRA_UTILS_

#include <QString>
#include <QBitArray>

namespace KRA {

QString   flagsToString(const QBitArray& flags, int size=-1, char trueToken='1', char falseToken='0', bool defaultTrue=true);
QBitArray stringToFlags(const QString& string, int size=-1, char token='0', bool defaultTrue=true);

}

#endif // _KIS_KRA_UTILS_
