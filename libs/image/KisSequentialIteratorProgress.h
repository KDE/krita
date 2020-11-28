/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
