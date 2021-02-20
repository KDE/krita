/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_FILL_SANITY_CHECKS_H
#define __KIS_FILL_SANITY_CHECKS_H

#define ENABLE_FILL_SANITY_CHECKS
//#define ENABLE_CHECKS_FOR_TESTING

#ifdef ENABLE_FILL_SANITY_CHECKS

#ifdef ENABLE_CHECKS_FOR_TESTING
#include <stdexcept>
#define SANITY_ASSERT_MSG(cond, msg) ((!(cond)) ? throw std::invalid_argument(msg) : qt_noop())
#else
#define SANITY_ASSERT_MSG(cond, msg) KIS_SAFE_ASSERT_RECOVER_NOOP((cond))
#endif /* ENABLE_CHECKS_FOR_TESTING */

#else

#define SANITY_ASSERT_MSG(cond, msg)

#endif /* ENABLE_FILL_SANITY_CHECKS */


#endif /* __KIS_FILL_SANITY_CHECKS_H */
