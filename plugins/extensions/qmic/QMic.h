/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef QMIC_H
#define QMIC_H

#include <KisActionPlugin.h>

class KisAction;

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
    KisAction *m_qmicAction;
    KisAction *m_againAction;
};

#endif // QMic_H
