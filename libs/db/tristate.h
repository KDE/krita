/* This file is part of the KDE project
   Copyright (C) 2004, 2006 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _TRISTATE_TYPE_H_
#define _TRISTATE_TYPE_H_

#include <QString>

/**
 * \e cancelled value, in most cases usable if there is a need for returning
 * \e cancelled value explicitly. Example use:
 * \code
 * tristate myFunctionThatCanBeCancelled() {
 *   doSomething();
 *   if (userCancelledOperation())
 *     return cancelled; //neither success or failure is returned here
 *   return operationSucceeded(); //return success or failure
 * }
 * \endcode
 * Even though ~ operator of tristate class can be used, it is also possible to test:
 * \code
 * if (cancelled == myFunctionThatCanBeCancelled()) { .... }
 * \endcode
 */
static const char cancelled = 2;

/**
 * Convenience name, the same as cancelled value.
 */
static const char dontKnow = cancelled;

/**
 * 3-state logical type with three values: \e true, \e false and \e cancelled and convenient operators.
 *
 * \e cancelled state can be also called \e dontKnow, it behaves as \e null in SQL.
 * A main goal of this class is to improve readibility when there's a need
 * for storing third, \e cancelled, state, especially in case C++ exceptions are not in use.
 * With it, developer can forget about declaring a specific enum type
 * having just three values: \e true, \e false, \e cancelled.
 *
 * Objects of this class can be used with similar convenience as standard bool type:
 * - use as return value when 'cancelled'
 *   \code
 *   tristate doSomething();
 *   \endcode
 * - convert from bool (1) or to bool (2)
 *   \code
 *   tristate t = true; //(1)
 *   setVisible(t);   //(2)
 *   \endcode
 * - clear comparisons
 *   \code
 *   tristate t = doSomething();
 *   if (t) doSomethingIfTrue();
 *   if (!t) doSomethingIfFalse();
 *   if (~t) doSomethingIfCancelled();
 *   \endcode
 *
 * "! ~" can be used as "not cancelled".
 *
 * With tristate class, developer can also forget about
 * it's additional meaning and treat it just as a bool, if the third state
 * is irrelevant to the current situation.
 *
 * Other use for tristate class could be to allow cancellation within
 * a callback function or a Qt slot. Example:
 * \code
 * public slots:
 *   void validateData(tristate& result);
 * \endcode
 * Having the single parameter, signals and slots have still simple look.
 * Developers can alter their code (by replacing 'bool& result' with 'tristate& result')
 * in case when a possibility of canceling of, say, data provessing needs to be implemented.
 * Let's say \e validateData() function uses a QDialog to get some validation from a user.
 * While QDialog::Rejected is returned after cancellation of the validation process,
 * the information about cancellation needs to be transferred up to a higher level of the program.
 * Storing values of type QDialog::DialogCode there could be found as unreadable, and
 * casting these to int is not typesafe. With tristate class it's easier to make it obvious that
 * cancellation should be taken into account.
 *
 * @author Jarosław Staniek
 */
class tristate
{
public:
    /**
     * Default constructor, object has \e cancelled value set.
     */
    tristate()
            : m_value(Cancelled) {
    }

    /**
     * Constructor accepting a boolean value.
     */
    tristate(bool boolValue)
            : m_value(boolValue ? True : False) {
    }

    /**
     * Constructor accepting a char value.
     * It is converted in the following way:
     * - 2 -> cancelled
     * - 1 -> true
     * - other -> false
     */
    tristate(char c)
            : m_value(c == cancelled ? tristate::Cancelled : (c == 1 ? True : False)) {
    }

    /** Constructor accepting an integer value.
     * It is converted in the following way:
     * - 2 -> cancelled
     * - 1 -> true
     * - other -> false
     */
    tristate(int intValue)
            : m_value(intValue == (int)cancelled ? tristate::Cancelled : (intValue == 1 ? True : False)) {
    }

    /**
     * Casting to bool type with negation: true is only returned
     * if the original tristate value is equal to false.
     */
    bool operator!() const {
        return m_value == False;
    }

    /**
     * Special casting to bool type: true is only returned
     * if the original tristate value is equal to \e cancelled.
     */
    bool operator~() const {
        return m_value == Cancelled;
    }

    tristate& operator=(const tristate& tsValue) {
        m_value = tsValue.m_value; return *this;
    }

    friend inline bool operator==(bool boolValue, tristate tsValue);

    friend inline bool operator==(tristate tsValue, bool boolValue);

    friend inline bool operator!=(bool boolValue, tristate tsValue);

    friend inline bool operator!=(tristate tsValue, bool boolValue);

    /**
     * \return text representation of the value: "true", "false" or "cancelled".
     */
    QString toString() const {
        if (m_value == False)
            return QString::fromLatin1("false");
        return m_value == True ? QString::fromLatin1("true") : QString::fromLatin1("cancelled");
    }

private:
    /**
     * @internal
     * States used internally.
     */
    enum Value {
        False = 0,
        True = 1,
        Cancelled = 2
    };

    /**
     * @internal
     */
    Value m_value;
};

/**
 * Inequality operator comparing a bool value @p boolValue and a tristate value @p tsValue.
 *
 * @return false if both @p boolValue and @p tsValue are true
 *         or if both  @p boolValue and @p tsValue are false.
 *         Else, returns true.
*/
inline bool operator!=(bool boolValue, tristate tsValue)
{
    return !((tsValue.m_value == tristate::True && boolValue)
             || (tsValue.m_value == tristate::False && !boolValue));
}

/**
 * Inequality operator comparing a tristate value @p tsValue and a bool value @p boolValue.
 * @see bool operator!=(bool boolValue, tristate tsValue)
*/
inline bool operator!=(tristate tsValue, bool boolValue)
{
    return !((tsValue.m_value == tristate::True && boolValue)
             || (tsValue.m_value == tristate::False && !boolValue));
}

/**
  * Equality operator comparing a tristate value @p tsValue and a bool value @p boolValue.
  * \return true if both
  * - both @p tsValue value and @p boolValue are true, or
  * - both @p tsValue value and @p boolValue are false
  * If the tristate value has value of cancelled, false is returned.
  */
inline bool operator==(tristate tsValue, bool boolValue)
{
    return (tsValue.m_value == tristate::True && boolValue)
           || (tsValue.m_value == tristate::False && !boolValue);
}

/**
  * Equality operator comparing a bool value @p boolValue and a tristate value @p tsValue.
  * \return true if both
  * - both @p tsValue value and @p boolValue are true, or
  * - both @p tsValue value and @p boolValue are false
  * If the tristate value has value of cancelled, false is returned.
  */
inline bool operator==(bool boolValue, tristate tsValue)
{
    return (tsValue.m_value == tristate::True && boolValue)
           || (tsValue.m_value == tristate::False && !boolValue);
}

#endif
