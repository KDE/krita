/*
 * Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef QMIC_H
#define QMIC_H

#include <QProcess>
#include <QVariant>
#include <QVector>
#include <KisActionPlugin.h>
#include <kis_types.h>

#include "gmic.h"

class KisAction;
class QLocalServer;
class QSharedMemory;

class KisQmicApplicator;

class QMic : public KisActionPlugin
{
    Q_OBJECT
public:
    QMic(QObject *parent, const QVariantList &);
    virtual ~QMic();

private Q_SLOTS:

    void slotQMicAgain();
    void slotQMic(bool again = false);
    void connected();
    void pluginStateChanged(QProcess::ProcessState);
    void pluginFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void slotGmicFinished(bool successfully, int milliseconds = -1, const QString& msg = QString());
    void slotStartApplicator(QStringList gmicImages);

private:

    bool prepareCroppedImages(QByteArray *message, QRectF &rc, int inputMode);

    QProcess *m_pluginProcess {0};
    QLocalServer *m_localServer {0};
    QString m_key;
    KisAction *m_qmicAction {0};
    KisAction *m_againAction {0};
    QVector<QSharedMemory *> m_sharedMemorySegments;
    KisQmicApplicator *m_gmicApplicator {0};
    InputLayerMode m_inputMode {ACTIVE_LAYER};
    OutputMode m_outputMode {IN_PLACE};

};

#endif // QMic_H
