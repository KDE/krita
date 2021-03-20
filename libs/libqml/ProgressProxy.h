/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 KO GmbH. Contact : Boudewijn Rempt <boud@kogmbh.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KRITA_SKETCH_PROGRESSPROXY_H
#define KRITA_SKETCH_PROGRESSPROXY_H

#include <QObject>
#include <KoProgressProxy.h>

#include "krita_sketch_export.h"

class KRITA_SKETCH_EXPORT ProgressProxy : public QObject, public KoProgressProxy
{
    Q_OBJECT
    Q_PROPERTY(QString taskName READ taskName NOTIFY taskNameChanged)

public:
    ProgressProxy(QObject* parent = 0);
    virtual ~ProgressProxy();

    QString taskName() const;

    virtual void setFormat(const QString& format);
    virtual void setRange(int minimum, int maximum);
    virtual void setValue(int value);
    virtual int maximum() const;

Q_SIGNALS:
    void valueChanged(int value);
    void taskStarted();
    void taskEnded();
    void taskNameChanged();

private:
    class Private;
    const QScopedPointer<Private> d;
};

#endif // CMPROGRESSPROXY_H
