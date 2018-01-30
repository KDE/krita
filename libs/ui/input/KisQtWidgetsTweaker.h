/* This file is part of the KDE project
   Copyright (C) 2017 Nikita Vertikov   <kitmouse.nikita@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef KISQTWIDGETTWEAKER_H
#define KISQTWIDGETTWEAKER_H

#include <QObject>
#include "kritaui_export.h"

class QEvent;

/**
 * KisQtWidgetsTweaker is used to make minor adjustments to
 * "native" qt widgets' behavior application-wise
 * by filtering events addressed to them
 * It expected to be installed on the application
 */
class KRITAUI_EXPORT KisQtWidgetsTweaker : public QObject
{
    Q_OBJECT
public:
    KisQtWidgetsTweaker(QObject* parent = nullptr);
    ~KisQtWidgetsTweaker();
    bool eventFilter(QObject *receiver, QEvent* event);
    static KisQtWidgetsTweaker *instance();

private:
    struct Private;
    Private* d;
};


#endif //KISQTWIDGETTWEAKER_H
