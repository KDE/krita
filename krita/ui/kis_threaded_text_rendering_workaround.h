/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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
