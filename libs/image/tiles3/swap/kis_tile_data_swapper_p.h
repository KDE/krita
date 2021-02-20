/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_TILE_DATA_SWAPPER_P_H_
#define KIS_TILE_DATA_SWAPPER_P_H_

#include "kis_image_config.h"
#include "tiles3/kis_tile_data.h"


/*
       Limits Diagram
  +------------------------+  <-- out of memory
  |                        |
  |                        |
  |                        |
  |## emergencyThreshold ##|  <-- new tiles are not created
  |                        |      until we free some memory
  |                        |
  |== hardLimitThreshold ==|  <-- the swapper thread starts
  |........................|      swapping out working (actually
  |........................|      needed) tiles until the level
  |........................|      reaches hardLimit level.
  |........................|
  |=====  hardLimit  ======|  <-- the swapper stops swapping
  |                        |      out needed tiles
  |                        |
  :                        :
  |                        |
  |                        |
  |== softLimitThreshold ==|  <-- the swapper starts swapping
  |........................|      out memento tiles (those, which
  |........................|      store undo information)
  |........................|
  |=====  softLimit  ======|  <-- the swapper stops swapping
  |                        |      out memento tiles
  |                        |
  :                        :
  |                        |
  +------------------------+  <-- 0 MiB

 */


class KisStoreLimits
{
public:
    KisStoreLimits() {
        KisImageConfig config(true);

        m_emergencyThreshold = MiB_TO_METRIC(config.tilesHardLimit());

        m_hardLimitThreshold = m_emergencyThreshold - (m_emergencyThreshold / 8);
        m_hardLimit = m_hardLimitThreshold - (m_hardLimitThreshold / 8);

        m_softLimitThreshold = qBound(0, MiB_TO_METRIC(config.tilesSoftLimit()), m_hardLimitThreshold);
        m_softLimit = m_softLimitThreshold - m_softLimitThreshold / 8;
    }

    /**
     * These methods return the "metric" of the size
     */

    inline qint32 emergencyThreshold() {
        return m_emergencyThreshold;
    }

    inline qint32 hardLimitThreshold() {
        return m_hardLimitThreshold;
    }

    inline qint32 hardLimit() {
        return m_hardLimit;
    }

    inline qint32 softLimitThreshold() {
        return m_softLimitThreshold;
    }

    inline qint32 softLimit() {
        return m_softLimit;
    }

private:
    qint32 m_emergencyThreshold;
    qint32 m_hardLimitThreshold;
    qint32 m_hardLimit;
    qint32 m_softLimitThreshold;
    qint32 m_softLimit;
};




#endif /* KIS_TILE_DATA_SWAPPER_P_H_ */

