/* 
 * This file is part of the KDE project
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
*/

#ifndef _KIS_KJSEMBED_FUNCTIONS_BINDINGS_H_
#define _KIS_KJSEMBED_FUNCTIONS_BINDINGS_H_

#include <kjsembed/jsproxy_imp.h>

namespace KJSEmbed {
	class KJSEmbedPart;
};

class KisView;

namespace Krita {
	namespace Plugins {
		namespace KisKJSEmbed {
			namespace Bindings {
				/** 
				 * This is the base class used to create a function for KisKJSEmbed.
				 * To add a new function, you need to derivate FunctionBase, then to implement
				 * its function call, then to instantiate FunctionBase from FunctionsFactory.
				 * This class is also used to add the functions to an object.
				 */
				class FunctionBase : public KJSEmbed::JSProxyImp {
					public:
						/**
						 * This function is used when a Function is attached to a single object
						 */
						FunctionBase(KJSEmbed::KJSEmbedPart *part, QString name, KJS::Object parent);
						/**
						 * This function is used for the functions of an object, because in such case,
						 * different objects share the same class
						 */
						FunctionBase(KJSEmbed::KJSEmbedPart *part, QString name);
					public:
						/** Returns true if this object implements the call function. */
						virtual bool implementsCall() const { return true; }
					
						/** Invokes the call function. */
						virtual KJS::Value call( KJS::ExecState *exec, KJS::Object &self, const KJS::List &args ) =0;
					private:
						KJSEmbed::KJSEmbedPart* m_part;
				};
				/**
				 * This class is used when you want to bind a function which needs to access to
				 * KisView.
				 */
				class GlobalFunctionBase : public FunctionBase {
					public:
						GlobalFunctionBase(KJSEmbed::KJSEmbedPart *part, QString name, KJS::Object parent, KisView* view );
					protected:
						KisView* view() { return m_view; };
					private:
						KisView* m_view;
				};
			};
		};
	};
};

#endif
