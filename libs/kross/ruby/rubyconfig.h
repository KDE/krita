/***************************************************************************
 * pythonconfig.h
 * This file is part of the KDE project
 * copyright (C)2005 by Cyrille Berger (cberger@cberger.net)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KROSS_RUBY_CONFIG_H
#define KROSS_RUBY_CONFIG_H

#include "../core/krossconfig.h"

namespace Kross {

/**
 * The Ruby plugin for the \a Kross scripting framework.
 *
 * @author Cyrille Berger
 * @sa http://www.ruby-lang.org
 */

 #define KROSS_RUBY_SCRIPT_DEBUG
 #define KROSS_RUBY_INTERPRETER_DEBUG
 #define KROSS_RUBY_EXTENSION_DEBUG
 #define KROSS_RUBY_MODULE_DEBUG

}

#endif

