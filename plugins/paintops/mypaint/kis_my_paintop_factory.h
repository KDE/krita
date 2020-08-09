#ifndef KIS_MY_PAINTOP_FACTORY_H
#define KIS_MY_PAINTOP_FACTORY_H

#include <QObject>
#include <kis_paintop_factory.h>
#include <kis_simple_paintop_factory.h>

#include "kis_my_paintop.h"

class KisMyPaintOpFactory: public KisPaintOpFactory
{
    Q_OBJECT

public:

    KisMyPaintOpFactory();
    virtual ~KisMyPaintOpFactory();

    KisPaintOp* createOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image) override;
    KisPaintOpSettingsSP createSettings(KisResourcesInterfaceSP resourcesInterface) override;
    KisPaintOpConfigWidget* createConfigWidget(QWidget* parent) override;
    QString id() const override;
    QString name() const override;
    QIcon icon() override;
    QString category() const override;    

    QList<KoResourceSP> prepareLinkedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface) {
        return detail::prepareLinkedResources<KisMyPaintOp>(settings, resourcesInterface);
    }

    QList<KoResourceSP> prepareEmbeddedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface) {
        return detail::prepareEmbeddedResources<KisMyPaintOp>(settings, resourcesInterface);
    }    

Q_SIGNALS:
    void progressMessage(const QString&);

public Q_SLOTS:
    void processAfterLoading();

private:

    class Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_MY_PAINTOP_FACTORY_H
