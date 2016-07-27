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

#ifndef __KIS_CALLBACK_BASED_PAINTOP_PROPERTY_H
#define __KIS_CALLBACK_BASED_PAINTOP_PROPERTY_H

#include <functional>


template <class ParentClass>
class KisCallbackBasedPaintopProperty : public ParentClass
{
public:
    KisCallbackBasedPaintopProperty(typename ParentClass::Type type,
                                    const QString &id,
                                    const QString &name,
                                    KisPaintOpSettingsSP settings,
                                    QObject *parent)
        : ParentClass(type, id, name, settings, parent) {}

    KisCallbackBasedPaintopProperty(const QString &id,
                                const QString &name,
                                KisPaintOpSettingsSP settings,
                                QObject *parent)
    : ParentClass(id, name, settings, parent) {}

    typedef std::function<void (KisUniformPaintOpProperty*)> Callback;

    void setReadCallback(Callback func) { m_readFunc = func; }
    void setWriteCallback(Callback func) { m_writeFunc = func; }

protected:
    virtual void readValueImpl() { if (m_readFunc) m_readFunc(this); }
    virtual void writeValueImpl() { if (m_writeFunc) m_writeFunc(this); }

private:
    Callback m_readFunc;
    Callback m_writeFunc;
};

#endif /* __KIS_CALLBACK_BASED_PAINTOP_PROPERTY_H */
