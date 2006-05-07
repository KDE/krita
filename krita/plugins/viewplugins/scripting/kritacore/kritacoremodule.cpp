/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kritacoremodule.h"

#include <kdebug.h>

//#include <api/variant.h>
#include <api/qtobject.h>
#include <main/manager.h>

#include <kis_autobrush_resource.h>
#include <kis_brush.h>
#include <kis_colorspace_factory_registry.h>
#include <kis_doc.h>
#include <kis_filter.h>
#include <kis_filter_registry.h>
#include <kis_image.h>
#include <kis_meta_registry.h>
#include <kis_pattern.h>
#include <kis_resourceserver.h>

#include "kis_script_progress.h"

#include "krs_brush.h"
#include "krs_color.h"
#include "krs_doc.h"
#include "krs_filter.h"
#include "krs_image.h"
#include "krs_pattern.h"
#include "krs_script_progress.h"
//Added by qt3to4:
#include <Q3ValueList>

extern "C"
{
    /**
     * Exported an loadable function as entry point to use
     * the \a KexiAppModule.
     */
    Kross::Api::Object* KDE_EXPORT init_module(Kross::Api::Manager* manager)
    {
        return new Kross::KritaCore::KritaCoreModule(manager);
    }
}


using namespace Kross::KritaCore;

