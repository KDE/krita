/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_composite_progress_proxy.h"

#include "kis_debug.h"

void KisCompositeProgressProxy::addProxy(KoProgressProxy *proxy)
{
    m_proxies.append(proxy);
    if (!m_uniqueProxies.contains(proxy)) {
        m_uniqueProxies.append(proxy);
    }
}

void KisCompositeProgressProxy::removeProxy(KoProgressProxy *proxy)
{
    m_proxies.removeOne(proxy);
    if (!m_proxies.contains(proxy)) {
        m_uniqueProxies.removeOne(proxy);
    }
}

int KisCompositeProgressProxy::maximum() const
{
    if(m_proxies.isEmpty()) return 0;

    return m_proxies.first()->maximum();
}

void KisCompositeProgressProxy::setValue(int value)
{
    Q_FOREACH (KoProgressProxy *proxy, m_uniqueProxies) {
        proxy->setValue(value);
    }
}

void KisCompositeProgressProxy::setRange(int minimum, int maximum)
{
    Q_FOREACH (KoProgressProxy *proxy, m_uniqueProxies) {
        proxy->setRange(minimum, maximum);
    }
}

void KisCompositeProgressProxy::setFormat(const QString &format)
{
    Q_FOREACH (KoProgressProxy *proxy, m_uniqueProxies) {
        proxy->setFormat(format);
    }
}

void KisCompositeProgressProxy::setAutoNestedName(const QString &name)
{
    Q_FOREACH (KoProgressProxy *proxy, m_uniqueProxies) {
        proxy->setAutoNestedName(name);
    }
}

