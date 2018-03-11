/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KoFakeProgressProxy.h"

#include <QtGlobal>
#include <QGlobalStatic>

Q_GLOBAL_STATIC(KoFakeProgressProxy, s_instance)


int KoFakeProgressProxy::maximum() const
{
    return 100;
}

void KoFakeProgressProxy::setValue(int value)
{
    Q_UNUSED(value);
}

void KoFakeProgressProxy::setRange(int minimum, int maximum)
{
    Q_UNUSED(minimum);
    Q_UNUSED(maximum);
}

void KoFakeProgressProxy::setFormat(const QString &format)
{
    Q_UNUSED(format);
}

void KoFakeProgressProxy::setAutoNestedName(const QString &name)
{
    Q_UNUSED(name);
}

KoProgressProxy *KoFakeProgressProxy::instance()
{
    return s_instance;
}
