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

#include "krs_module.h"
#include "krs_progress.h"
#include "krs_brush.h"
#include "krs_color.h"
#include "krs_filter.h"
#include "krs_image.h"
#include "krs_pattern.h"

#include <kdebug.h>

#include <QApplication>

// koffice
#include <KoDocumentAdaptor.h>
#include <KoApplicationAdaptor.h>
#include <KoColorSpaceRegistry.h>

// krita
#include <kis_view2.h>
#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_autobrush_resource.h>
#include <kis_resourceserver.h>
#include <kis_brush.h>
#include <kis_pattern.h>
#include <kis_filter.h>
#include <kis_filter_registry.h>
//#include <kis_image.h>
#include <kis_meta_registry.h>

#if 0
extern "C"
{
    /**
     * Exported an loadable function as entry point to use
     * the \a KexiAppModule.
     */
    Kross::Api::Object* init_module(Kross::Api::Manager* manager)
    {
        return new Scripting::Module(manager);
    }
}
#endif

using namespace Scripting;

namespace Scripting {

	/// \internal d-pointer class.
	class Module::Private
	{
		public:
			KisView2* view;
			Progress* progress;

			Private(KisView2* v) : view(v), progress(0) {}
			~Private() { delete progress; }
	};

}

Module::Module(KisView2* view)
	: QObject()
	, d(new Private(view))
{
	setObjectName("Krita");

#if 0
	Kross::Manager::self().addObject(d->view->canvasSubject()->document(), "KritaDocument");
	Kross::Manager::self().addObject(d->view, "KritaView");
#endif
}

Module::~Module()
{
	delete d;
}

QObject* Module::application()
{
	return qApp->findChild< KoApplicationAdaptor* >();
}

#if 0
QObject* Module::document()
{
	return d->view->document() ? d->view->document()->findChild< KoDocumentAdaptor* >() : 0;
}
#endif

QObject* Module::progress()
{
    if(! d->progress)
        d->progress = new Progress(d->view);
    return d->progress;
}

QObject* Module::image()
{
    ::KisDoc2* document = d->view->document();
    return document ? new Image(this, d->view->image(), document) : 0;
}

QObject* Module::createRGBColor(int r, int g, int b)
{
    return new Color(r, g, b, QColor::Rgb);
}

QObject* Module::createHSVColor(int hue, int saturation, int value)
{
    return new Color(hue, saturation, value, QColor::Hsv);
}

QObject* Module::pattern(const QString& patternname)
{
    KisResourceServerBase* rServer = KisResourceServerRegistry::instance()->get("PatternServer");
    foreach(KisResource* res, rServer->resources())
        if(res->name() == patternname)
            return new Pattern(this, dynamic_cast<KisPattern*>(res), true);
    kWarning() << QString("Unknown pattern \"%1\"").arg(patternname) << endl;
    return 0;
}

QObject* Module::brush(const QString& brushname)
{
    KisResourceServerBase* rServer = KisResourceServerRegistry::instance()->get("BrushServer");
    foreach(KisResource* res, rServer->resources())
        if(res->name() == brushname)
            return new Brush(this, dynamic_cast<KisBrush*>(res), true);
    kWarning() << QString("Unknown brush \"%1\"").arg(brushname) << endl;
    return 0;
}

QObject* Module::createCircleBrush(uint w, uint h, uint hf, uint vf)
{
    KisAutobrushShape* kas = new KisAutobrushCircleShape(qMax(1u,w), qMax(1u,h), hf, vf);
    QImage* brsh = new QImage();
    kas->createBrush(brsh);
    KisAutobrushResource *thing = new KisAutobrushResource(*brsh);
    delete brsh;
    return new Brush(this, thing, false);
}

QObject* Module::createRectBrush(uint w, uint h, uint hf, uint vf)
{
    KisAutobrushShape* kas = new KisAutobrushRectShape(qMax(1u,w), qMax(1u,h), hf, vf);
    QImage* brsh = new QImage();
    kas->createBrush(brsh);
    KisAutobrushResource *thing = new KisAutobrushResource(*brsh);
    delete brsh;
    return new Brush(this, thing, false);
}

QObject* Module::loadPattern(const QString& filename)
{
    KisPattern* pattern = new KisPattern(filename);
    if(pattern->load())
        return new Pattern(this, pattern, false);
    delete pattern;
    kWarning() << i18n("Unknown pattern \"%1\"", filename) << endl;
    return 0;
}

QObject* Module::loadBrush(const QString& filename)
{
    KisBrush* brush = new KisBrush(filename);
    if(brush->load())
        return new Brush(this, brush, false);
    delete brush;
    kWarning() << i18n("Unknown brush \"%1\"", filename) << endl;
    return 0;
}

QObject* Module::filter(const QString& filtername)
{
    KisFilter* filter = KisFilterRegistry::instance()->get(filtername).data();
    if(filter)
    {
        return new Filter(this, filter);
    } else {
      return 0;
    }
}

QObject* Module::createImage(int width, int height, const QString& colorspace, const QString& name)
{
    if( width < 0 || height < 0)
    {
        kWarning() << i18n("Invalid image size") << endl;
        return 0;
    }
    KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace(colorspace, 0);
    if(!cs)
    {
        kWarning() << i18n("Colorspace %1 is not available, please check your installation.", colorspace ) << endl;
        return 0;
    }
    return new Image(this, KisImageSP(new KisImage(0, width, height, cs, name)));
}

#include "krs_module.moc"
