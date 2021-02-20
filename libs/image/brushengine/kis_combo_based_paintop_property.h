/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