KritaCoreFactory::KritaCoreFactory(QString packagePath) : Kross::Api::Event<KritaCoreFactory>("KritaCoreFactory", 0), m_packagePath(packagePath)
{
    addFunction("newRGBColor", &KritaCoreFactory::newRGBColor, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::UInt") << Kross::Api::Argument("Kross::Api::Variant::UInt") << Kross::Api::Argument("Kross::Api::Variant::UInt") );
    addFunction("newHSVColor", &KritaCoreFactory::newHSVColor, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::UInt") << Kross::Api::Argument("Kross::Api::Variant::UInt") << Kross::Api::Argument("Kross::Api::Variant::UInt") );
    addFunction("getPattern", &KritaCoreFactory::getPattern, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::String") );
    addFunction("loadPattern", &KritaCoreFactory::loadPattern);
    addFunction("getBrush", &KritaCoreFactory::getBrush, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::String") );
    addFunction("loadBrush", &KritaCoreFactory::loadBrush);
    addFunction("getFilter", &KritaCoreFactory::getFilter, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::String") );
    addFunction("newCircleBrush", &KritaCoreFactory::newCircleBrush, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") );
    addFunction("newRectBrush", &KritaCoreFactory::newRectBrush, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") );
    addFunction("newImage", &KritaCoreFactory::newImage, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant::String") << Kross::Api::Argument("Kross::Api::Variant::String") );
    addFunction("getPackagePath", &KritaCoreFactory::getPackagePath);
}

Kross::Api::Object::Ptr KritaCoreFactory::newRGBColor(Kross::Api::List::Ptr args)
{
    Color* c = new Color(Kross::Api::Variant::toUInt(args->item(0)), Kross::Api::Variant::toUInt(args->item(1)), Kross::Api::Variant::toUInt(args->item(2)), QColor::Rgb);
    return Kross::Api::Object::Ptr(c);
}
Kross::Api::Object::Ptr KritaCoreFactory::newHSVColor(Kross::Api::List::Ptr args)
{
    return Kross::Api::Object::Ptr(new Color(Kross::Api::Variant::toUInt(args->item(0)), Kross::Api::Variant::toUInt(args->item(1)), Kross::Api::Variant::toUInt(args->item(2)), QColor::Hsv));
}

Kross::Api::Object::Ptr KritaCoreFactory::getPattern(Kross::Api::List::Ptr args)
{
    KisResourceServerBase* rServer = KisResourceServerRegistry::instance()->get("PatternServer");
    Q3ValueList<KisResource*> resources = rServer->resources();

    QString name = Kross::Api::Variant::toString(args->item(0));

    for (Q3ValueList<KisResource*>::iterator it = resources.begin(); it != resources.end(); ++it )
    {
        if((*it)->name() == name)
        {
            return Kross::Api::Object::Ptr(new Pattern(dynamic_cast<KisPattern*>(*it), true));
        }
    }
    throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(  i18n("Unknown pattern") ) );
    return Kross::Api::Object::Ptr(0);

}

Kross::Api::Object::Ptr KritaCoreFactory::loadPattern(Kross::Api::List::Ptr args)
{
    QString filename = Kross::Api::Variant::toString(args->item(0));
    KisPattern* pattern = new KisPattern(filename);
    if(pattern->load())
    {
        return Kross::Api::Object::Ptr(new Pattern( pattern, false ));
    } else {
        delete pattern;
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception( i18n("Unknown pattern") ) );
        return Kross::Api::Object::Ptr(0);
    }
}

Kross::Api::Object::Ptr KritaCoreFactory::getBrush(Kross::Api::List::Ptr args)
{
    KisResourceServerBase* rServer = KisResourceServerRegistry::instance()->get("BrushServer");
    Q3ValueList<KisResource*> resources = rServer->resources();

    QString name = Kross::Api::Variant::toString(args->item(0));

    for (Q3ValueList<KisResource*>::iterator it = resources.begin(); it != resources.end(); ++it )
    {
        if((*it)->name() == name)
        {
            return Kross::Api::Object::Ptr(new Brush(dynamic_cast<KisBrush*>(*it), true));
        }
    }
    throw Kross::Api::Exception::Ptr( new Kross::Api::Exception( i18n("Unknown brush") ) );
    return Kross::Api::Object::Ptr(0);
}

Kross::Api::Object::Ptr KritaCoreFactory::loadBrush(Kross::Api::List::Ptr args)
{
    QString filename = Kross::Api::Variant::toString(args->item(0));
    KisBrush* brush = new KisBrush(filename);
    if(brush->load())
    {
        return Kross::Api::Object::Ptr(new Brush( brush, false ));
    } else {
        delete brush;
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception( i18n("Unknown brush") ) );
        return Kross::Api::Object::Ptr(0);
    }
}

Kross::Api::Object::Ptr KritaCoreFactory::getFilter(Kross::Api::List::Ptr args)
{
    QString name = Kross::Api::Variant::toString(args->item(0));
    KisFilter* filter = KisFilterRegistry::instance()->get(name).data();
    return Kross::Api::Object::Ptr(new Filter(filter));
}

Kross::Api::Object::Ptr KritaCoreFactory::newCircleBrush(Kross::Api::List::Ptr args)
{
    uint w = qMax(1u, Kross::Api::Variant::toUInt(args->item(0)));
    uint h = qMax(1u, Kross::Api::Variant::toUInt(args->item(1)));
    uint hf = 0;
    uint vf = 0;
    if( args.count() > 2)
    {
        hf = Kross::Api::Variant::toUInt(args->item(2));
        vf = Kross::Api::Variant::toUInt(args->item(3));
    }
    KisAutobrushShape* kas = new KisAutobrushCircleShape(w, h, hf, vf);
    QImage* brsh = new QImage();
    kas->createBrush(brsh);
    return Kross::Api::Object::Ptr(new Brush(new KisAutobrushResource(*brsh), false));
}
Kross::Api::Object::Ptr KritaCoreFactory::newRectBrush(Kross::Api::List::Ptr args)
{
    uint w = qMax(1u, Kross::Api::Variant::toUInt(args->item(0)));
    uint h = qMax(1u, Kross::Api::Variant::toUInt(args->item(1)));
    uint hf = 0;
    uint vf = 0;
    if( args.count() > 2)
    {
        hf = Kross::Api::Variant::toUInt(args->item(2));
        vf = Kross::Api::Variant::toUInt(args->item(3));
    }
    KisAutobrushShape* kas = new KisAutobrushRectShape(w, h, hf, vf);
    QImage* brsh = new QImage();
    kas->createBrush(brsh);
    return Kross::Api::Object::Ptr(new Brush(new KisAutobrushResource(*brsh), false));
}

Kross::Api::Object::Ptr KritaCoreFactory::newImage(Kross::Api::List::Ptr args)
{
    int w = Kross::Api::Variant::toInt(args->item(0));
    int h = Kross::Api::Variant::toInt(args->item(1));
    QString csname = Kross::Api::Variant::toString(args->item(2));
    QString name = Kross::Api::Variant::toString(args->item(3));
    if( w < 0 || h < 0)
    {
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception( i18n("Invalid image size") ) );
        return Kross::Api::Object::Ptr(0);
    }
    KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID(csname, ""), "");
    if(!cs)
    {
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception( i18n("Colorspace %1 is not available, please check your installation.").arg(csname ) ) );
        return Kross::Api::Object::Ptr(0);
    }

    return Kross::Api::Object::Ptr(new Image(KisImageSP(new KisImage(0,w,h, cs, name))));
}

Kross::Api::Object::Ptr KritaCoreFactory::getPackagePath(Kross::Api::List::Ptr)
{
    return Kross::Api::Object::Ptr(new Kross::Api::Variant(m_packagePath));
}

KritaCoreModule::KritaCoreModule(Kross::Api::Manager* manager)
    : Kross::Api::Module("kritacore") , m_manager(manager), m_factory(0)
{

    QMap<QString, Object::Ptr> children = manager->getChildren();
    kDebug(41011) << " there are " << children.size() << endl;
    for(QMap<QString, Object::Ptr>::const_iterator it = children.begin(); it != children.end(); it++)
    {
        kDebug(41011) << it.key() << " " << it.value() << endl;
    }

    // Wrap doc
    Kross::Api::Object::Ptr kritadocument = ((Kross::Api::Object*)manager)->getChild("KritaDocument");
    if(kritadocument) {
        Kross::Api::QtObject* kritadocumentqt = (Kross::Api::QtObject*)( kritadocument.data() );
        if(kritadocumentqt) {
            ::KisDoc* document = (::KisDoc*)( kritadocumentqt->getObject() );
            if(document) {
                addChild( new Doc(document) );
            } else {
                throw Kross::Api::Exception::Ptr( new Kross::Api::Exception("There was no 'KritaDocument' published.") );
            }
         }
    }
   // Wrap KritaScriptProgress
    QString packagePath;
    Kross::Api::Object::Ptr kritascriptprogress = ((Kross::Api::Object*)manager)->getChild("KritaScriptProgress");
    if(kritadocument) {
        Kross::Api::QtObject* kritascriptprogressqt = (Kross::Api::QtObject*)( kritascriptprogress.data() );
        if(kritascriptprogressqt) {
                ::KisScriptProgress* scriptprogress = (::KisScriptProgress*)( kritascriptprogressqt->getObject() );
                scriptprogress->activateAsSubject();
                packagePath = scriptprogress->packagePath();
                if(scriptprogress) {
                    addChild( new ScriptProgress(scriptprogress) );
                } else {
                    throw Kross::Api::Exception::Ptr( new Kross::Api::Exception("There was no 'KritaScriptProgress' published.") );
                }
        }
    }
    m_factory = new KritaCoreFactory(packagePath);
}

KritaCoreModule::~KritaCoreModule()
{
    if(m_factory)
        delete m_factory;
}


const QString KritaCoreModule::getClassName() const
{
    return "Kross::KritaCore::KritaCoreModule";
}

Kross::Api::Object::Ptr KritaCoreModule::call(const QString& name, Kross::Api::List::Ptr arguments)
{
    kDebug(41011) << "KritaCoreModule::call = " << name << endl;
    if( m_factory->isAFunction(name))
    {
        return m_factory->call(name, arguments);
    } else {
        return Kross::Api::Module::call(name, arguments);
    }
}
