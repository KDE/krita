/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCURVEWIDGETCONTROLSMANAGER_H
#define KISCURVEWIDGETCONTROLSMANAGER_H

#include <kritaui_export_instance.h>
#include <QObject>

class QSpinBox;
class QDoubleSpinBox;

namespace detail
{
/**
 * We don't use decltype(std::declval<SpinBox>().value()) to avoid
 * inclusion of both spinbox headers into this header.
 */
template <typename SpinBox>
struct value_of_spin_box;

template <>
struct value_of_spin_box<QSpinBox> {
    using type = int;
};

template <>
struct value_of_spin_box<QDoubleSpinBox> {
    using type = qreal;
};

template <typename SpinBox>
using value_of_spin_box_t = typename value_of_spin_box<SpinBox>::type;

} // namespace detail


class KisCurveWidget;

/**
 * A base class for the manager to let moc generate signal/slot metadata
 * (remember, moc doesn't support template classes)
 */
class KRITAUI_EXPORT KisCurveWidgetControlsManagerBase : public QObject
{
    Q_OBJECT
public:
    KisCurveWidgetControlsManagerBase(KisCurveWidget *curveWidget);
    ~KisCurveWidgetControlsManagerBase();

protected Q_SLOTS:
    virtual void inOutChanged() = 0;
    virtual void syncIOControls() = 0;
    virtual void focusIOControls() = 0;

protected:
    KisCurveWidget *m_curveWidget {nullptr};
};

template <typename SpinBox>
class KRITAUI_EXPORT_TEMPLATE KisCurveWidgetControlsManager : public KisCurveWidgetControlsManagerBase
{
public:
    using ValueType = detail::value_of_spin_box_t<SpinBox>;
public:
    KisCurveWidgetControlsManager(KisCurveWidget *curveWidget);
    KisCurveWidgetControlsManager(KisCurveWidget *curveWidget,
                                  SpinBox *in, SpinBox *out,
                                  ValueType inMin, ValueType inMax,
                                  ValueType outMin, ValueType outMax);
    ~KisCurveWidgetControlsManager();

    void setupInOutControls(SpinBox *in, SpinBox *out,
                            ValueType inMin, ValueType inMax,
                            ValueType outMin, ValueType outMax);
    void dropInOutControls();

protected:
    void inOutChanged() override;
    void syncIOControls() override;
    void focusIOControls() override;

private:
    /* In/Out controls */
    SpinBox *m_intIn {nullptr};
    SpinBox *m_intOut {nullptr};

    /* Working range of them */
    ValueType m_inMin {0};
    ValueType m_inMax {0};
    ValueType m_outMin {0};
    ValueType m_outMax {0};
};

extern template class KisCurveWidgetControlsManager<QSpinBox>;
extern template class KisCurveWidgetControlsManager<QDoubleSpinBox>;

using KisCurveWidgetControlsManagerInt = KisCurveWidgetControlsManager<QSpinBox>;
using KisCurveWidgetControlsManagerDouble = KisCurveWidgetControlsManager<QDoubleSpinBox>;


#endif // KISCURVEWIDGETCONTROLSMANAGER_H
