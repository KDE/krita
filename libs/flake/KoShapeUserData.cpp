/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoShapeUserData.h"

KoShapeUserData::KoShapeUserData(QObject *parent)
    : QObject(parent)
{
}

KoShapeUserData::~KoShapeUserData()
{
}

KoShapeUserData::KoShapeUserData(const KoShapeUserData &rhs)
    : QObject()
{
    Q_UNUSED(rhs);
}
