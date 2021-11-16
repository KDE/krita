/*
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.com>
 * SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MY_PAINTOP_FACTORY_H
#define KIS_MY_PAINTOP_FACTORY_H

#include <QObject>
#include <kis_paintop_factory.h>

class KisMyPaintOpFactory: public KisPaintOpFactory
{
    Q_OBJECT

public:

    KisMyPaintOpFactory();
    virtual ~KisMyPaintOpFactory();

    KisPaintOp *createOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image) override;
    KisPaintOpSettingsSP createSettings(KisResourcesInterfaceSP resourcesInterface) override;
    KisPaintOpConfigWidget* createConfigWidget(QWidget* parent) override;
    QString id() const override;
    QString name() const override;
    QIcon icon() override;
    QString category() const override;

    QList<KoResourceLoadResult> prepareLinkedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface) override;
    QList<KoResourceLoadResult> prepareEmbeddedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface) override;


private:

    class Private;
    Private* const m_d;
};

#endif // KIS_MY_PAINTOP_FACTORY_H
