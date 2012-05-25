/*
 *  Copyright (c) 2006-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007 Thomas Zander <zander@kde.org>
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_odg_import.h"

#include <kpluginfactory.h>

#include <KoFilterChain.h>

#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_view2.h>
#include <kis_shape_layer.h>

#include <KoOdfReadStore.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoXmlNS.h>
#include <KoShapeRegistry.h>


#include <KoShapeBasedDocumentBase.h>
#include <KoColorSpaceRegistry.h>

K_PLUGIN_FACTORY(ODGImportFactory, registerPlugin<KisODGImport>();)
K_EXPORT_PLUGIN(ODGImportFactory("calligrafilters"))

KisODGImport::KisODGImport(QObject *parent, const QVariantList &) : KoFilter(parent)
{
}

KisODGImport::~KisODGImport()
{
}

KoFilter::ConversionStatus KisODGImport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile << "Import odg";

    if (to != "application/x-krita")
        return KoFilter::BadMimeType;

    KisDoc2 * doc = dynamic_cast<KisDoc2*>(m_chain -> outputDocument());

    if (!doc)
        return KoFilter::CreationError;

    QString filename = m_chain->inputFile();

    KoStore* store = KoStore::createStore(filename, KoStore::Read, from, KoStore::Zip);
    if (!store || store->bad()) {
        delete store;
        return KoFilter::BadConversionGraph;
    }
    store->disallowNameExpansion();

    doc -> prepareForImport();

    KoOdfReadStore odfStore(store);
    QString errorMessage;

    odfStore.loadAndParse(errorMessage);

    if (!errorMessage.isEmpty()) {
        warnKrita << errorMessage;
        return KoFilter::CreationError;
    }

    KoXmlElement contents = odfStore.contentDoc().documentElement();

    KoXmlElement body(KoXml::namedItemNS(contents, KoXmlNS::office, "body"));

    if (body.isNull()) {
        //setErrorMessage( i18n( "Invalid OASIS document. No office:body tag found." ) );
        return KoFilter::CreationError;
    }

    body = KoXml::namedItemNS(body, KoXmlNS::office, "drawing");
    if (body.isNull()) {
        //setErrorMessage( i18n( "Invalid OASIS document. No office:drawing tag found." ) );
        return KoFilter::CreationError;
    }

    KoXmlElement page(KoXml::namedItemNS(body, KoXmlNS::draw, "page"));
    if (page.isNull()) {
        //setErrorMessage( i18n( "Invalid OASIS document. No draw:page tag found." ) );
        return KoFilter::CreationError;
    }

    KoXmlElement * master = 0;
    if (odfStore.styles().masterPages().contains("Standard"))
        master = odfStore.styles().masterPages().value("Standard");
    else if (odfStore.styles().masterPages().contains("Default"))
        master = odfStore.styles().masterPages().value("Default");
    else if (! odfStore.styles().masterPages().empty())
        master = odfStore.styles().masterPages().begin().value();

    qint32 width = 1000;
    qint32 height = 1000;
    if (master) {
        const KoXmlElement *style = odfStore.styles().findStyle(
                                        master->attributeNS(KoXmlNS::style, "page-layout-name", QString()));
        KoPageLayout pageLayout;
        pageLayout.loadOdf(*style);
        width = pageLayout.width;
        height = pageLayout.height;
    }
    // We work fine without a master page

    KoOdfLoadingContext context(odfStore.styles(), odfStore.store());
    context.setManifestFile(QString("tar:/") + odfStore.store()->currentPath() + "META-INF/manifest.xml");
    KoShapeLoadingContext shapeContext(context, doc->shapeController()->resourceManager());

    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageWSP image = new KisImage(doc->createUndoStore(), width, height, cs, "built image");
    doc->setCurrentImage(image);

    KoShapeLayer *rootLayer = doc->shapeForNode(image->root());

    KoXmlElement layerElement;
    forEachElement(layerElement, KoXml::namedItemNS(page, KoXmlNS::draw, "layer-set")) {

    KisShapeLayerSP shapeLayer = new KisShapeLayer(rootLayer, doc->shapeController(), image,
                                        i18n("Vector Layer"),
                                        OPACITY_OPAQUE_U8);
    if (!shapeLayer->loadOdf(layerElement, shapeContext)) {
            kWarning() << "Could not load vector layer!";
            return KoFilter::CreationError;
        }
        image->addNode(shapeLayer, image->rootLayer(), 0);
    }

    KoXmlElement child;
    forEachElement(child, page) {
        /*KoShape * shape = */KoShapeRegistry::instance()->createShapeFromOdf(child, shapeContext);
    }

    return KoFilter::OK;
}


