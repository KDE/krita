/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_PAINTOP_LOD_LIMITATIONS_H
#define __KIS_PAINTOP_LOD_LIMITATIONS_H

#include <KoID.h>
#include <QVector>
#include <QMetaType>

inline uint qHash(const KoID &id) {
    return qHash(id.id());
}

class KisPaintopLodLimitations
{
public:
    KisPaintopLodLimitations()
    {
    }

    QSet<KoID> limitations;
    QSet<KoID> blockers;
};

Q_DECLARE_METATYPE(KisPaintopLodLimitations);

#endif /* __KIS_PAINTOP_LOD_LIMITATIONS_H */
