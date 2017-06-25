/*
 * Copyright (c) 2014 Lukáš Tvrdý <lukast.dev@gmail.com>
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
