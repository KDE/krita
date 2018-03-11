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

#ifndef KISSEQUENTIALITERATORPROGRESS_H
#define KISSEQUENTIALITERATORPROGRESS_H

#include "kis_sequential_iterator.h"
#include <KoProgressProxy.h>
#include <KoFakeProgressProxy.h>

struct ProxyBasedProgressPolicy
{
    ProxyBasedProgressPolicy(KoProgressProxy *proxy)
        : m_proxy(proxy ? proxy : KoFakeProgressProxy::instance())
    {
    }

    void setRange(int minimum, int maximum)
    {
        m_proxy->setRange(minimum, maximum);
    }

    void setValue(int value)
    {
        m_proxy->setValue(value);
    }

    void setFinished()
    {
        m_proxy->setValue(m_proxy->maximum());
    }

private:
    KoProgressProxy *m_proxy;
};

typedef KisSequentialIteratorBase<ReadOnlyIteratorPolicy<>, DevicePolicy, ProxyBasedProgressPolicy> KisSequentialConstIteratorProgress;
typedef KisSequentialIteratorBase<WritableIteratorPolicy<>, DevicePolicy, ProxyBasedProgressPolicy> KisSequentialIteratorProgress;


#endif // KISSEQUENTIALITERATORPROGRESS_H
