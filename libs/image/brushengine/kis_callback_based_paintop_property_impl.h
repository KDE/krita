/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_CALLBACK_BASED_PAINTOP_PROPERTY_IMPL_H
#define __KIS_CALLBACK_BASED_PAINTOP_PROPERTY_IMPL_H

#include <functional>

template <class ParentClass>
KisCallbackBasedPaintopProperty<ParentClass>::KisCallbackBasedPaintopProperty(typename ParentClass::Type type,
                                                                              const QString &id,
                                                                              const QString &name,
                                                                              KisPaintOpSettingsRestrictedSP settings,
                                                                              QObject *parent)
    : ParentClass(type, id, name, settings, parent)
{
}

template <class ParentClass>
KisCallbackBasedPaintopProperty<ParentClass>::KisCallbackBasedPaintopProperty(const QString &id,
                                                                              const QString &name,
                                                                              KisPaintOpSettingsRestrictedSP settings,
                                                                              QObject *parent)
    : ParentClass(id, name, settings, parent)
{
}

template <class ParentClass>
void KisCallbackBasedPaintopProperty<ParentClass>::setReadCallback(Callback func)
{
    m_readFunc = func;
}

template <class ParentClass>
void KisCallbackBasedPaintopProperty<ParentClass>::setWriteCallback(Callback func)
{
    m_writeFunc = func;
}

template <class ParentClass>
void KisCallbackBasedPaintopProperty<ParentClass>::setIsVisibleCallback(VisibleCallback func)
{
    m_visibleFunc = func;
}

template <class ParentClass>
void KisCallbackBasedPaintopProperty<ParentClass>::readValueImpl()
{
    if (m_readFunc) m_readFunc(this);
}

template <class ParentClass>
void KisCallbackBasedPaintopProperty<ParentClass>::writeValueImpl()
{
    if (m_writeFunc) m_writeFunc(this);
}

template <class ParentClass>
bool KisCallbackBasedPaintopProperty<ParentClass>::isVisible() const
{
    return m_visibleFunc ? m_visibleFunc(this) : true;
}

#endif /* __KIS_CALLBACK_BASED_PAINTOP_PROPERTY_IMPL_H */
