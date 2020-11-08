/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef QMIC_H
#define QMIC_H

#include <KisActionPlugin.h>
#include <QProcess>
#include <QVariant>
#include <QVector>
#include <kis_types.h>

#include "KritaGmicPluginInterface.h"
#include "gmic.h"
#include "kis_image_interface.h"

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

    QSize getSelection();
    QByteArray prepareImages(int mode, QRectF cropRect);
    void outputImages(int mode, QStringList layers);
    void detach();

private Q_SLOTS:

    void slotQMicAgain();
    void slotQMic(bool again = false);

private:
    KritaGmicPluginInterface *plugin {nullptr};
    QString m_key;
    KisAction *m_qmicAction {0};
    KisAction *m_againAction {0};
};

#endif // QMic_H
