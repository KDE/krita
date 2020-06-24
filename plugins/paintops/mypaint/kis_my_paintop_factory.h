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

    KisPaintOp* createOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image) override;
    KisPaintOpSettingsSP settings() override;
    KisPaintOpConfigWidget* createConfigWidget(QWidget* parent) override;
    QString id() const override;
    QString name() const override;
    QIcon icon() override;
    QString category() const override;
    void processAfterLoading() override;

private:

    class Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_MY_PAINTOP_FACTORY_H
