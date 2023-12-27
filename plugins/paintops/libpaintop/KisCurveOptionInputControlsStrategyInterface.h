/*
 * SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCURVEOPTIONINPUTCONTROLSSTRATEGYINTERFACE_H
#define KISCURVEOPTIONINPUTCONTROLSSTRATEGYINTERFACE_H

#include <kritapaintop_export.h>
#include <functional>

class KisCurveRangeModelInterface;
class KisCurveWidget;
class QWidget;

class PAINTOP_EXPORT KisCurveOptionInputControlsStrategyInterface
{
public:
    virtual ~KisCurveOptionInputControlsStrategyInterface();

    /**
     * erm... yep! no public interface! :)
     */
};

using KisCurveOptionInputControlsStrategyFactory =
    std::function<
        KisCurveOptionInputControlsStrategyInterface* (
            KisCurveRangeModelInterface* /*rangeInterface*/,
            KisCurveWidget* /*curveWidget*/,
            QWidget* /*inPlaceholder*/,
            QWidget* /*outPlaceholder*/)>;

#endif // KISCURVEOPTIONINPUTCONTROLSSTRATEGYINTERFACE_H
