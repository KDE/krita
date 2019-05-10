/*
 *  Copyright (c) 2019 Boudewijn Rempt <boud@kde.org>
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
#include "KisDialogStateSaver.h"

#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include <QDebug>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QAbstractSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include "kis_int_parse_spin_box.h"
#include "kis_double_parse_spin_box.h"
#include "kis_double_parse_unit_spin_box.h"


void KisDialogStateSaver::saveState(QWidget *parent, const QString &dialogName)
{
    Q_ASSERT(parent);
    Q_ASSERT(!dialogName.isEmpty());

    KConfigGroup group(KSharedConfig::openConfig(), dialogName);
    Q_FOREACH(QWidget *widget, parent->findChildren<QWidget*>(QString())) {

        if (!widget->objectName().isEmpty() ) {
            if (qobject_cast<KisIntParseSpinBox*>(widget)) {
                group.writeEntry(widget->objectName(), qobject_cast<KisIntParseSpinBox*>(widget)->value());
            }
            else if (qobject_cast<KisDoubleParseSpinBox*>(widget)) {
                group.writeEntry(widget->objectName(), qobject_cast<KisDoubleParseSpinBox*>(widget)->value());
            }
            else if (qobject_cast<KisDoubleParseUnitSpinBox*>(widget)) {
                // XXX: also save the unit
                group.writeEntry(widget->objectName(), qobject_cast<KisDoubleParseUnitSpinBox*>(widget)->value());
            }
            else if (qobject_cast<QCheckBox*>(widget)) {
                group.writeEntry(widget->objectName(), qobject_cast<const QCheckBox*>(widget)->isChecked());
            }
            else if (qobject_cast<QComboBox*>(widget)) {
                group.writeEntry(widget->objectName(), qobject_cast<QComboBox*>(widget)->currentIndex());
            }
            else if (qobject_cast<QLineEdit*>(widget)) {
                group.writeEntry(widget->objectName(), qobject_cast<QLineEdit*>(widget)->text());
            }
            else if (qobject_cast<QAbstractSlider*>(widget)) {
                group.writeEntry(widget->objectName(), qobject_cast<QAbstractSlider*>(widget)->value());
            }
            else if (qobject_cast<QSpinBox*>(widget)) {
                group.writeEntry(widget->objectName(), qobject_cast<QSpinBox*>(widget)->value());
            }
            else if (qobject_cast<QDoubleSpinBox*>(widget)) {
                group.writeEntry(widget->objectName(), qobject_cast<QDoubleSpinBox*>(widget)->value());
            }

            else {
                qWarning() << "Cannot save state for object" << widget;
            }
        }
        else {
            qWarning() << "Dialog" << dialogName << "has a widget without an objectname:" << widget;
        }

    }
}

void KisDialogStateSaver::restoreState(QWidget *parent, const QString &dialogName, const QMap<QString, QVariant> &defaults)
{
    Q_ASSERT(parent);
    Q_ASSERT(!dialogName.isEmpty());

    KConfigGroup group( KSharedConfig::openConfig(), dialogName);

    Q_FOREACH(QWidget *widget, parent->findChildren<QWidget*>(QString())) {

        if (!widget->objectName().isEmpty()) {

            QString widgetName = widget->objectName();

            QVariant defaultValue;
            if (defaults.contains(widgetName)) {
                defaultValue = defaults[widgetName];
            }

            if (qobject_cast<KisIntParseSpinBox*>(widget)) {
                if (defaultValue.isValid()) {
                    qobject_cast<KisIntParseSpinBox*>(widget)->setValue(defaultValue.toInt());
                }
                else {
                    qobject_cast<KisIntParseSpinBox*>(widget)->setValue(group.readEntry<int>(widgetName, qobject_cast<KisIntParseSpinBox*>(widget)->value()));
                }
            }
            else if (qobject_cast<KisDoubleParseSpinBox*>(widget)) {
                if (defaultValue.isValid()) {
                    qobject_cast<KisDoubleParseSpinBox*>(widget)->setValue(defaultValue.toInt());
                }
                else {
                    qobject_cast<KisDoubleParseSpinBox*>(widget)->setValue(group.readEntry<int>(widgetName, qobject_cast<KisDoubleParseSpinBox*>(widget)->value()));
                }
            }
            else if (qobject_cast<KisDoubleParseUnitSpinBox*>(widget)) {
                if (defaultValue.isValid()) {
                    qobject_cast<KisDoubleParseUnitSpinBox*>(widget)->setValue(defaultValue.toInt());
                }
                else {
                    qobject_cast<KisDoubleParseUnitSpinBox*>(widget)->setValue(group.readEntry<int>(widgetName, qobject_cast<KisDoubleParseUnitSpinBox*>(widget)->value()));
                }
            }
            else if (qobject_cast<QCheckBox*>(widget)) {
                if (defaultValue.isValid()) {
                    qobject_cast<QCheckBox*>(widget)->setChecked(defaultValue.toBool());
                }
                else {
                    qobject_cast<QCheckBox*>(widget)->setChecked(group.readEntry<bool>(widgetName, qobject_cast<QCheckBox*>(widget)->isChecked()));
                }
            }
            else if (qobject_cast<QComboBox*>(widget)) {
                if (defaultValue.isValid()) {
                    qobject_cast<QComboBox*>(widget)->setCurrentIndex(defaultValue.toInt());
                }
                else {
                    qobject_cast<QComboBox*>(widget)->setCurrentIndex(group.readEntry<int>(widgetName, qobject_cast<QComboBox*>(widget)->currentIndex()));
                }
            }
            else if (qobject_cast<QLineEdit*>(widget)) {
                if (defaultValue.isValid()) {
                    qobject_cast<QLineEdit*>(widget)->setText(defaultValue.toString());
                }
                else {
                    qobject_cast<QLineEdit*>(widget)->setText(group.readEntry<QString>(widgetName, qobject_cast<QLineEdit*>(widget)->text()));
                }
            }
            else if (qobject_cast<QAbstractSlider*>(widget)) {
                if (defaultValue.isValid()) {
                    qobject_cast<QAbstractSlider*>(widget)->setValue(defaultValue.toInt());
                }
                else {
                    qobject_cast<QAbstractSlider*>(widget)->setValue(group.readEntry<int>(widgetName, qobject_cast<QAbstractSlider*>(widget)->value()));
                }
            }
            else if (qobject_cast<QSpinBox*>(widget)) {
                if (defaultValue.isValid()) {
                    qobject_cast<QSpinBox*>(widget)->setValue(defaultValue.toInt());
                }
                else {
                    qobject_cast<QSpinBox*>(widget)->setValue(group.readEntry<int>(widgetName, qobject_cast<QSpinBox*>(widget)->value()));
                }
            }
            else if (qobject_cast<QDoubleSpinBox*>(widget)) {
                if (defaultValue.isValid()) {
                    qobject_cast<QDoubleSpinBox*>(widget)->setValue(defaultValue.toDouble());
                }
                else {
                    qobject_cast<QDoubleSpinBox*>(widget)->setValue(group.readEntry<int>(widgetName, qobject_cast<QDoubleSpinBox*>(widget)->value()));
                }

            }
            else {
                //qWarning() << "Cannot restore state for object" << widget;
            }
        }
        else {
            qWarning() << "Dialog" << dialogName << "has a widget without an object name:" << widget;
        }
    }
}
