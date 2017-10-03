/* This file is part of the KDE project
 * Copyright (C) 2012 KO GmbH. Contact: Boudewijn Rempt <boud@kogmbh.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
