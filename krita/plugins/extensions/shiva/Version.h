/*
 * Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <OpenShiva/Version.h>
#include <kdeversion.h>


#define OPENSHIVA_12 KDE_MAKE_VERSION(OPENSHIVA_VERSION_MAJOR, OPENSHIVA_VERSION_MINOR, OPENSHIVA_VERSION_REVISION) == KDE_MAKE_VERSION(0, 9, 12)
#define OPENSHIVA_13_OR_MORE KDE_MAKE_VERSION(OPENSHIVA_VERSION_MAJOR, OPENSHIVA_VERSION_MINOR, OPENSHIVA_VERSION_REVISION) >= KDE_MAKE_VERSION(0, 9, 13)
