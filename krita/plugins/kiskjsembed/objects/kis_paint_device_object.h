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

#ifndef _KIS_KJSEMBED_PAINT_DEVICE_OBJECT_H_
#define _KIS_KJSEMBED_PAINT_DEVICE_OBJECT_H_

#include "kis_object_base.h"
#include "kis_function_base.h"
#include <kjs/object.h>

class KisPaintDevice;

namespace Krita {
	namespace Plugins {
		namespace KisKJSEmbed {
			namespace Bindings {
				namespace Objects {
					/**
					 * This class is used to construct a new PaintDevice for use in JSEmbed
					 */
					class PaintDeviceFactory : public ObjectFactoryBase {
						public:
							PaintDeviceFactory( KJS::Object parent, KJSEmbed::KJSEmbedPart *part);
						public:
							virtual KJS::Object construct( KJS::ExecState *exec, const KJS::List &args );
						private:
							virtual void createBindings();
					
					};
					/**
					 * This class is a proxy for a KisPaintDevice
					 */
					class PaintDeviceObject : public KJS::ObjectImp {
						public:
							PaintDeviceObject();
						public:
							inline KisPaintDevice* paintDevice() { return m_paintDevice; };
						public:
							static PaintDeviceObject* toPaintDeviceObject(KJS::ObjectImp* imp) { return dynamic_cast<PaintDeviceObject*>(imp); }
						private:
							KisPaintDevice* m_paintDevice;
					};
				};
				namespace Functions {
					namespace PaintDevice {
						class Width : public FunctionBase {
							public:
								Width(KJSEmbed::KJSEmbedPart *part);
							public:
								virtual KJS::Value call( KJS::ExecState *exec, KJS::Object &self, const KJS::List &args );
						};
					};
				};
			};
		};
	};
};

#endif
