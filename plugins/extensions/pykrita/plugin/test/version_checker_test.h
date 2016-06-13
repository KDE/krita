// This file is part of PyKrita, Krita' Python scripting plugin.
//
// Copyright (C) 2013 Alex Turbov <i.zaufi@gmail.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) version 3.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library; see the file COPYING.LIB.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.

#ifndef __VERSION_CHECKER_TEST_H__
# define  __VERSION_CHECKER_TEST_H__

#include <QtTest/QtTest>

class version_checker_tests : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void version_ctor_test();
    void version_ops_test();
    void version_string_test();

    void version_checker_test();
    void version_checker_string_test();
};

#endif                                                      //  __VERSION_CHECKER_TEST_H__
