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
#include "kritacoreprogress.h"
#include "krs_brush.h"
#include "krs_color.h"
#include "krs_filter.h"
#include "krs_image.h"
#include "krs_pattern.h"

// qt
//#include <Q3ValueList>

// kde
#include <kapplication.h>
#include <kdebug.h>

// koffice
#include <KoDocumentAdaptor.h>
#include <KoApplicationAdaptor.h>

// krita
#include <kis_view.h>
#include <kis_doc.h>
#include <kis_image.h>
#include <kis_autobrush_resource.h>
#include <kis_resourceserver.h>
#include <kis_brush.h>
#include <kis_pattern.h>
//#include <KoColorSpaceRegistry.h>
//#include <kis_doc.h>
//#include <kis_filter.h>
//#include <kis_filter_registry.h>
//#include <kis_image.h>
//#include <kis_meta_registry.h>

using namespace Kross::KritaCore;

namespace Kross { namespace KritaCore {

	class KritaCoreModule::Private
	{
		public:
			KisView* view;
			KritaCoreProgress* progress;

			Private(KisView* v) : view(v), progress(0) {}
			~Private() { delete progress; }
	};

}}

KritaCoreModule::KritaCoreModule(KisView* view)
	: QObject()
	, d(new Private(view))
{
	setObjectName("Krita");

	d->progress = new KritaCoreProgress(view);
#if 0
	Kross::Manager::self().addObject(d->view->canvasSubject()->document(), "KritaDocument");
	Kross::Manager::self().addObject(d->view, "KritaView");
#endif
}

KritaCoreModule::~KritaCoreModule()
{
	delete d;
}

QObject* KritaCoreModule::application()
{
	return KApplication::kApplication()->findChild< KoApplicationAdaptor* >();
}

#if 0
QObject* KritaCoreModule::document()
{
	return d->view->document() ? d->view->document()->findChild< KoDocumentAdaptor* >() : 0;
}
#endif

QObject* KritaCoreModule::progress()
{
    return d->progress;
}

QObject* KritaCoreModule::image()
{
    ::KisDoc* document = d->view->canvasSubject()->document();
    return document ? new Image(document->currentImage(), document) : 0;
}

QObject* KritaCoreModule::createRGBColor(int r, int g, int b)
{
    return new Color(r, g, b, QColor::Rgb);
}

QObject* KritaCoreModule::createHSVColor(int hue, int saturation, int value)
{
    return new Color(hue, saturation, value, QColor::Hsv);
}

QObject* KritaCoreModule::pattern(const QString& patternname)
{
    KisResourceServerBase* rServer = KisResourceServerRegistry::instance()->get("PatternServer");
    foreach(KisResource* res, rServer->resources())
        if(res->name() == patternname)
            return new Pattern(dynamic_cast<KisPattern*>(res), true);
    kWarning() << QString("Unknown pattern \"%1\"").arg(patternname) << endl;
    return 0;
}

QObject* KritaCoreModule::brush(const QString& brushname)
{
    KisResourceServerBase* rServer = KisResourceServerRegistry::instance()->get("BrushServer");
    foreach(KisResource* res, rServer->resources())
        if(res->name() == brushname)
            return new Brush(dynamic_cast<KisBrush*>(res), true);
    kWarning() << QString("Unknown brush \"%1\"").arg(brushname) << endl;
    return 0;
}

QObject* KritaCoreModule::createCircleBrush(uint w, uint h, uint hf, uint vf)
{
    KisAutobrushShape* kas = new KisAutobrushCircleShape(qMax(1u,w), qMax(1u,h), hf, vf);
    QImage* brsh = new QImage();
    kas->createBrush(brsh);
    return new Brush(new KisAutobrushResource(*brsh), false);
}

QObject* KritaCoreModule::createRectBrush(uint w, uint h, uint hf, uint vf)
{
    KisAutobrushShape* kas = new KisAutobrushRectShape(qMax(1u,w), qMax(1u,h), hf, vf);
    QImage* brsh = new QImage();
    kas->createBrush(brsh);
    return new Brush(new KisAutobrushResource(*brsh), false);
}

#if 0
extern "C"
{
    /**
     * Exported an loadable function as entry point to use
     * the \a KexiAppModule.
     */
    Kross::Api::Object* init_module(Kross::Api::Manager* manager)
    {
        return new Kross::KritaCore::KritaCoreModule(manager);
    }
}

Kross::Api::Object::Ptr KritaCoreModule::loadPattern(Kross::Api::List::Ptr args)
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

Kross::Api::Object::Ptr KritaCoreModule::loadBrush(Kross::Api::List::Ptr args)
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

Kross::Api::Object::Ptr KritaCoreModule::getFilter(Kross::Api::List::Ptr args)
{
    QString name = Kross::Api::Variant::toString(args->item(0));
    KisFilter* filter = KisFilterRegistry::instance()->get(name).data();
    return Kross::Api::Object::Ptr(new Filter(filter));
}

Kross::Api::Object::Ptr KritaCoreModule::newImage(Kross::Api::List::Ptr args)
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
    KoColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->colorSpace(csname, 0);
    if(!cs)
    {
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception( i18n("Colorspace %1 is not available, please check your installation.", csname ) ) );
        return Kross::Api::Object::Ptr(0);
    }

    return Kross::Api::Object::Ptr(new Image(KisImageSP(new KisImage(0,w,h, cs, name))));
}

Kross::Api::Object::Ptr KritaCoreModule::getPackagePath(Kross::Api::List::Ptr)
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
    Kross::Api::Object::Ptr kritadocument = manager->getChild("KritaDocument");
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
    Kross::Api::Object::Ptr kritascriptprogress = manager->getChild("KritaScriptProgress");
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
    m_factory = new KritaCoreModule(packagePath);
}

KritaCoreModule::~KritaCoreModule()
{
    if(m_factory)
        delete m_factory;
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
#endif

#include "kritacoremodule.moc"
