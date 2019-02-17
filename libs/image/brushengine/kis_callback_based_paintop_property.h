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
class KRITAIMAGE_EXPORT KisCallbackBasedPaintopProperty : public ParentClass
{
public:
    KisCallbackBasedPaintopProperty(typename ParentClass::Type type,
                                    const QString &id,
                                    const QString &name,
                                    KisPaintOpSettingsRestrictedSP settings,
                                    QObject *parent);

    KisCallbackBasedPaintopProperty(const QString &id,
                                const QString &name,
                                KisPaintOpSettingsRestrictedSP settings,
                                QObject *parent);

    typedef std::function<void (KisUniformPaintOpProperty*)> Callback;
    typedef std::function<bool (const KisUniformPaintOpProperty*)> VisibleCallback;

    void setReadCallback(Callback func);
    void setWriteCallback(Callback func);
    void setIsVisibleCallback(VisibleCallback func);

protected:
    void readValueImpl() override;
    void writeValueImpl() override;
    bool isVisible() const override;

private:
    Callback m_readFunc;
    Callback m_writeFunc;
    VisibleCallback m_visibleFunc;
};

#endif /* __KIS_CALLBACK_BASED_PAINTOP_PROPERTY_H */
