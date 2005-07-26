/*
 * This file is part of Krita
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

#include "kis_mainwindow_functions.h"

#include <qstring.h>

#include "kis_view.h"
#include "koMainWindow.h"

namespace Krita {
namespace Plugins {
namespace KisKJSEmbed {
namespace Bindings {
namespace Functions {
RaiseFunction::RaiseFunction(KJSEmbed::KJSEmbedPart *part, KJS::Object parent, KisView* view) : GlobalFunctionBase( part, QString("raise"), parent, view)
{

}

KJS::Value RaiseFunction::call( KJS::ExecState*, KJS::Object&, const KJS::List& )
{
	view()->mainWindow()->raise();
	return KJS::Null();
}

LowerFunction::LowerFunction(KJSEmbed::KJSEmbedPart *part, KJS::Object parent, KisView* view) : GlobalFunctionBase( part, QString("lower"), parent, view)
{

}

KJS::Value LowerFunction::call( KJS::ExecState*, KJS::Object&, const KJS::List& )
{
	view()->mainWindow()->lower();
	return KJS::Null();
}

CloseFunction::CloseFunction(KJSEmbed::KJSEmbedPart *part, KJS::Object parent, KisView* view) : GlobalFunctionBase( part, QString("close"), parent, view)
{

}

KJS::Value CloseFunction::call( KJS::ExecState*, KJS::Object&, const KJS::List& )
{
	view()->shell()->slotFileClose();
	return KJS::Null();
}

QuitFunction::QuitFunction(KJSEmbed::KJSEmbedPart *part, KJS::Object parent, KisView* view) : GlobalFunctionBase( part, QString("quit"), parent, view)
{

}

KJS::Value QuitFunction::call( KJS::ExecState*, KJS::Object&, const KJS::List& )
{
	view()->shell()->slotFileQuit();
	return KJS::Null();
}

}; }; }; }; };
