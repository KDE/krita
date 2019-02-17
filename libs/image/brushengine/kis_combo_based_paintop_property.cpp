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
