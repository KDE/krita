/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_composite_progress_proxy.h"

void KisCompositeProgressProxy::addProxy(KoProgressProxy *proxy)
{
    m_proxies.append(proxy);
}

void KisCompositeProgressProxy::removeProxy(KoProgressProxy *proxy)
{
    m_proxies.removeAll(proxy);
}


int KisCompositeProgressProxy::maximum() const
{
    if(m_proxies.isEmpty()) return 0;

    return m_proxies.first()->maximum();
}

void KisCompositeProgressProxy::setValue(int value)
{
    foreach(KoProgressProxy *proxy, m_proxies) {
        proxy->setValue(value);
    }
}

void KisCompositeProgressProxy::setRange(int minimum, int maximum)
{
    foreach(KoProgressProxy *proxy, m_proxies) {
        proxy->setRange(minimum, maximum);
    }
}

void KisCompositeProgressProxy::setFormat(const QString &format)
{
    foreach(KoProgressProxy *proxy, m_proxies) {
        proxy->setFormat(format);
    }
}

