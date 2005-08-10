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
