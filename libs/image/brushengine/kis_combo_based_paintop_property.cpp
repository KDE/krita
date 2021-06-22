/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_combo_based_paintop_property.h"
#include "kis_paintop_settings.h"

#include "QIcon"


struct KisComboBasedPaintOpProperty::Private
{
    QList<QString> items;
    QList<QIcon> icons;
};

KisComboBasedPaintOpProperty::KisComboBasedPaintOpProperty(const QString &id,
                                                           const QString &name,
                                                           KisPaintOpSettingsRestrictedSP settings,
                                                           QObject *parent)
    : KisUniformPaintOpProperty(Combo, id, name, settings, parent),
      m_d(new Private)
{
}

KisComboBasedPaintOpProperty::KisComboBasedPaintOpProperty(Type type,
                                                           const QString &id,
                                                           const QString &name,
                                                           KisPaintOpSettingsRestrictedSP settings,
                                                           QObject *parent)
    : KisUniformPaintOpProperty(Combo, id, name, settings, parent),
      m_d(new Private)
{
    KIS_ASSERT_RECOVER_RETURN(type == Combo);
}

KisComboBasedPaintOpProperty::KisComboBasedPaintOpProperty(Type type,
                                                           SubType subType,
                                                           const QString &id,
                                                           const QString &name,
                                                           KisPaintOpSettingsRestrictedSP settings,
                                                           QObject *parent)
    : KisUniformPaintOpProperty(Combo, subType, id, name, settings, parent),
      m_d(new Private)
{
    KIS_ASSERT_RECOVER_RETURN(type == Combo);
}

KisComboBasedPaintOpProperty::~KisComboBasedPaintOpProperty()
{
}

QList<QString> KisComboBasedPaintOpProperty::items() const
{
    return m_d->items;
}

void KisComboBasedPaintOpProperty::setItems(const QList<QString> &list)
{
    m_d->items = list;
}

QList<QIcon> KisComboBasedPaintOpProperty::icons() const
{
    return m_d->icons;
}

void KisComboBasedPaintOpProperty::setIcons(const QList<QIcon> &list)
{
    m_d->icons = list;
}

#include "kis_callback_based_paintop_property_impl.h"
template class KisCallbackBasedPaintopProperty<KisComboBasedPaintOpProperty>;
