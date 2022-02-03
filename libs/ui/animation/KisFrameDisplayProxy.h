/*
 *  SPDX-FileCopyrightText: 2022 Eoin O'Neill <eoinoneill1991@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISFRAMEDISPLAYPROXY_H
#define KISFRAMEDISPLAYPROXY_H

#include <QObject>
#include <QPointer>

class KisFrameDisplayProxy : public QObject
{
    Q_OBJECT
public:
    KisFrameDisplayProxy(class KisCanvas2 *canvas, QObject *parent = nullptr);
    ~KisFrameDisplayProxy();

    bool displayFrame(int frame);
    int visibleFrame() const;

Q_SIGNALS:
    void sigDisplayFrameChanged();

private:
    QScopedPointer<struct Private> m_d;
};

#endif // KISFRAMEDISPLAYPROXY_H
