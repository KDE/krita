/*
 * SPDX-FileCopyrightText: 2022 Freya Lupen <penguinflyer2222@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisCurveWidgetConnectionHelper.h"

#include <QMetaProperty>
#include "kis_debug.h"

#include "kis_cubic_curve.h"
#include "kis_curve_widget.h"


namespace {
class ConnectCurveWidgetHelper : public QObject
{
    Q_OBJECT
public:

    ConnectCurveWidgetHelper(KisCurveWidget *parent)
        : QObject(parent),
        m_curveWidget(parent)
    {
        connect(parent, &KisCurveWidget::curveChanged,
                this, &ConnectCurveWidgetHelper::slotWidgetChanged);
    }
public Q_SLOTS:
    void slotWidgetChanged() {
        Q_EMIT sigWidgetChanged(m_curveWidget->curve().toString());
    }

    void slotPropertyChanged(const QString &curve) {
        m_curveWidget->setCurve(curve);
    }

Q_SIGNALS:
    void sigWidgetChanged(const QString &curve);

private:
    KisCurveWidget *m_curveWidget;
};
}

namespace KisWidgetConnectionUtils
{

void connectControl(KisCurveWidget *widget, QObject *source, const char *property)
{
    const QMetaObject* meta = source->metaObject();
    QMetaProperty prop = meta->property(meta->indexOfProperty(property));

    KIS_SAFE_ASSERT_RECOVER_RETURN(prop.hasNotifySignal());

    QMetaMethod signal = prop.notifySignal();

    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterCount() >= 1);
    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterType(0) == QMetaType::type("QString"));

    ConnectCurveWidgetHelper *helper = new ConnectCurveWidgetHelper(widget);

    const QMetaObject* dstMeta = helper->metaObject();

    QMetaMethod updateSlot = dstMeta->method(
        dstMeta->indexOfSlot("slotPropertyChanged(QString)"));
    QObject::connect(source, signal, helper, updateSlot);

    helper->slotPropertyChanged(prop.read(source).toString());

    if (prop.isWritable()) {
        QObject::connect(helper, &ConnectCurveWidgetHelper::sigWidgetChanged,
                         source, [prop, source] (const QString &value) { prop.write(source, value); });
    }
}

}

#include "KisCurveWidgetConnectionHelper.moc"
