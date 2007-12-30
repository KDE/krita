/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2007
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_kra_loader.h"

#include <KoStore.h>
#include <KoColorSpaceRegistry.h>
#include <KoIccColorProfile.h>
#include <KoDocumentInfo.h>

#include "kis_doc2.h"
#include "kis_shape_layer.h"
#include "kis_config.h"
#include "kis_kra_load_visitor.h"

#include <kis_base_node.h>
#include <kis_adjustment_layer.h>
#include <kis_annotation.h>
#include <kis_debug.h>
#include <kis_external_layer_iface.h>
#include <kis_filter.h>
#include <kis_filter_registry.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_name_server.h>
#include <kis_paint_device_action.h>
#include <kis_paint_layer.h>
#include <kis_selection.h>
#include <kis_shape_layer.h>

/**
 * Mime type for native file format
 */
#define NATIVE_MIMETYPE "application/x-kra"


class KisKraLoader::Private
{
public:

    KisDoc2* document;
    QString imageName; // used to be stored in the image, is now in the documentInfo block
    QString imageComment; // used to be stored in the image, is now in the documentInfo block
    QMap<KisLayer *, QString> layerFilenames; // temp storage during
                                              // load

};
    
KisKraLoader::KisKraLoader( KisDoc2 * document )
    : m_d(new Private())
{
    m_d->document = document;
}


KisKraLoader::~KisKraLoader()
{
}


KisImageSP KisKraLoader::loadXML(const KoXmlElement& element)
{

    KisConfig cfg;
    QString attr;
    KoXmlNode node;
    KoXmlNode child;
    KisImageSP img = 0;
    QString name;
    qint32 width;
    qint32 height;
    QString description;
    QString profileProductName;
    double xres;
    double yres;
    QString colorspacename;
    const KoColorSpace * cs;

    if ((attr = element.attribute("mime")) == NATIVE_MIMETYPE) {
    
        if ((m_d->imageName = element.attribute("name")).isNull())
            return KisImageSP(0);
        
        if ((attr = element.attribute("width")).isNull())
            return KisImageSP(0);
        width = attr.toInt();
        
        if ((attr = element.attribute("height")).isNull())
            return KisImageSP(0);
        height = attr.toInt();

        m_d->imageComment = element.attribute("description");

        xres = 100.0 / 72.0;
        if (!(attr = element.attribute("x-res")).isNull()) {
            if (attr.toDouble() > 1.0)
                xres = attr.toDouble() / 72.0;
        }
        
        yres = 100.0 / 72.0;
        if (!(attr = element.attribute("y-res")).isNull()) {
            if (attr.toDouble() > 1.0)
                yres = attr.toDouble() / 72.0;
        }

        if ((colorspacename = element.attribute("colorspacename")).isNull())
        {
            // An old file: take a reasonable default.
            // Krita didn't support anything else in those
            // days anyway.
            colorspacename = "RGBA";
        }

        // A hack for an old colorspacename
        if (colorspacename  == "Grayscale + Alpha")
            colorspacename  = "GRAYA";

        if ((profileProductName = element.attribute("profile")).isNull()) {
            // no mention of profile so get default profile
            cs = KoColorSpaceRegistry::instance()->colorSpace(colorspacename,"");
        }
        else {
            cs = KoColorSpaceRegistry::instance()->colorSpace(colorspacename, profileProductName);
        }

        if (cs == 0) {
            kWarning(41008) <<"Could not open colorspace";
            return KisImageSP(0);
        }

        img = new KisImage(m_d->document->undoAdapter(), width, height, cs, name);
        
        img->lock();
        
        img->setResolution(xres, yres);

        loadLayers(element, img, img->rootLayer());
        img->unlock();

    }

    return img;
}

