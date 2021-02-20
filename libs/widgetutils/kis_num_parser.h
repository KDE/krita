/*
 *  SPDX-FileCopyrightText: 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_NUMPARSER_H
#define KIS_NUMPARSER_H

#include <QString>

#include "kritawidgetutils_export.h"

/*!
 * \brief the namespace contains functions to transform math expression written as QString in numbers.
 *
 * Computation is done in a recursive way, maybe not the most efficient way compared to infix to postfix conversion before parsing.
 * (TODO: look if it need to be changed).
 */
namespace KisNumericParser {

    //! \brief parse an expression to a double.
    KRITAWIDGETUTILS_EXPORT double parseSimpleMathExpr(QString const& expr, bool* noProblem = 0);

    //! \brief parse an expression to an int.
    KRITAWIDGETUTILS_EXPORT int parseIntegerMathExpr(QString const& expr, bool* noProblem = 0);
}

#endif // KIS_NUMPARSER_H

