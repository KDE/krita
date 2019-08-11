/* 
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_BASE_ITERATOR_H_
#define _KIS_BASE_ITERATOR_H_

#include "kis_datamanager.h"
#include "kis_tiled_data_manager.h"
#include "kis_tile.h"
#include "kis_types.h"
#include "kis_shared.h"
#include "kis_iterator_complete_listener.h"

class KisBaseIterator {
protected:
    KisBaseIterator(KisTiledDataManager * _dataManager, bool _writable, KisIteratorCompleteListener *listener) {
        m_dataManager = _dataManager;
        m_pixelSize = m_dataManager->pixelSize();
        m_writable = _writable;
        m_completeListener = listener;
    }
    ~KisBaseIterator() {
        if (m_writable && m_completeListener) {
            m_completeListener->notifyWritableIteratorCompleted();
        }
    }

    KisTiledDataManager *m_dataManager;
    qint32 m_pixelSize;        // bytes per pixel
    bool m_writable;
    inline void lockTile(KisTileSP &tile) {
        if (m_writable)
            tile->lockForWrite();
        else
            tile->lockForRead();
    }
    inline void lockOldTile(KisTileSP &tile) {
        // Doesn't depend on current access type
        tile->lockForRead();
    }
    inline void unlockTile(KisTileSP &tile) {
        if (m_writable) {
            tile->unlockForWrite();
        } else {
            tile->unlockForRead();
        }
    }

    inline void unlockOldTile(KisTileSP &tile) {
        tile->unlockForRead();
    }

    inline quint32 xToCol(quint32 x) const {
        return m_dataManager ? m_dataManager->xToCol(x) : 0;
    }
    inline quint32 yToRow(quint32 y) const {
        return m_dataManager ? m_dataManager->yToRow(y) : 0;
    }

    inline qint32 calcXInTile(qint32 x, qint32 col) const {
        return x - col * KisTileData::WIDTH;
    }

    inline qint32 calcYInTile(qint32 y, qint32 row) const {
        return y - row * KisTileData::HEIGHT;
    }
    
private:
    KisIteratorCompleteListener *m_completeListener;
};

#endif
