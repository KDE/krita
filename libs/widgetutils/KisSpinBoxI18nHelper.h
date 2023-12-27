/*
 * SPDX-FileCopyrightText: 2021-2023 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KIS_SPIN_BOX_I18N_HELPER_H
#define KIS_SPIN_BOX_I18N_HELPER_H

#include <functional>

#include <QStringView>

#include "kritawidgetutils_export.h"

class QDoubleSpinBox;
class QSpinBox;
class QString;

class KisSelectionPropertySliderBase;

namespace KisSpinBoxI18nHelper
{
    /**
     * Handles pluralization of prefix/suffix of `QSpinBox`-like widgets using
     * an i18n string in the form of `prefix {n} suffix`. This uses the
     * `valueChanged` signal to automatically update the text.
     *
     * In case the `valueChanged` signal wouldn't be emitted (i.e. signals
     * are blocked), call `KisSpinBoxI18nHelper::update` to update the text.
     *
     * @param spinBox The QSpinBox to handle.
     * @param messageFn A function (usually a lambda expression) that receives
     *                  the current value to be shown and returns a localized
     *                  `QString` in the form of `prefix {n} suffix`. The
     *                  prefix and suffix of `spinBox` will be set accordingly.
     */
    KRITAWIDGETUTILS_EXPORT void install(QSpinBox *spinBox, std::function<QString(int)> messageFn);

    /**
     * Manually updates the prefix/suffix of a spinbox with its current value,
     * in case signals are blocked for the spinbox while its value is being
     * changed.
     *
     * @param spinBox The QSpinBox to update.
     */
    KRITAWIDGETUTILS_EXPORT bool update(QSpinBox *spinBox);

    /**
     * Set the prefix/suffix of a `QSpinbox`-like widget using an i18n string
     * in the form of `prefix {n} suffix`. This is only done once immediately.
     * If plural handling is required, use `install` instead.
     *
     * @param spinbox The `QSpinBox` to set prefix/suffix on.
     * @param textTemplate The text in the form of `prefix{n}suffix`, usually
     *                     passed through `i18n` or `i18nc`.
     */
    KRITAWIDGETUTILS_EXPORT void setText(QSpinBox *spinBox, QStringView textTemplate);

    /**
     * Set the prefix/suffix of a `QDoubleSpinbox`-like widget using an i18n
     * string in the form of `prefix {n} suffix`. This is only done once
     * immediately.
     *
     * @param spinbox The `QDoubleSpinBox` to set prefix/suffix on.
     * @param textTemplate The text in the form of `prefix{n}suffix`, usually
     *                     passed through `i18n` or `i18nc`.
     */
    KRITAWIDGETUTILS_EXPORT void setText(QDoubleSpinBox *spinBox, QStringView textTemplate);

    /**
    * **Deleted overload** - KisSelectionPropertySlider contains special handling
    * to switch its prefix/suffix internally. Do not use `KisSpinBoxI18nHelper::setText`
    * to directly set the prefix/suffix. Use `KisSelectionPropertySliderBase::setTextTemplates`
    * instead.
    */
    void setText(KisSelectionPropertySliderBase *spinBox, const QStringView textTemplate) = delete;

} /* namespace KisSpinBoxI18nHelper */

#endif /* KIS_SPIN_BOX_I18N_HELPER_H */
