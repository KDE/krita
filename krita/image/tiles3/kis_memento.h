/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *            (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KIS_MEMENTO_H_
#define KIS_MEMENTO_H_

#include <qglobal.h>
#include <QRect>

#include "kis_global.h"

#include <kis_shared.h>
#include <kis_shared_ptr.h>

#include "kis_memento_manager.h"

class KisTile;
class KisTiledDataManager;

class KisMemento;
typedef KisSharedPtr<KisMemento> KisMementoSP;


class KisMemento : public KisShared
{
public:
    inline KisMemento(KisMementoManager* mementoManager) {
        m_mementoManager = mementoManager;

        m_valid = true;

        m_extentMinX = qint32_MAX;
        m_extentMinY = qint32_MAX;
        m_extentMaxX = qint32_MIN;
        m_extentMaxY = qint32_MIN;
    }

    inline ~KisMemento() {
    }

    inline void reportEndTransaction() {
        if (m_mementoManager)
            m_mementoManager->commit();
    }

    inline void extent(qint32 &x, qint32 &y, qint32 &w, qint32 &h) {
        x = m_extentMinX;
        y = m_extentMinY;
        w = (m_extentMaxX >= m_extentMinX) ? m_extentMaxX - m_extentMinX + 1 : 0;
        h = (m_extentMaxY >= m_extentMinY) ? m_extentMaxY - m_extentMinY + 1 : 0;
    }

    inline QRect extent() {
        qint32 x, y, w, h;
        extent(x, y, w, h);
        return QRect(x, y, w, h);
    }

    inline bool containsTile(qint32 col, qint32 row, quint32 tileHash) const {
        Q_UNUSED(col);
        Q_UNUSED(row);
        Q_UNUSED(tileHash);
        Q_ASSERT_X(0, "KisMemento::containsTile", "Not implemented");
        return false; // Compiller would be happy! =)
    }

    // For debugging use
    inline bool valid() const {
        return m_valid;
    }
    inline void setInvalid() {
        m_valid = false;
    }

private:
    friend class KisMementoManager;

    inline void updateExtent(qint32 col, qint32 row) {
        const qint32 tileMinX = col * KisTileData::WIDTH;
        const qint32 tileMinY = row * KisTileData::HEIGHT;
        const qint32 tileMaxX = tileMinX + KisTileData::WIDTH - 1;
        const qint32 tileMaxY = tileMinY + KisTileData::HEIGHT - 1;

        m_extentMinX = qMin(m_extentMinX, tileMinX);
        m_extentMaxX = qMax(m_extentMaxX, tileMaxX);
        m_extentMinY = qMin(m_extentMinY, tileMinY);
        m_extentMaxY = qMax(m_extentMaxY, tileMaxY);
    }

private:
    KisMementoManager* m_mementoManager;

    qint32 m_extentMinX;
    qint32 m_extentMaxX;
    qint32 m_extentMinY;
    qint32 m_extentMaxY;

    bool m_valid;
};

#endif // KIS_MEMENTO_H_
