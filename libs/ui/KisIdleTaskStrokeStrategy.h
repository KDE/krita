/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISIDLETASKSTROKESTRATEGY_H
#define KISIDLETASKSTROKESTRATEGY_H

#include <kritaui_export.h>

#include <functional>

#include <boost/none.hpp>
#include <kis_types.h>
#include <KisRunnableBasedStrokeStrategy.h>

/**
 * A base class for strategies used in "idle tasks". Such strategy
 * does **not** modify any undo stack and can be cancelled by the
 * image at any moment (e.g. when the user starts a real brush stroke).
 *
 * If you need to handle the cancellation event, implement
 * cancelStrokeCallback() function.
 */
class KRITAUI_EXPORT KisIdleTaskStrokeStrategy: public QObject, public KisRunnableBasedStrokeStrategy {
    Q_OBJECT
public:
    KisIdleTaskStrokeStrategy(const QLatin1String &id, const KUndo2MagicString &name = KUndo2MagicString());
    ~KisIdleTaskStrokeStrategy();

    KisStrokeStrategy* createLodClone(int levelOfDetail) override;
    QWeakPointer<boost::none_t> idleTaskCookie();

protected:
    void finishStrokeCallback() override;

Q_SIGNALS:
    void sigIdleTaskFinished();

private:
    QSharedPointer<boost::none_t> m_idleTaskCookie;
};

using KisIdleTaskStrokeStrategyFactory = std::function<KisIdleTaskStrokeStrategy*(KisImageSP image)>;

#endif // KISIDLETASKSTROKESTRATEGY_H