void KisKraLoader::loadBinaryData( KoStore * store, KisImageSP img, const QString & uri, bool external )
{
    // Load the layers data
    KisKraLoadVisitor visitor(img, store, m_d->layerFilenames, m_d->imageName);

    if(external)
        visitor.setExternalUri(uri);
    img->lock();
    img->rootLayer()->accept(visitor);
    // annotations
    // exif
    QString location = external ? QString::null : uri;
    location += m_d->imageName + "/annotations/exif";
    if (store->hasFile(location)) {
        QByteArray data;
        store->open(location);
        data = store->read(store->size());
        store->close();
        img->addAnnotation(KisAnnotationSP(new KisAnnotation("exif", "", data)));
    }
    // icc profile
    location = external ? QString::null : uri;
    location += m_d->imageName + "/annotations/icc";
    if (store->hasFile(location)) {
        QByteArray data;
        store->open(location);
        data = store->read(store->size());
        store->close();
        img->setProfile(new KoIccColorProfile(data));
    }


    if (m_d->document->documentInfo()->aboutInfo("title").isNull())
        m_d->document->documentInfo()->setAboutInfo("title", m_d->imageName);
    if (m_d->document->documentInfo()->aboutInfo("comment").isNull())
        m_d->document->documentInfo()->setAboutInfo("comment", m_d->imageComment);

    img->unlock();
}

void KisKraLoader::loadLayers(const KoXmlElement& element, KisImageSP img, KisGroupLayerSP parent)
{
    KoXmlNode node = element.lastChild();
    KoXmlNode child;

    if(!node.isNull())
    {
        if (node.isElement()) {
            if (node.nodeName() == "LAYERS") {
                for (child = node.lastChild(); !child.isNull(); child = child.previousSibling()) {
                    KisLayerSP layer = loadLayer(child.toElement(), img);

                    if (!layer) {
                        kWarning(41008) <<"Could not load layer";
                    }
                    else {
                        img->nextLayerName(); // Make sure the nameserver is current with the number of layers.
                        img->addNode(layer.data(), parent.data());
                    }
                }
            }
        }
    }
}

KisLayerSP KisKraLoader::loadLayer(const KoXmlElement& element, KisImageSP img)
{
    // Nota bene: If you add new properties to layers, you should
    // ALWAYS define a default value in case the property is not
    // present in the layer definition: this helps a LOT with backward
    // compatibility.
    QString attr;
    QString name;
    qint32 x;
    qint32 y;
    qint32 opacity;
    bool visible;
    bool locked;

    if ((name = element.attribute("name")).isNull())
        return KisLayerSP(0);

    if ((attr = element.attribute("x")).isNull())
        return KisLayerSP(0);
    x = attr.toInt();

    if ((attr = element.attribute("y")).isNull())
        return KisLayerSP(0);

    y = attr.toInt();

    if ((attr = element.attribute("opacity")).isNull())
        return KisLayerSP(0);

    if ((opacity = attr.toInt()) < 0 || opacity > quint8_MAX)
        opacity = OPACITY_OPAQUE;


    QString compositeOpName = element.attribute("compositeop");

    if ((attr = element.attribute("visible")).isNull())
        attr = "1";

    visible = attr == "0" ? false : true;

    if ((attr = element.attribute("locked")).isNull())
        attr = "0";

    locked = attr == "0" ? false : true;

    // Now find out the layer type and do specific handling
    if ((attr = element.attribute("layertype")).isNull())
        return loadPaintLayer(element, img, name, x, y, opacity, visible, locked, compositeOpName);

    if(attr == "paintlayer")
        return loadPaintLayer(element, img, name, x, y, opacity, visible, locked, compositeOpName);

    if(attr == "grouplayer")
        return KisLayerSP(loadGroupLayer(element, img, name, x, y, opacity, visible, locked, compositeOpName).data());

    if(attr == "adjustmentlayer")
        return KisLayerSP(loadAdjustmentLayer(element, img, name, x, y, opacity, visible, locked, compositeOpName).data());

    if(attr == "shapelayer")
        return KisLayerSP(loadShapeLayer(element, img, name, x, y, opacity, visible, locked, compositeOpName).data());

    kWarning(41008) <<"Specified layertype is not recognised";
    return KisLayerSP(0);
}


