/*
 * This file is part of Krita
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

#include "kis_paint_device_object.h"

#include <qstring.h>
#include <kdebug.h>
#include <klocale.h>
#include <qrect.h>

// kjs
#include <kjs/interpreter.h>

// kjsembed
#include <kjsembed/jsvalueproxy.h>
#include <kjsembed/jsbinding.h> // use to have UString::qstring()


// Krita
#include "core/kis_paint_device_impl.h"
#include "core/color_strategy/kis_colorspace_factory_registry.h"

namespace Krita {
namespace Plugins {
namespace KisKJSEmbed {
namespace Bindings {
namespace Objects {

using namespace Functions::PaintDevice;

PaintDeviceFactory::PaintDeviceFactory(KJS::Object parent,KJSEmbed::KJSEmbedPart *part) : ObjectFactoryBase( parent, QString("KisPaintDeviceImpl"),part)
{

}

KJS::Object PaintDeviceFactory::construct( KJS::ExecState *exec, const KJS::List &args )
{
    if( args.size() < 2 )
    {
        QString msg = i18n( "Method requires at least 2 arguments, received %1" ).arg( args.size() );
        KJS::Object err = KJS::Error::create( exec, KJS::GeneralError, msg.utf8() );
        exec->setException( err );
        return err;
    }
    int w = args[0].toInt32(exec);
    int h = args[1].toInt32(exec);
    QString cs;
    if( args.size() > 2 )
    {
        cs = args[2].toString(exec).qstring();
    } else {
        cs = "RGBA";
    }
    QString name = "unamed";
    if( args.size() > 3 )
    {
        name = args[3].toString(exec).qstring();
    }
    KJS::Object proxy( new PaintDeviceObject() );
    bind(proxy);
    return proxy;
}

void PaintDeviceFactory::createBindings()
{
    addBinding( new Width(part()) );
}

PaintDeviceObject::PaintDeviceObject()
{
    m_paintDevice = new KisPaintDeviceImpl(KisColorSpaceFactoryRegistry::instance()->getColorSpace( KisID("RGBA", ""), ""), "JSPaintDevice");
}

};
namespace Functions {
namespace PaintDevice {
Width::Width(KJSEmbed::KJSEmbedPart *part) : FunctionBase(part, "width") {
}
KJS::Value Width::call( KJS::ExecState *exec, KJS::Object &self, const KJS::List &args )
{
    Objects::PaintDeviceObject* pdo = Objects::PaintDeviceObject::toPaintDeviceObject( self.imp() );
    return KJS::Number( pdo->paintDevice()->extent().width() );
}

}; }; }; }; }; };
