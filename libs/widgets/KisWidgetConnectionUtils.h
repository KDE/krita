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
class QAction;
class QComboBox;
class QButtonGroup;
class QSpinBox;
class QDoubleSpinBox;
class QObject;
class KisSpacingSelectionWidget;
class KisAngleSelector;
class QLineEdit;
class KisMultipliersDoubleSliderSpinBox;
class KisFileNameRequester;

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
using ButtonGroupState = ControlState<int>;

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

template <typename T>
struct SpinBoxState {
    T value = T{};
    T min = T{};
    T max = T{};
    bool enabled = true;
};

struct ToSpinBoxState {
    template <typename T>
    SpinBoxState<std::decay_t<T>> operator()(T &&value, T &&min, T &&max, bool enabled) {
        return {std::forward<T>(value), std::forward<T>(min), std::forward<T>(max), enabled};
    }
};

using IntSpinBoxState = SpinBoxState<int>;
using DoubleSpinBoxState = SpinBoxState<qreal>;

void KRITAWIDGETS_EXPORT connectControl(QAbstractButton *button, QObject *source, const char *property);
void KRITAWIDGETS_EXPORT connectControl(QAction *button, QObject *source, const char *property);
void KRITAWIDGETS_EXPORT connectControl(QSpinBox *spinBox, QObject *source, const char *property);
void KRITAWIDGETS_EXPORT connectControl(QDoubleSpinBox *spinBox, QObject *source, const char *property);
void KRITAWIDGETS_EXPORT connectControlState(QDoubleSpinBox *spinBox, QObject *source, const char *readStateProperty, const char *writeProperty);
void KRITAWIDGETS_EXPORT connectControlState(QSpinBox *spinBox, QObject *source, const char *readStateProperty, const char *writeProperty);
void KRITAWIDGETS_EXPORT connectControl(KisMultipliersDoubleSliderSpinBox *spinBox, QObject *source, const char *property);
void KRITAWIDGETS_EXPORT connectControl(QButtonGroup *button, QObject *source, const char *property);
void KRITAWIDGETS_EXPORT connectControl(QComboBox *button, QObject *source, const char *property);
void KRITAWIDGETS_EXPORT connectControl(QComboBox *button, QObject *source, const char *property);
void KRITAWIDGETS_EXPORT connectControlState(QAbstractButton *button, QObject *source, const char *readStateProperty, const char *writeProperty);
void KRITAWIDGETS_EXPORT connectControlState(QButtonGroup *group, QObject *source, const char *readStateProperty, const char *writeProperty);
void KRITAWIDGETS_EXPORT connectControlState(QComboBox *button, QObject *source, const char *readStateProperty, const char *writeProperty);
void KRITAWIDGETS_EXPORT connectControl(KisSpacingSelectionWidget *widget, QObject *source, const char *property);
void KRITAWIDGETS_EXPORT connectControl(KisAngleSelector *widget, QObject *source, const char *property);
void KRITAWIDGETS_EXPORT connectControl(QLineEdit *widget, QObject *source, const char *property);
void KRITAWIDGETS_EXPORT connectControl(KisFileNameRequester *widget, QObject *source, const char *property);
void KRITAWIDGETS_EXPORT connectWidgetVisibleToProperty(QWidget* widget, QObject* source, const char* property);


} // namespace KisWidgetConnectionUtils

using KisWidgetConnectionUtils::CheckBoxState;
using KisWidgetConnectionUtils::ButtonGroupState;
using KisWidgetConnectionUtils::ComboBoxState;
using KisWidgetConnectionUtils::SpacingState;
using KisWidgetConnectionUtils::DoubleSpinBoxState;
using KisWidgetConnectionUtils::IntSpinBoxState;

Q_DECLARE_METATYPE(CheckBoxState)
Q_DECLARE_METATYPE(ButtonGroupState)
Q_DECLARE_METATYPE(ComboBoxState)
Q_DECLARE_METATYPE(SpacingState)
Q_DECLARE_METATYPE(DoubleSpinBoxState)
Q_DECLARE_METATYPE(IntSpinBoxState)


#endif // KISWIDGETCONNECTIONUTILS_H
