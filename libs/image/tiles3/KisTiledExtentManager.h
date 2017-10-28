/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISTILEDEXTENTMANAGER_H
#define KISTILEDEXTENTMANAGER_H

#include <QMutex>
#include <QMap>
#include <QRect>
#include "kritaimage_export.h"


class KRITAIMAGE_EXPORT KisTiledExtentManager
{
public:
    KisTiledExtentManager();

    void notifyTileAdded(int col, int row);
    void notifyTileRemoved(int col, int row);
    void replaceTileStats(const QVector<QPoint> &indexes);

    void clear();

    QRect extent() const;

private:
    void updateExtent();

private:
    mutable QMutex m_mutex;
    QMap<int, int> m_colMap;
    QMap<int, int> m_rowMap;
    QRect m_currentExtent;
};

#endif // KISTILEDEXTENTMANAGER_H
