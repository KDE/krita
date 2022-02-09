/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisWidgetConnectionUtils.h"

#include <QMetaObject>
#include <QMetaProperty>
#include <QAbstractButton>
#include <QComboBox>
#include <QButtonGroup>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include "kis_debug.h"
#include "kis_spacing_selection_widget.h"
#include "KisAngleSelector.h"

class ConnectButtonStateHelper : public QObject
{
    Q_OBJECT
public:

    ConnectButtonStateHelper(QAbstractButton *parent)
        : QObject(parent),
          m_button(parent)
    {
    }
public Q_SLOTS:
    void updateState(const CheckBoxState &state) {
        QSignalBlocker b(m_button);
        m_button->setEnabled(state.enabled);
        m_button->setChecked(state.value);

        // TODO: verify if the two properties are equal or the control is disabled
    }

private:
    QAbstractButton *m_button;
};

class ConnectComboBoxStateHelper : public QObject
{
    Q_OBJECT
public:

    ConnectComboBoxStateHelper(QComboBox *parent)
        : QObject(parent),
          m_comboBox(parent)
    {
    }
public Q_SLOTS:
    void updateState(const ComboBoxState &state) {
        QSignalBlocker b(m_comboBox);

        while (m_comboBox->count() > 0) {
            m_comboBox->removeItem(0);
        }

        m_comboBox->addItems(state.items);
        m_comboBox->setCurrentIndex(state.currentIndex);
        m_comboBox->setEnabled(state.enabled);

        // TODO: verify if the two properties are equal or the control is disabled
    }

private:
    QComboBox *m_comboBox;
};


namespace KisWidgetConnectionUtils {

void connectControl(QAbstractButton *button, QObject *source, const char *property)
{
    const QMetaObject* meta = source->metaObject();
    QMetaProperty prop = meta->property(meta->indexOfProperty(property));

    KIS_SAFE_ASSERT_RECOVER_RETURN(prop.hasNotifySignal());

    QMetaMethod signal = prop.notifySignal();

    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterCount() >= 1);
    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterType(0) == QMetaType::type("bool"));

    const QMetaObject* dstMeta = button->metaObject();

    QMetaMethod updateSlot = dstMeta->method(
                dstMeta->indexOfSlot("setChecked(bool)"));
    QObject::connect(source, signal, button, updateSlot);

    button->setChecked(prop.read(source).toBool());

    if (prop.isWritable()) {
        QObject::connect(button, &QAbstractButton::toggled, [prop, source] (bool value) { prop.write(source, value); });
    }
}

void connectControl(QSpinBox *spinBox, QObject *source, const char *property)
{
    const QMetaObject* meta = source->metaObject();
    QMetaProperty prop = meta->property(meta->indexOfProperty(property));

    KIS_SAFE_ASSERT_RECOVER_RETURN(prop.hasNotifySignal());

    QMetaMethod signal = prop.notifySignal();

    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterCount() >= 1);
    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterType(0) == QMetaType::type("int"));

    const QMetaObject* dstMeta = spinBox->metaObject();

    QMetaMethod updateSlot = dstMeta->method(
                dstMeta->indexOfSlot("setValue(int)"));
    QObject::connect(source, signal, spinBox, updateSlot);

    spinBox->setValue(prop.read(source).toInt());

    if (prop.isWritable()) {
        QObject::connect(spinBox, qOverload<int>(&QSpinBox::valueChanged), [prop, source] (int value) { prop.write(source, value); });
    }
}

void connectControl(QDoubleSpinBox *spinBox, QObject *source, const char *property)
{
    const QMetaObject* meta = source->metaObject();
    QMetaProperty prop = meta->property(meta->indexOfProperty(property));

    KIS_SAFE_ASSERT_RECOVER_RETURN(prop.hasNotifySignal());

    QMetaMethod signal = prop.notifySignal();

    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterCount() >= 1);
    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterType(0) == QMetaType::type("qreal"));

    const QMetaObject* dstMeta = spinBox->metaObject();

    QMetaMethod updateSlot = dstMeta->method(
                dstMeta->indexOfSlot("setValue(qreal)"));
    QObject::connect(source, signal, spinBox, updateSlot);

    spinBox->setValue(prop.read(source).toReal());

    if (prop.isWritable()) {
        QObject::connect(spinBox, qOverload<qreal>(&QDoubleSpinBox::valueChanged), [prop, source] (qreal value) { prop.write(source, value); });
    }
}


