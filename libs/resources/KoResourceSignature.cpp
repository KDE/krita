/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoResourceSignature.h"

#include <QDebug>

KoResourceSignature::KoResourceSignature()
{
}

KoResourceSignature::KoResourceSignature(const QString &_type, const QString &_md5sum, const QString &_filename, const QString &_name)
    : type(_type),
      md5sum(_md5sum),
      filename(_filename),
      name(_name)
{
}

bool operator==(const KoResourceSignature &lhs, const KoResourceSignature &rhs)
{
    return lhs.md5sum == rhs.md5sum && lhs.filename == rhs.filename && lhs.name == rhs.name;
}

QDebug operator<<(QDebug dbg, const KoResourceSignature &sig)
{
    dbg.nospace() << "KoResourceSignature("
        << sig.type << ", "
        << sig.md5sum << ", "
        << sig.filename << ", "
        << sig.name << ")";

    return dbg.space();
}
