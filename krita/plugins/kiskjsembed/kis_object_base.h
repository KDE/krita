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

#ifndef _KIS_KJSEMBED_OBJECTS_BINDINGS_H_
#define _KIS_KJSEMBED_OBJECTS_BINDINGS_H_

#include <list>
#include <kjsembed/jsproxy_imp.h>

namespace KJSEmbed {
	class KJSEmbedPart;
};


namespace Krita {
	namespace Plugins {
		namespace KisKJSEmbed {
			namespace Bindings {
				class FunctionBase;
				/**
				 * This class serves as a base class for however wants to bind a new class to
				 * KisKJSEmbed
				 */
				class ObjectFactoryBase : public KJSEmbed::JSProxyImp {
					typedef std::list<FunctionBase* > FunctionsList;
					typedef std::list<FunctionBase* >::iterator FunctionsList_it;
					public:
						ObjectFactoryBase(KJS::Object parent, QString name, KJSEmbed::KJSEmbedPart *part);
					public:
						/** Returns true if this object implements the construct function. */
						virtual bool implementsConstruct() const { return true; }
						/** Invokes the construct function. */
						virtual KJS::Object construct( KJS::ExecState *exec, const KJS::List &args ) =0;
					protected:
						KJSEmbed::KJSEmbedPart* part() { return m_part; };
					protected:
						/**
					 * This function bind the list of functions to the object. It uses the
					 * m_functionsList, it calls createBindings if there is no functions inside
					 * the list.
						 */
						void bind( KJS::Object );
						virtual void createBindings() =0;
						inline void addBinding( FunctionBase* fn ) { m_functionsList.insert( m_functionsList.begin(), fn ); };
					private:
						KJSEmbed::KJSEmbedPart* m_part;
						FunctionsList m_functionsList;
				};
			};
		};
	};
};

#endif
