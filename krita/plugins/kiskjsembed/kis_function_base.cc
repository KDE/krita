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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "kis_function_base.h"

#include <kjsembed/kjsembedpart.h>

namespace Krita {
namespace Plugins {
namespace KisKJSEmbed {
namespace Bindings {

FunctionBase::FunctionBase(KJSEmbed::KJSEmbedPart *part, QString name, KJS::Object parent) : JSProxyImp(part->globalExec()), m_part(part)
{
	parent.put( part->globalExec() , KJS::Identifier(name), KJS::Object(this) );
	setName( KJS::Identifier( name ) );
}

FunctionBase::FunctionBase(KJSEmbed::KJSEmbedPart *part, QString name) : JSProxyImp(part->globalExec()), m_part(part)
{
	setName( KJS::Identifier( name ) );
}

GlobalFunctionBase::GlobalFunctionBase(KJSEmbed::KJSEmbedPart *part, QString name, KJS::Object parent, KisView* view ) : FunctionBase( part, name, parent), m_view(view)
{
}

}; }; }; };
