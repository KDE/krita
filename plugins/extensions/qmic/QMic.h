/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef QMIC_H
#define QMIC_H

#include <KisActionPlugin.h>
#include <QPointer>
#include <QVariant>
#include <QVector>
#include <kis_types.h>

#include "gmic.h"
#include "kis_qmic_interface.h"
#include "kis_qmic_plugin_interface.h"

class KisAction;
class QLocalServer;
class QSharedMemory;

class KisQmicApplicator;

class QMic : public KisActionPlugin
{
    Q_OBJECT
public:
    QMic(QObject *parent, const QVariantList &);
    ~QMic() override = default;

private Q_SLOTS:

    void slotQMicAgain();
    void slotQMic(bool again = false);

private:
    KisQmicPluginInterface *plugin {nullptr};
    QString m_key;
    KisAction *m_qmicAction {0};
    KisAction *m_againAction {0};
};

#endif // QMic_H
