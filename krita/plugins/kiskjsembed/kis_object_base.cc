 /* This file is part of the KDE project
   Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

#include "kis_object_base.h"
#include "kis_function_base.h"

#include <kjsembed/kjsembedpart.h>

namespace Krita {
namespace Plugins {
namespace KisKJSEmbed {
namespace Bindings {

ObjectFactoryBase::ObjectFactoryBase(KJS::Object parent, QString name, KJSEmbed::KJSEmbedPart *part) : JSProxyImp(part->globalExec()), m_part(part)
{
	parent.put( part->globalExec() , KJS::Identifier(name), KJS::Object(this) );
	setName( KJS::Identifier( name ) );
}

void ObjectFactoryBase::bind( KJS::Object obj)
{
	if( m_functionsList.empty() )
	{
		createBindings();
	}
	FunctionsList_it i = m_functionsList.begin();
	while (i != m_functionsList.end())
	{
		FunctionBase* fb = *i;
		obj.put( part()->globalExec() , KJS::Identifier(fb->name()), KJS::Object(fb) );
		++i;
	}
}

}; }; }; };
