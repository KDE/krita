/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
