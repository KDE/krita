/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISTOOLCHANGESTRACKER_H
#define KISTOOLCHANGESTRACKER_H

#include "kritaui_export.h"
#include <QScopedPointer>
#include "KisToolChangesTrackerData.h"

class KRITAUI_EXPORT KisToolChangesTracker :public QObject
{
    Q_OBJECT

public:
    KisToolChangesTracker();
    ~KisToolChangesTracker();

    void commitConfig(KisToolChangesTrackerDataSP state);
    void requestUndo();
    KisToolChangesTrackerDataSP lastState() const;
    void reset();

    bool isEmpty() const;

Q_SIGNALS:
    void sigConfigChanged(KisToolChangesTrackerDataSP state);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISTOOLCHANGESTRACKER_H
