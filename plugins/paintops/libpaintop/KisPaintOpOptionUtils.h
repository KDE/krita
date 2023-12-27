/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISPAINTOPOPTIONUTILS_H
#define KISPAINTOPOPTIONUTILS_H

class KisPropertiesConfiguration;

namespace KisPaintOpOptionUtils
{
template <typename T>
T loadOptionData(const KisPropertiesConfiguration *setting)
{
    T data;
    data.read(setting);
    return data;
}
}

#endif // KISPAINTOPOPTIONUTILS_H
