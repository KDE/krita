/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISWIDGETCONNECTIONUTILS_H
#define KISWIDGETCONNECTIONUTILS_H

#include <QMetaType>
#include <QStringList>

#include "kritawidgets_export.h"

class QAbstractButton;
class QComboBox;
class QButtonGroup;
class QSpinBox;
class QDoubleSpinBox;
class QObject;
class KisSpacingSelectionWidget;
class KisAngleSelector;

namespace KisWidgetConnectionUtils {

template<typename T>
struct ControlState {
    T value = T{};
    bool enabled = true;
};

struct ToControlState {
    template<typename T>
    ControlState<std::decay_t<T>> operator()(T &&value, bool enabled) {
        return {std::forward<T>(value), enabled};
    }
};

using CheckBoxState = ControlState<bool>;

struct ComboBoxState {
    QStringList items;
    int currentIndex = -1;
    bool enabled = true;
    QStringList toolTips;
};

struct SpacingState {
    qreal spacing = 0.05;
    bool useAutoSpacing = false;
    qreal autoSpacingCoeff = 1.0;
};

struct ToSpacingState {
    SpacingState operator() (qreal spacing, bool useAutoSpacing, qreal autoSpacingCoeff) {
        return {spacing, useAutoSpacing, autoSpacingCoeff};
    }
};

struct FromSpacingState {
    std::tuple<qreal, bool, qreal> operator() (const SpacingState &x) {
        return {x.spacing, x.useAutoSpacing, x.autoSpacingCoeff};
    }
};

void KRITAWIDGETS_EXPORT connectControl(QAbstractButton *button, QObject *source, const char *property);
void KRITAWIDGETS_EXPORT connectControl(QSpinBox *spinBox, QObject *source, const char *property);
void KRITAWIDGETS_EXPORT connectControl(QDoubleSpinBox *spinBox, QObject *source, const char *property);
void KRITAWIDGETS_EXPORT connectControl(QButtonGroup *button, QObject *source, const char *property);
void KRITAWIDGETS_EXPORT connectControl(QComboBox *button, QObject *source, const char *property);
void KRITAWIDGETS_EXPORT connectControl(QComboBox *button, QObject *source, const char *property);
void KRITAWIDGETS_EXPORT connectControlState(QAbstractButton *button, QObject *source, const char *readStateProperty, const char *writeProperty);
void KRITAWIDGETS_EXPORT connectControlState(QComboBox *button, QObject *source, const char *readStateProperty, const char *writeProperty);
void KRITAWIDGETS_EXPORT connectControl(KisSpacingSelectionWidget *widget, QObject *source, const char *property);
void KRITAWIDGETS_EXPORT connectControl(KisAngleSelector *widget, QObject *source, const char *property);

};

using KisWidgetConnectionUtils::CheckBoxState;
using KisWidgetConnectionUtils::ComboBoxState;
using KisWidgetConnectionUtils::SpacingState;

Q_DECLARE_METATYPE(CheckBoxState)
Q_DECLARE_METATYPE(ComboBoxState)
Q_DECLARE_METATYPE(SpacingState)


#endif // KISWIDGETCONNECTIONUTILS_H
