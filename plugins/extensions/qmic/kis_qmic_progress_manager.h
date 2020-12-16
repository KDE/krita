/*
 * SPDX-FileCopyrightText: 2014 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KIS_GMIC_PROGRESS_MANAGER
#define KIS_GMIC_PROGRESS_MANAGER

#include <QObject>
#include <QTimer>

#include <KoUpdater.h>
#include <kis_types.h>

class KisViewManager;
class QTimer;

class KisQmicProgressManager : public QObject
{
    Q_OBJECT
public:
    KisQmicProgressManager(KisViewManager *viewManager);
    virtual ~KisQmicProgressManager();

    void initProgress();
    void updateProgress(float progress);
    void finishProgress();
    bool inProgress();

Q_SIGNALS:
    void sigProgress();

private:
    QTimer m_progressTimer;
    KoProgressUpdater *m_progressUpdater;
    KoUpdaterPtr m_updater;
    quint32 m_progressPulseRequest;
};

#endif
