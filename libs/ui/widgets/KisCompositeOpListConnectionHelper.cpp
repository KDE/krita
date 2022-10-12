/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisCompositeOpListConnectionHelper.h"

#include <QMetaObject>
#include <QMetaProperty>
#include <KoCompositeOpRegistry.h>
#include <kis_assert.h>
#include "kis_cmb_composite.h"

namespace KisWidgetConnectionUtils
{
class ConnectCompositeOpListWidgetHelper : public QObject
{
    Q_OBJECT
public:

    ConnectCompositeOpListWidgetHelper(KisCompositeOpListWidget *parent)
        : QObject(parent),
          m_listWidget(parent)
    {
        connect(parent, &KisCompositeOpListWidget::clicked,
                this, &ConnectCompositeOpListWidgetHelper::slotWidgetChanged);
    }
public Q_SLOTS:
    void slotWidgetChanged() {
        Q_EMIT sigWidgetChanged(m_listWidget->selectedCompositeOp().id());
    }

    void slotPropertyChanged(const QString &id) {
        m_listWidget->setCompositeOp(KoCompositeOpRegistry::instance().getKoID(id));
    }

Q_SIGNALS:
    void sigWidgetChanged(const QString &id);

private:
    KisCompositeOpListWidget *m_listWidget;
};

void connectControl(KisCompositeOpListWidget *widget, QObject *source, const char *property)
{
    const QMetaObject* meta = source->metaObject();
    QMetaProperty prop = meta->property(meta->indexOfProperty(property));

    KIS_SAFE_ASSERT_RECOVER_RETURN(prop.hasNotifySignal());

    QMetaMethod signal = prop.notifySignal();

    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterCount() >= 1);
    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterType(0) == QMetaType::type("QString"));

    ConnectCompositeOpListWidgetHelper *helper = new ConnectCompositeOpListWidgetHelper(widget);

    const QMetaObject* dstMeta = helper->metaObject();

    QMetaMethod updateSlot = dstMeta->method(
                dstMeta->indexOfSlot("slotPropertyChanged(QString)"));
    QObject::connect(source, signal, helper, updateSlot);

    helper->slotPropertyChanged(prop.read(source).value<QString>());

    if (prop.isWritable()) {
        QObject::connect(helper, &ConnectCompositeOpListWidgetHelper::sigWidgetChanged, [prop, source] (const QString &value) { prop.write(source, QVariant::fromValue(value)); });
    }
}

}

#include "KisCompositeOpListConnectionHelper.moc"
