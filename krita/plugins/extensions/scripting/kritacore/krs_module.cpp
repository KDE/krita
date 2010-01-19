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


#include <kis_debug.h>

#include <QApplication>

// koffice
#include <KoDocumentAdaptor.h>
#include <KoApplicationAdaptor.h>
#include <KoColorSpaceRegistry.h>

// krita
#if 0
#include <kis_auto_brush.h>
#include <kis_brush.h>
#endif
#include <kis_doc2.h>
#include <filter/kis_filter.h>
#include <filter/kis_filter_registry.h>
#include <kis_image.h>
#include <kis_layer.h>

#include <kis_mask_generator.h>
#include <kis_paint_layer.h>
#include <kis_pattern.h>
#include <kis_resource_server_provider.h>
#include <kis_view2.h>

// kritacore
#if 0
#include "krs_brush.h"
#endif
#include "krs_color.h"
#include "krs_filter.h"
#include "krs_image.h"
#include "krs_pattern.h"
#include "krs_paint_layer.h"
#include "krs_progress.h"


extern "C" {
    KROSSKRITACORE_EXPORT QObject* krossmodule() {
        KisDoc2* doc = new KisDoc2(0, 0, true);

        // dirty hack to get an image defined
        const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
        doc->newImage("unnamed", 100, 100, cs /*,KoColor(QColor(255,255,255),cs)*/);
        //doc->prepareForImport();

        KisView2* view = dynamic_cast< KisView2* >(doc->createViewInstance(0 /*no parent widget*/));
        Q_ASSERT(view);
        return new Scripting::Module(view);
    }
}

using namespace Scripting;

namespace Scripting
{

/// \internal d-pointer class.
class Module::Private
{
public:
    KisView2* view;
    Progress* progress;
};

}

Module::Module(KisView2* view)
        : KoScriptingModule(view, "Krita")
        , d(new Private())
{
    d->view = view;
    d->progress = 0;
    /*
    Kross::Manager::self().addObject(d->view->canvasSubject()->document(), "KritaDocument");
    Kross::Manager::self().addObject(d->view, "KritaView");
    */
}

Module::~Module()
{
    delete d->progress;
    delete d;
}

KoDocument* Module::doc()
{
    return d->view->document();
}

QObject* Module::progress()
{
    if (! d->progress)
        d->progress = new Progress(this, d->view);
    return d->progress;
}

QObject* Module::image()
{
    ::KisDoc2* document = d->view->document();
    return document ? new Image(this, d->view->image(), document) : 0;
}

QObject* Module::activeLayer()
{
    KisLayerSP aL = d->view->activeLayer();
    KisPaintLayerSP aPL = dynamic_cast<KisPaintLayer*>(aL.data());
    if (aPL) {
        return new PaintLayer(aPL, d->view->document());
    }
    return 0;
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
    KoResourceServer<KisPattern>* rServer = KisResourceServerProvider::instance()->patternServer();
    foreach(KisPattern* pattern, rServer->resources()) {
	if (pattern->name() == patternname) {
		return new Pattern(this, pattern, true); 
	}
    }
    warnScript << QString("Unknown pattern \"%1\"").arg(patternname);
    return 0;
}
#if 0
QObject* Module::brush(const QString& brushname)
{
    KoResourceServer<KisBrush>* rServer = KisResourceServerProvider::instance()->brushServer();
    foreach(KisBrush* brush, rServer->resources()) {
        if (brush->name() == brushname) {
             return new Brush(this, brush, true);
	}
    }
    warnScript << QString("Unknown brush \"%1\"").arg(brushname);
    return 0;
}

QObject* Module::createCircleBrush(uint w, uint h, uint hf, uint vf)
{
    KisCircleMaskGenerator* kas = new KisCircleMaskGenerator(qMax(1u, w), qMax(1u, h), hf, vf);
    KisAutoBrush *thing = new KisAutoBrush(kas);
    return new Brush(this, thing, false);
}

QObject* Module::createRectBrush(uint w, uint h, uint hf, uint vf)
{
    KisRectangleMaskGenerator* kas = new KisRectangleMaskGenerator(qMax(1u, w), qMax(1u, h), hf, vf);
    KisAutoBrush *thing = new KisAutoBrush(kas);
    return new Brush(this, thing, false);
}
#endif

QObject* Module::loadPattern(const QString& filename)
{
    KisPattern* pattern = new KisPattern(filename);
    if (pattern->load())
        return new Pattern(this, pattern, false);
    delete pattern;
    warnScript << i18n("Unknown pattern \"%1\"", filename);
    return 0;
}
#if 0
QObject* Module::loadBrush(const QString& filename)
{
    KisBrush* brush = new KisBrush(filename);
    if (brush->load())
        return new Brush(this, brush, false);
    delete brush;
    warnScript << i18n("Unknown brush \"%1\"", filename);
    return 0;
}
#endif
QObject* Module::filter(const QString& filtername)
{
    KisFilter* filter = KisFilterRegistry::instance()->get(filtername).data();
    if (filter) {
        return new Filter(this, filter);
    } else {
        return 0;
    }
}

QObject* Module::createImage(int width, int height, const QString& colorSpaceModel, const QString& colorSpaceDepth, const QString& name)
{
    if (width < 0 || height < 0) {
        warnScript << i18n("Invalid image size");
        return 0;
    }
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace(colorSpaceModel, colorSpaceDepth, 0);
    if (!cs) {
        warnScript << i18n("ColorSpace %1 %2 is not available, please check your installation.", colorSpaceModel, colorSpaceDepth);
        return 0;
    }
    return new Image(this, KisImageWSP(new KisImage(0, width, height, cs, name)));
}

QWidget* Module::view()
{
    return d->view;
}

#include "krs_module.moc"
