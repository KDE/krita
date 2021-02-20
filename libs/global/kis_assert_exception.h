/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ASSERT_EXCEPTION_H
#define __KIS_ASSERT_EXCEPTION_H

#include <stdexcept>
#include <QException>

class KisAssertException : public std::runtime_error, public QException
{
public:
    KisAssertException(const std::string& what_arg)
        : std::runtime_error(what_arg)
    {
    }

    QException* clone() const override { return new KisAssertException(*this); }
    void raise() const override { throw *this; }
};

#endif /* __KIS_ASSERT_EXCEPTION_H */
