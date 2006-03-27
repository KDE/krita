/*
 *  koImageResource.cc - part of KOffice
 *
 *  Copyright (c) 2005 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation;version 2.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoImageResource.h"

namespace {
    /* XPM -- copyright The Gimp */
    const char *chain_broken_24[] = {
        /* columns rows colors chars-per-pixel */
        "9 24 10 1",
        "  c black",
        ". c #020204",
        "X c #5A5A5C",
        "o c gray43",
        "O c #8F8F91",
        "+ c #9A9A98",
        "@ c #B5B5B6",
        "# c #D0D0D1",
        "$ c #E8E8E9",
        "% c None",
        /* pixels */
        "%%.....%%",
        "%.o##@X.%",
        "%.+...$.%",
        "%.#.%.#.%",
        "%.#.%.#.%",
        "%.@.%.#.%",
        "%.+...#.%",
        "%.O.o.O.%",
        "%%..@..%%",
        "%%%.#.%%%",
        "%%%%%%%%%",
        "%%%%%%%%%",
        "%%%%%%%%%",
        "%%%%%%%%%",
        "%%%.#.%%%",
        "%%..#..%%",
        "%.o.@.O.%",
        "%.@...@.%",
        "%.@.%.$.%",
        "%.@.%.$.%",
        "%.@.%.$.%",
        "%.#...$.%",
        "%.o$#$@.%",
        "%%.....%%"
    };

    /* XPM  -- copyright The Gimp */
    const char *chain_24[] = {
        /* columns rows colors chars-per-pixel */
        "9 24 10 1",
        "  c black",
        ". c #020204",
        "X c #5A5A5C",
        "o c gray43",
        "O c #8F8F91",
        "+ c #9A9A98",
        "@ c #B5B5B6",
        "# c #D0D0D1",
        "$ c #E8E8E9",
        "% c None",
        /* pixels */
        "%%%%%%%%%",
        "%%%%%%%%%",
        "%%.....%%",
        "%.o##@X.%",
        "%.+...$.%",
        "%.#.%.#.%",
        "%.#.%.#.%",
        "%.@.%.#.%",
        "%.+...#.%",
        "%.O.o.O.%",
        "%%..@..%%",
        "%%%.#.%%%",
        "%%%.#.%%%",
        "%%..#..%%",
        "%.o.@.O.%",
        "%.@...@.%",
        "%.@.%.$.%",
        "%.@.%.$.%",
        "%.@.%.$.%",
        "%.#...$.%",
        "%.o$#$@.%",
        "%%.....%%",
        "%%%%%%%%%",
        "%%%%%%%%%"
    };
}

KoImageResource::KoImageResource() {}

const char** KoImageResource::chain()
{
    return chain_24;
}

const char** KoImageResource::chainBroken()
{
    return chain_broken_24;
}