class ConnectButtonGroupHelper : public QObject
{
    Q_OBJECT
public:

    ConnectButtonGroupHelper(QButtonGroup *parent)
        : QObject(parent),
          m_buttonGroup(parent)
    {
    }
public Q_SLOTS:
    void updateState(int value) {
        QAbstractButton *btn = m_buttonGroup->button(value);
        KIS_SAFE_ASSERT_RECOVER_RETURN(btn);
        btn->setChecked(true);
    }

    void slotButtonClicked(QAbstractButton *btn) {
        int id = m_buttonGroup->id(btn);
        KIS_SAFE_ASSERT_RECOVER_RETURN(id >= 0);

        Q_EMIT idClicked(id);
    }

Q_SIGNALS:
    // this signal was added only in Qt 5.15
    void idClicked(int id);

private:
    QButtonGroup *m_buttonGroup;
};

void connectControl(QButtonGroup *group, QObject *source, const char *property)
{
    const QMetaObject* meta = source->metaObject();
    QMetaProperty prop = meta->property(meta->indexOfProperty(property));

    KIS_SAFE_ASSERT_RECOVER_RETURN(prop.hasNotifySignal());

    QMetaMethod signal = prop.notifySignal();

    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterCount() >= 1);
    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterType(0) == QMetaType::type("int"));


    ConnectButtonGroupHelper *helper = new ConnectButtonGroupHelper(group);

    const QMetaObject* dstMeta = helper->metaObject();

    QMetaMethod updateSlot = dstMeta->method(
                dstMeta->indexOfSlot("updateState(int)"));
    QObject::connect(source, signal, helper, updateSlot);

    helper->updateState(prop.read(source).toInt());

    if (prop.isWritable()) {
        QObject::connect(group, qOverload<QAbstractButton *>(&QButtonGroup::buttonClicked), helper, &ConnectButtonGroupHelper::slotButtonClicked);
        QObject::connect(helper, &ConnectButtonGroupHelper::idClicked, [prop, source] (int value) { prop.write(source, value); });
    }
}

void connectControlState(QAbstractButton *button, QObject *source, const char *readStatePropertyName, const char *writePropertyName)
{
    const QMetaObject* meta = source->metaObject();
    QMetaProperty readStateProp = meta->property(meta->indexOfProperty(readStatePropertyName));

    KIS_SAFE_ASSERT_RECOVER_RETURN(readStateProp.hasNotifySignal());

    QMetaMethod signal = readStateProp.notifySignal();

    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterCount() >= 1);
    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterType(0) == QMetaType::type("CheckBoxState"));

    ConnectButtonStateHelper *helper = new ConnectButtonStateHelper(button);

    const QMetaObject* dstMeta = helper->metaObject();

    QMetaMethod updateSlot = dstMeta->method(
                dstMeta->indexOfSlot("updateState(CheckBoxState)"));
    QObject::connect(source, signal, helper, updateSlot);

    helper->updateState(readStateProp.read(source).value<CheckBoxState>());

    QMetaProperty writeProp = meta->property(meta->indexOfProperty(writePropertyName));
    if (writeProp.isWritable()) {
        button->connect(button, &QAbstractButton::toggled, [writeProp, source] (bool value) { writeProp.write(source, value); });
    }
}


void connectControlState(QComboBox *button, QObject *source, const char *readStatePropertyName, const char *writePropertyName)
{
    const QMetaObject* meta = source->metaObject();
    QMetaProperty readStateProp = meta->property(meta->indexOfProperty(readStatePropertyName));

    KIS_SAFE_ASSERT_RECOVER_RETURN(readStateProp.hasNotifySignal());

    QMetaMethod signal = readStateProp.notifySignal();

    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterCount() >= 1);
    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterType(0) == QMetaType::type("ComboBoxState"));

    ConnectComboBoxStateHelper *helper = new ConnectComboBoxStateHelper(button);

    const QMetaObject* dstMeta = helper->metaObject();

    QMetaMethod updateSlot = dstMeta->method(
                dstMeta->indexOfSlot("updateState(ComboBoxState)"));
    QObject::connect(source, signal, helper, updateSlot);

    helper->updateState(readStateProp.read(source).value<ComboBoxState>());

    QMetaProperty writeProp = meta->property(meta->indexOfProperty(writePropertyName));
    if (writeProp.isWritable()) {
        QObject::connect(button, qOverload<int>(&QComboBox::currentIndexChanged), [writeProp, source] (int value) { writeProp.write(source, value); });
    }
}

