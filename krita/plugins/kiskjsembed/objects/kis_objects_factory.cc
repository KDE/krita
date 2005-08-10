/* This file is part of Krita
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

#include "kis_objects_factory.h"

// KJSEmbed
#include <kjsembed/kjsembedpart.h>

// Include of objects bindings
#include "kis_paint_device_object.h"

namespace Krita {
namespace Plugins {
namespace KisKJSEmbed {
namespace Bindings {

using namespace Objects;

ObjectsFactory::ObjectsFactory( KJSEmbed::KJSEmbedPart *part, KisView* )
{
    KJS::ExecState *exec = part->globalExec();

    new PaintDeviceFactory( part->globalObject() , part);
}

}; }; }; };
