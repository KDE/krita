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

// kde
#include <kapplication.h>
#include <kdebug.h>

// koffice
#include <KoDocumentAdaptor.h>
#include <KoApplicationAdaptor.h>
#include <KoColorSpaceRegistry.h>

// krita
#include <kis_view.h>
#include <kis_doc.h>
#include <kis_image.h>
#include <kis_autobrush_resource.h>
#include <kis_resourceserver.h>
#include <kis_brush.h>
#include <kis_pattern.h>
//#include <kis_doc.h>
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
        return new Kross::KritaCore::KritaCoreModule(manager);
    }
}
#endif

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
    if(! d->progress)
        d->progress = new KritaCoreProgress(d->view);
    return d->progress;
}

QObject* KritaCoreModule::image()
{
    ::KisDoc* document = d->view->canvasSubject()->document();
    return document ? new Image(this, document->currentImage(), document) : 0;
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
            return new Pattern(this, dynamic_cast<KisPattern*>(res), true);
    kWarning() << QString("Unknown pattern \"%1\"").arg(patternname) << endl;
    return 0;
}

QObject* KritaCoreModule::brush(const QString& brushname)
{
    KisResourceServerBase* rServer = KisResourceServerRegistry::instance()->get("BrushServer");
    foreach(KisResource* res, rServer->resources())
        if(res->name() == brushname)
            return new Brush(this, dynamic_cast<KisBrush*>(res), true);
    kWarning() << QString("Unknown brush \"%1\"").arg(brushname) << endl;
    return 0;
}

QObject* KritaCoreModule::createCircleBrush(uint w, uint h, uint hf, uint vf)
{
    KisAutobrushShape* kas = new KisAutobrushCircleShape(qMax(1u,w), qMax(1u,h), hf, vf);
    QImage* brsh = new QImage();
    kas->createBrush(brsh);
    return new Brush(this, new KisAutobrushResource(*brsh), false);
}

QObject* KritaCoreModule::createRectBrush(uint w, uint h, uint hf, uint vf)
{
    KisAutobrushShape* kas = new KisAutobrushRectShape(qMax(1u,w), qMax(1u,h), hf, vf);
    QImage* brsh = new QImage();
    kas->createBrush(brsh);
    return new Brush(this, new KisAutobrushResource(*brsh), false);
}

QObject* KritaCoreModule::loadPattern(const QString& filename)
{
    KisPattern* pattern = new KisPattern(filename);
    if(pattern->load())
        return new Pattern(this, pattern, false);
    delete pattern;
    kWarning() << i18n("Unknown pattern \"%1\"", filename) << endl;
    return 0;
}

QObject* KritaCoreModule::loadBrush(const QString& filename)
{
    KisBrush* brush = new KisBrush(filename);
    if(brush->load())
        return new Brush(this, brush, false);
    delete brush;
    kWarning() << i18n("Unknown brush \"%1\"", filename) << endl;
    return 0;
}

QObject* KritaCoreModule::filter(const QString& filtername)
{
    KisFilter* filter = KisFilterRegistry::instance()->get(filtername).data();
    return new Filter(this, filter);
}

QObject* KritaCoreModule::createImage(int width, int height, const QString& colorspace, const QString& name)
{
    if( width < 0 || height < 0)
    {
        kWarning() << i18n("Invalid image size") << endl;
        return 0;
    }
    KoColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->colorSpace(colorspace, 0);
    if(!cs)
    {
        kWarning() << i18n("Colorspace %1 is not available, please check your installation.", colorspace ) << endl;
        return 0;
    }
    return new Image(this, KisImageSP(new KisImage(0, width, height, cs, name)));
}

#include "kritacoremodule.moc"