void connectControl(QComboBox *button, QObject *source, const char *property)
{
    const QMetaObject* meta = source->metaObject();
    QMetaProperty stateProp = meta->property(meta->indexOfProperty(property));

    KIS_SAFE_ASSERT_RECOVER_RETURN(stateProp.hasNotifySignal());

    QMetaMethod signal = stateProp.notifySignal();

    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterCount() >= 1);
    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterType(0) == QMetaType::type("int"));

    const QMetaObject* dstMeta = button->metaObject();

    QMetaMethod updateSlot = dstMeta->method(
                dstMeta->indexOfSlot("setCurrentIndex(int)"));
    QObject::connect(source, signal, button, updateSlot);

    button->setCurrentIndex(stateProp.read(source).value<int>());

    if (stateProp.isWritable()) {
        QObject::connect(button, qOverload<int>(&QComboBox::currentIndexChanged), [stateProp, source] (int value) { stateProp.write(source, value); });
    }
}

class ConnectSpacingWidgetHelper : public QObject
{
    Q_OBJECT
public:

    ConnectSpacingWidgetHelper(KisSpacingSelectionWidget *parent)
        : QObject(parent),
          m_spacingWidget(parent)
    {
    }
public Q_SLOTS:
    void slotWidgetChanged() {
        Q_EMIT sigWidgetChanged({m_spacingWidget->spacing(), m_spacingWidget->autoSpacingActive(), m_spacingWidget->autoSpacingCoeff()});
    }

    void slotPropertyChanged(SpacingState state) {
        m_spacingWidget->setSpacing(state.useAutoSpacing, state.useAutoSpacing ? state.autoSpacingCoeff : state.spacing);
    }

Q_SIGNALS:
    // this signal was added only in Qt 5.15
    void sigWidgetChanged(SpacingState state);

private:
    KisSpacingSelectionWidget *m_spacingWidget;
};

void connectControl(KisSpacingSelectionWidget *widget, QObject *source, const char *property)
{
    const QMetaObject* meta = source->metaObject();
    QMetaProperty stateProp = meta->property(meta->indexOfProperty(property));

    KIS_SAFE_ASSERT_RECOVER_RETURN(stateProp.hasNotifySignal());

    QMetaMethod signal = stateProp.notifySignal();

    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterCount() >= 1);
    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterType(0) == QMetaType::type("SpacingState"));

    ConnectSpacingWidgetHelper *helper = new ConnectSpacingWidgetHelper(widget);

    const QMetaObject* dstMeta = helper->metaObject();

    QMetaMethod updateSlot = dstMeta->method(
                dstMeta->indexOfSlot("slotPropertyChanged(SpacingState)"));
    QObject::connect(source, signal, helper, updateSlot);

    helper->slotPropertyChanged(stateProp.read(source).value<SpacingState>());

    if (stateProp.isWritable()) {
        QObject::connect(helper, &ConnectSpacingWidgetHelper::sigWidgetChanged, [stateProp, source] (SpacingState value) { stateProp.write(source, QVariant::fromValue(value)); });
    }
}

void connectControl(KisAngleSelector *widget, QObject *source, const char *property)
{
    const QMetaObject* meta = source->metaObject();
    QMetaProperty prop = meta->property(meta->indexOfProperty(property));

    KIS_SAFE_ASSERT_RECOVER_RETURN(prop.hasNotifySignal());

    QMetaMethod signal = prop.notifySignal();

    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterCount() >= 1);
    KIS_SAFE_ASSERT_RECOVER_RETURN(signal.parameterType(0) == QMetaType::type("qreal"));

    const QMetaObject* dstMeta = widget->metaObject();

    QMetaMethod updateSlot = dstMeta->method(
                dstMeta->indexOfSlot("setAngle(qreal)"));
    QObject::connect(source, signal, widget, updateSlot);

    widget->setAngle(prop.read(source).toReal());

    if (prop.isWritable()) {
        QObject::connect(widget, &KisAngleSelector::angleChanged, [prop, source] (qreal value) { prop.write(source, value); });
    }
}

}

#include <KisWidgetConnectionUtils.moc>
