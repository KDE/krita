/*
 * Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef QMIC_H
#define QMIC_H

#include <QProcess>
#include <QVariant>

#include <kis_view_plugin.h>

class KisAction;
class QLocalServer;

class QMic : public KisViewPlugin
{
    Q_OBJECT
public:
    QMic(QObject *parent, const QVariantList &);
    virtual ~QMic();

private Q_SLOTS:
    void slotQMicAgain();
    void slotQMic(bool again = false);
    void connected();
    void pluginFinished(int exitCode, QProcess::ExitStatus exitStatus);
private:
    QProcess *m_pluginProcess {0};
    QLocalServer *m_localServer {0};
    QString m_key;
    KisAction *m_qmicAction {0};
    KisAction *m_againAction {0};
};

#endif // QMic_H
