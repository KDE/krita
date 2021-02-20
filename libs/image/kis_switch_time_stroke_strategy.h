/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    struct SharedToken {
        SharedToken(int initialTime, bool needsRegeneration);
        ~SharedToken();

        bool tryResetDestinationTime(int time, bool needsRegeneration);
        int fetchTime() const;

    private:
        struct Private;
        QScopedPointer<Private> m_d;
    };

    typedef QSharedPointer<SharedToken> SharedTokenSP;
    typedef QWeakPointer<SharedToken> SharedTokenWSP;

public:
    /**
     * Switches current time to \p frameId
     *
     * NOTE: switching time *does* end all the running stroke, because it is
     *       not a background action, but a distinct user action.
     */
    KisSwitchTimeStrokeStrategy(int frameId,
                                bool needsRegeneration,
                                KisImageAnimationInterface *interface,
                                KisPostExecutionUndoAdapter *undoAdapter);
    ~KisSwitchTimeStrokeStrategy() override;

    void initStrokeCallback() override;
    KisStrokeStrategy* createLodClone(int levelOfDetail) override;

    SharedTokenSP token() const;


private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_SWITCH_TIME_STROKE_STRATEGY_H */
