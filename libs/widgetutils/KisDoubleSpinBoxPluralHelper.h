/*
 * SPDX-FileCopyrightText: 2021 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KIS_DOUBLE_SPIN_BOX_PLURAL_HELPER_H
#define KIS_DOUBLE_SPIN_BOX_PLURAL_HELPER_H

#include <functional>

#include "kritawidgetutils_export.h"

class QDoubleSpinBox;
class QString;

namespace KisDoubleSpinBoxPluralHelper
{
    /**
     * Handles pluralization of prefix/suffix of `QSpinBox`-like widgets using
     * an i18n string in the form of `prefix {n} suffix`. This uses the
     * `valueChanged` signal to automatically update the text.
     *
     * In case the `valueChanged` signal wouldn't be emitted (i.e. signals
     * are blocked), call `KisDoubleSpinBoxPluralHelper::update` to update the text.
     *
     * @param spinBox The QSpinBox to handle.
     * @param messageFn A function (usually a lambda expression) that receives
     *                  the current value to be shown and returns a localized
     *                  `QString` in the form of `prefix {n} suffix`. The
     *                  prefix and suffix of `spinBox` will be set accordingly.
     */
    KRITAWIDGETUTILS_EXPORT void install(QDoubleSpinBox *spinBox, std::function<QString(double)> messageFn);

    /**
     * Manually updates the prefix/suffix of a spinbox with its current value,
     * in case signals are blocked for the spinbox while its value is being
     * changed.
     *
     * @param spinBox The QSpinBox to update.
     */
    KRITAWIDGETUTILS_EXPORT bool update(QDoubleSpinBox *spinBox);

} /* namespace KisDoubleSpinBoxPluralHelper */

#endif /* KIS_DOUBLE_SPIN_BOX_PLURAL_HELPER_H */
