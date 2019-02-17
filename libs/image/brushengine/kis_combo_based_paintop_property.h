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

#ifndef __KIS_COMBO_BASED_PAINTOP_PROPERTY_H
#define __KIS_COMBO_BASED_PAINTOP_PROPERTY_H

#include <QScopedPointer>

#include "kis_uniform_paintop_property.h"
#include "kis_types.h"
#include "kritaimage_export.h"

class QIcon;


class KRITAIMAGE_EXPORT KisComboBasedPaintOpProperty : public KisUniformPaintOpProperty
{
public:
    KisComboBasedPaintOpProperty(const QString &id,
                                 const QString &name,
                                 KisPaintOpSettingsRestrictedSP settings,
                                 QObject *parent);
    ~KisComboBasedPaintOpProperty() override;

    // callback-compatible c-tor
    KisComboBasedPaintOpProperty(Type type,
                                 const QString &id,
                                 const QString &name,
                                 KisPaintOpSettingsRestrictedSP settings,
                                 QObject *parent);

    QList<QString> items() const;
    void setItems(const QList<QString> &list);

    QList<QIcon> icons() const;
    void setIcons(const QList<QIcon> &list);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#include "kis_callback_based_paintop_property.h"
extern template class KisCallbackBasedPaintopProperty<KisComboBasedPaintOpProperty>;
typedef KisCallbackBasedPaintopProperty<KisComboBasedPaintOpProperty> KisComboBasedPaintOpPropertyCallback;

#endif /* __KIS_COMBO_BASED_PAINTOP_PROPERTY_H */
