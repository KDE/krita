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

#ifndef _KIS_KJSEMBED_MAINWINDOW_FUNCTIONS_H_
#define _KIS_KJSEMBED_MAINWINDOW_FUNCTIONS_H_

#include "kis_function_base.h"

namespace Krita {
	namespace Plugins {
		namespace KisKJSEmbed {
			namespace Bindings {
				namespace Functions {
					class RaiseFunction : public GlobalFunctionBase
					{
						public:
							RaiseFunction(KJSEmbed::KJSEmbedPart *part, KJS::Object parent, KisView* view);
						public:
							virtual KJS::Value call( KJS::ExecState *exec, KJS::Object &self, const KJS::List &args );
					};
					
					class LowerFunction : public GlobalFunctionBase
					{
						public:
							LowerFunction(KJSEmbed::KJSEmbedPart *part, KJS::Object parent, KisView* view);
						public:
							virtual KJS::Value call( KJS::ExecState *exec, KJS::Object &self, const KJS::List &args );
					};
					
					class CloseFunction : public GlobalFunctionBase
					{
						public:
							CloseFunction(KJSEmbed::KJSEmbedPart *part, KJS::Object parent, KisView* view);
						public:
							virtual KJS::Value call( KJS::ExecState *exec, KJS::Object &self, const KJS::List &args );
					};
					
					class QuitFunction : public GlobalFunctionBase
					{
						public:
							QuitFunction(KJSEmbed::KJSEmbedPart *part, KJS::Object parent, KisView* view);
						public:
							virtual KJS::Value call( KJS::ExecState *exec, KJS::Object &self, const KJS::List &args );
					};
				};
			};
		};
	};
};

#endif