KisLayerSP KisKraLoader::loadPaintLayer(const KoXmlElement& element, KisImageSP img,
                                  const QString & name, qint32 x, qint32 y,
                                  qint32 opacity, bool visible, bool locked, const QString & compositeOp)
{
    QString attr;
    KisPaintLayerSP layer;
    const KoColorSpace * cs;

    QString colorspacename;
    QString profileProductName;

    if ((colorspacename = element.attribute("colorspacename")).isNull())
        cs = img->colorSpace();
    else
        // use default profile - it will be replaced later in completLoading
        cs = KoColorSpaceRegistry::instance()->colorSpace(colorspacename,"");

    const KoCompositeOp * op = cs->compositeOp(compositeOp);

    layer = new KisPaintLayer(img.data(), name, opacity, cs);
    Q_CHECK_PTR(layer);

    layer->setCompositeOp(op);
    layer->setVisible(visible);
    layer->setLocked(locked);
    layer->setX(x);
    layer->setY(y);

    if ((element.attribute("filename")).isNull())
        m_d->layerFilenames[layer.data()] = name;
    else
        m_d->layerFilenames[layer.data()] = QString(element.attribute("filename"));

    // Load exif info
/*TODO: write and use the legacy stuff to load that exif tag
        for( KoXmlNode node = element.firstChild(); !node.isNull(); node = node.nextSibling() )
    {
        KoXmlElement e = node.toElement();
        if ( !e.isNull() && e.tagName() == "ExifInfo" )
        {
            layer->paintDevice()->exifInfo()->load(e);
        }
    }*/
    // TODO load metadata
    return KisLayerSP(layer.data());
}

KisGroupLayerSP KisKraLoader::loadGroupLayer(const KoXmlElement& element, KisImageSP img,
                                       const QString & name, qint32 x, qint32 y, qint32 opacity, bool visible, bool locked,
                                       const QString & compositeOp)
{
    QString attr;
    KisGroupLayerSP layer;

    layer = new KisGroupLayer(img.data(), name, opacity);
    Q_CHECK_PTR(layer);
    const KoCompositeOp * op = img->colorSpace()->compositeOp(compositeOp);
    layer->setCompositeOp(op);
    layer->setVisible(visible);
    layer->setLocked(locked);
    layer->setX(x);
    layer->setY(y);

    loadLayers(element, img, layer);

    return layer;
}

KisAdjustmentLayerSP KisKraLoader::loadAdjustmentLayer(const KoXmlElement& element, KisImageSP img,
                                             const QString & name, qint32 x, qint32 y, qint32 opacity, bool visible, bool locked,
                                             const QString & compositeOp)
{
    QString attr;
    KisAdjustmentLayerSP layer;
    QString filtername;

    if ((filtername = element.attribute("filtername")).isNull()) {
        // XXX: Invalid adjustmentlayer! We should warn about it!
        kWarning(41008) <<"No filter in adjustment layer";
        return KisAdjustmentLayerSP(0);
    }

    KisFilterSP f = KisFilterRegistry::instance()->value(filtername);
    if (!f) {
        kWarning(41008) <<"No filter for filtername" << filtername <<"";
        return KisAdjustmentLayerSP(0); // XXX: We don't have this filter. We should warn about it!
    }

    KisFilterConfiguration * kfc = f->defaultConfiguration(0);

    // We'll load the configuration and the selection later.
    layer = KisAdjustmentLayerSP(new KisAdjustmentLayer(img, name, kfc, KisSelectionSP(0)));
    Q_CHECK_PTR(layer);

    const KoCompositeOp * op = img->colorSpace()->compositeOp(compositeOp);
    layer->setCompositeOp(op);
    layer->setVisible(visible);
    layer->setLocked(locked);
    layer->setX(x);
    layer->setY(y);
    layer->setOpacity(opacity);

    if ((element.attribute("filename")).isNull())
        m_d->layerFilenames[layer.data()] = name;
    else
        m_d->layerFilenames[layer.data()] = QString(element.attribute("filename"));

    return layer;
}


KisShapeLayerSP KisKraLoader::loadShapeLayer(const KoXmlElement& elem, KisImageSP img, const QString & name, qint32 x, qint32 y, qint32 opacity, bool visible, bool locked, const QString &compositeOp)
{
    QString attr;
    
    KisShapeLayerSP layer = new KisShapeLayer(0, img.data(), name, opacity);
    Q_CHECK_PTR(layer);
    const KoCompositeOp * op = img->colorSpace()->compositeOp(compositeOp);
    layer->setCompositeOp(op);
    static_cast<KisBaseNode*>(layer.data())->setVisible(visible);
    static_cast<KisBaseNode*>(layer.data())->setLocked(locked);
    layer->setX(x);
    layer->setY(y);

    if ((elem.attribute("filename")).isNull())
        m_d->layerFilenames[layer.data()] = name;
    else
        m_d->layerFilenames[layer.data()] = QString(elem.attribute("filename"));

    
    return layer;
}

#include "kis_kra_loader.moc"
