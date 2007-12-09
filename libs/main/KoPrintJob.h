/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#ifndef KOPRINTJOB_H
#define KOPRINTJOB_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtGui/QPrinter>

#include "komain_export.h"

class QWidget;

/// interface class for printing
class KOMAIN_EXPORT KoPrintJob : public QObject {
    Q_OBJECT
public:
    KoPrintJob(QWidget *parent = 0);
    virtual ~KoPrintJob();

    enum RemovePolicy {
        DeleteWhenDone,
        DoNotDelete
    };

    virtual QPrinter &printer() = 0;
    virtual QList<QWidget*> createOptionWidgets() const = 0;

public slots:
    virtual void startPrinting(RemovePolicy removePolicy = DoNotDelete);
};

#endif
