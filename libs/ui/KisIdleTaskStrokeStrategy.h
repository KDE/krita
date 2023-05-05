/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISIDLETASKSTROKESTRATEGY_H
#define KISIDLETASKSTROKESTRATEGY_H

#include <kritaui_export.h>

#include <functional>

#include <kis_types.h>
#include <KisRunnableBasedStrokeStrategy.h>


class KRITAUI_EXPORT KisIdleTaskStrokeStrategy: public QObject, public KisRunnableBasedStrokeStrategy {
    Q_OBJECT
public:
    KisIdleTaskStrokeStrategy(const QLatin1String &id, const KUndo2MagicString &name = KUndo2MagicString());
    ~KisIdleTaskStrokeStrategy();

    KisStrokeStrategy* createLodClone(int levelOfDetail) override;
    QWeakPointer<bool> idleTaskCookie();

protected:
    void finishStrokeCallback() override;

Q_SIGNALS:
    void sigIdleTaskFinished();

private:
    QSharedPointer<bool> m_idleTaskCookie;
};

using KisIdleTaskStrokeStrategyFactory = std::function<KisIdleTaskStrokeStrategy*(KisImageSP image)>;

#endif // KISIDLETASKSTROKESTRATEGY_H
