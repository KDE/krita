/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISPROPAGATECONSTWRAPPER_H
#define KISPROPAGATECONSTWRAPPER_H

#if __has_include(<experimental/propagate_const>) && !defined( __cppcheck__)
#include <experimental/propagate_const>
#else
#include "3rdparty/propagate_const.h"
#endif

#endif // KISPROPAGATECONSTWRAPPER_H
