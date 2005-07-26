/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include "kis_functions_factory.h"

// KJSEmbed
#include <kjsembed/kjsembedpart.h>

// Include of functions bindings
#include "kis_mainwindow_functions.h"

namespace Krita {
namespace Plugins {
namespace KisKJSEmbed {
namespace Bindings {

using namespace Bindings::Functions;

FunctionsFactory::FunctionsFactory( KJSEmbed::KJSEmbedPart *part, KisView* view )
{
	KJS::ExecState *exec = part->globalExec();
	// Functions concerning KisView
	m_jsObjMainWindow = exec->interpreter()->builtinObject().construct( exec, KJS::List() );
	new RaiseFunction( part, m_jsObjMainWindow, view );
	new LowerFunction( part, m_jsObjMainWindow, view );
	new CloseFunction( part, m_jsObjMainWindow, view );
	new QuitFunction( part, m_jsObjMainWindow, view );

	exec->interpreter()->globalObject().put( exec, "MainWindow", m_jsObjMainWindow);
}

}; }; }; };
