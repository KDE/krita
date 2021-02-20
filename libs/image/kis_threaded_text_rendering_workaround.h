/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_THREADED_TEXT_RENDERING_WORKAROUND_H
#define __KIS_THREADED_TEXT_RENDERING_WORKAROUND_H

/**
 * There is a bug in Qt/X11 which prevents the QPainter::drawText() call
 * be used in any non-gui thread, even when
 * QFontDatabase::supportsThreadedFontRendering() returns true. It seems
 * like some function in the font rendering routine eats the X11 replies
 * which are awaited by the GUI thread, effectively making the GUI thread
 * to hang up. The hangup happens in xcb_wait_for_reply().
 *
 * This dirty workaround makes the text brush be initialized in the
 * GUI thread, saved to a global singleton and then fetched by the
 * threaded code in the paintop.  Yes, that is weird, but this is the
 * best thing we can do right now :(
 *
 * \see bug 330492
 */

#ifdef HAVE_X11
#define HAVE_THREADED_TEXT_RENDERING_WORKAROUND
#endif

#endif /* __KIS_THREADED_TEXT_RENDERING_WORKAROUND_H */
