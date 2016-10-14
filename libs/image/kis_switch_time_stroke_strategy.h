/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_SWITCH_TIME_STROKE_STRATEGY_H
#define __KIS_SWITCH_TIME_STROKE_STRATEGY_H

#include <kis_simple_stroke_strategy.h>

#include <QScopedPointer>

class KisImageAnimationInterface;
class KisPostExecutionUndoAdapter;


class KisSwitchTimeStrokeStrategy : public KisSimpleStrokeStrategy
{
public:
    /**
     * Switches current time to \p frameId
     *
     * NOTE: in contrast to the other c-tor, refreshing current frame
     *       *does* end all the running stroke, because it is not a
     *       background action, but a distinct user action.
     */
    KisSwitchTimeStrokeStrategy(int frameId,
                                KisImageAnimationInterface *interface,
                                KisPostExecutionUndoAdapter *undoAdapter);
    ~KisSwitchTimeStrokeStrategy();

    void initStrokeCallback();
    KisStrokeStrategy* createLodClone(int levelOfDetail);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_SWITCH_TIME_STROKE_STRATEGY_H */
