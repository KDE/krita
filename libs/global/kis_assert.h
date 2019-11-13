/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_ASSERT_H
#define __KIS_ASSERT_H

#include <QtGlobal>
#include <kritaglobal_export.h>

KRITAGLOBAL_EXPORT void kis_assert_exception(const char *assertion, const char *file, int line);
KRITAGLOBAL_EXPORT void kis_assert_recoverable(const char *assertion, const char *file, int line);
KRITAGLOBAL_EXPORT void kis_assert_x_exception(const char *assertion, const char *where, const char *what, const char *file, int line);

KRITAGLOBAL_EXPORT void kis_safe_assert_recoverable(const char *assertion, const char *file, int line);


/**
 * KIS_ASSERT family of macros allows the user to choose whether to
 * try to continue working in Krita or to abort an application and see
 * a backtrace.
 *
 * Note, the macro are present in Release mode by default!
 */

/**
 * Checks the condition and depending on the user action either aborts
 * the program or throws an exception, which restarts event loop.
 */
#define KIS_ASSERT(cond) ((!(cond)) ? kis_assert_exception(#cond,__FILE__,__LINE__) : qt_noop())

/**
 * Same as KIS_ASSERT, but allows to show more text to the user.
 *
 * \see KIS_ASSERT
 */
#define KIS_ASSERT_X(cond, where, what) ((!(cond)) ? kis_assert_x_exception(#cond,where, what,__FILE__,__LINE__) : qt_noop())


/**
 * This is a recoverable variant of KIS_ASSERT. It doesn't throw any
 * exceptions.  It checks the condition, and either aborts the
 * application, or executes user-supplied code. The typical usecase is
 * the following:
 *
 * int fooBar = ...;
 * KIS_ASSERT_RECOVER (fooBar > 0) {
 *     // the code which is executed in a case of emergency
 * }
 *
 */
#define KIS_ASSERT_RECOVER(cond) if (!(cond) && (kis_assert_recoverable(#cond,__FILE__,__LINE__), true))

/**
 * Equivalent of the following:
 *
 * KIS_ASSERT_RECOVER(cond) {
 *     break;
 * }
 *
 */
#define KIS_ASSERT_RECOVER_BREAK(cond) do { KIS_ASSERT_RECOVER(cond) { break; } } while (0)

/**
 * Equivalent of the following:
 *
 * KIS_ASSERT_RECOVER(cond) {
 *     return;
 * }
 *
 */
#define KIS_ASSERT_RECOVER_RETURN(cond) do { KIS_ASSERT_RECOVER(cond) { return; } } while (0)

/**
 * Equivalent of the following:
 *
 * KIS_ASSERT_RECOVER(cond) {
 *     return val;
 * }
 *
 */
#define KIS_ASSERT_RECOVER_RETURN_VALUE(cond, val) do { KIS_ASSERT_RECOVER(cond) { return (val); } } while (0)

/**
 * Does nothing in case of a failure. Just continues execution.
 *
 * Equivalent of the following:
 *
 * KIS_ASSERT_RECOVER(cond) {
 *     qt_noop();
 * }
 *
 */
#define KIS_ASSERT_RECOVER_NOOP(cond) do { KIS_ASSERT_RECOVER(cond) { qt_noop(); } } while (0)


/**
 * This set of macros work in exactly the same way as their non-safe
 * counterparts, but they are automatically converted into console
 * warnings in release builds. That is the user will not see any
 * message box, just a warning message will be printed in a terminal
 * and a recovery branch will be taken automatically.
 *
 * Rules when to use "safe" asserts. Use them if the following
 * conditions are met:
 *
 * 1) The condition in the assert shows that a real *bug* has
 *    happened. It is not just a warning. It is a bug that should be
 *    fixed.
 *
 * 2) The recovery branch will *workaround* the bug and the user will
 *    be able to continue his work *as if nothing has
 *    happened*. Again, please mark the assert "safe" if and only if
 *    you are 100% sure Krita will not crash in a minute after you
 *    faced that bug. The user is not notified about this bug, so he
 *    is not going to take any emergency steps like saving his work
 *    and restarting Krita!
 *
 * 3) If you think that Krita should be better restarted after this
 *    bug, please use a usual KIS_ASSERT_RECOVER.
 */

#define KIS_SAFE_ASSERT_RECOVER(cond) if (!(cond) && (kis_safe_assert_recoverable(#cond,__FILE__,__LINE__), true))
#define KIS_SAFE_ASSERT_RECOVER_BREAK(cond) do { KIS_SAFE_ASSERT_RECOVER(cond) { break; } } while (0)
#define KIS_SAFE_ASSERT_RECOVER_RETURN(cond) do { KIS_SAFE_ASSERT_RECOVER(cond) { return; } } while (0)
#define KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(cond, val) do { KIS_SAFE_ASSERT_RECOVER(cond) { return (val); } } while (0)
#define KIS_SAFE_ASSERT_RECOVER_NOOP(cond) do { KIS_SAFE_ASSERT_RECOVER(cond) { qt_noop(); } } while (0)

#endif /* __KIS_ASSERT_H */
