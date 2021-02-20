/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoProgressProxy.h"

#include <QString>

void KoProgressProxy::setAutoNestedName(const QString &name)
{
    if (name.isEmpty()) {
        setFormat("%p%");
    } else {
        if (maximum() > 0) {
            setFormat(QString("%1: %p%").arg(name));
        } else {
            setFormat(name);
        }
    }
}
