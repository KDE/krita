/*
 *  Copyright (c) 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
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

