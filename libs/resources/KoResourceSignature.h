/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KORESOURCESIGNATURE_H
#define KORESOURCESIGNATURE_H

#include "kritaresources_export.h"
#include <QString>
#include <boost/operators.hpp>

/**
 * @brief A simple wrapper object for the main information about the
 * resource.
 *
 * This information is needed to somewhat uniquely identify the resource
 * in the world
 */
class KRITARESOURCES_EXPORT KoResourceSignature : public boost::equality_comparable<KoResourceSignature>
{
public:
    KoResourceSignature();
    KoResourceSignature(const QString &type, const QString &md5, const QString &filename, const QString &name);

    KRITARESOURCES_EXPORT
    friend bool operator==(const KoResourceSignature &lhs, const KoResourceSignature &rhs);

    QString type;
    QString md5;
    QString filename;
    QString name;
};

KRITARESOURCES_EXPORT QDebug operator<<(QDebug dbg, const KoResourceSignature &sig);

#endif // KORESOURCESIGNATURE_H
