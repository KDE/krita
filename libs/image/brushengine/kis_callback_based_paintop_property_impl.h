/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_CALLBACK_BASED_PAINTOP_PROPERTY_IMPL_H
#define __KIS_CALLBACK_BASED_PAINTOP_PROPERTY_IMPL_H

#include <functional>

template <class ParentClass>
KisCallbackBasedPaintopProperty<ParentClass>::KisCallbackBasedPaintopProperty(typename ParentClass::Type type,
                                                                              typename ParentClass::SubType subType,
                                                                              const QString &id,
                                                                              const QString &name,
                                                                              KisPaintOpSettingsRestrictedSP settings,
                                                                              QObject *parent)
    : ParentClass(type, subType, id, name, settings, parent)
{
}

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
