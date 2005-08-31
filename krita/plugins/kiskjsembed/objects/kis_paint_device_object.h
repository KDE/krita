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

#ifndef _KIS_KJSEMBED_PAINT_DEVICE_OBJECT_H_
#define _KIS_KJSEMBED_PAINT_DEVICE_OBJECT_H_

#include "kis_object_base.h"
#include "kis_function_base.h"
#include <kjs/object.h>

class KisPaintDeviceImpl;

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
                     * This class is a proxy for a KisPaintDeviceImpl
                     */
                    class PaintDeviceObject : public KJS::ObjectImp {
                        public:
                            PaintDeviceObject();
                        public:
                            inline KisPaintDeviceImpl* paintDevice() { return m_paintDevice; };
                        public:
                            static PaintDeviceObject* toPaintDeviceObject(KJS::ObjectImp* imp) { return dynamic_cast<PaintDeviceObject*>(imp); }
                        private:
                            KisPaintDeviceImpl* m_paintDevice;
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
