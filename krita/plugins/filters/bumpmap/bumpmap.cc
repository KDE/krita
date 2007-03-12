/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005 Boudewijn <boud@valdyas.org>
 * Copyright (c) 2007 Benjamin Schleimer <bensch128@yahoo.com>
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
 *
 *
 * This implementation completely and utterly based on the gimp's bumpmap.c,
 * copyright:
 * Copyright (C) 1997 Federico Mena Quintero <federico@nuclecu.unam.mx>
 * Copyright (C) 1997-2000 Jens Lautenbacher <jtl@gimp.org>
 * Copyright (C) 2000 Sven Neumann <sven@gimp.org>
 *
 */

#include <stdlib.h>
#include <vector>

#include <qpoint.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qlineedit.h>

#include <knuminput.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kcombobox.h>

#include <kis_doc.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_filter_registry.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_layer.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <kis_adjustment_layer.h>

#include "wdgbumpmap.h"
#include "bumpmap.h"

#define MOD(x, y) \
  ((x) < 0 ? ((y) - 1 - ((y) - 1 - (x)) % (y)) : (x) % (y))

typedef KGenericFactory<KritaBumpmap> KritaBumpmapFactory;
K_EXPORT_COMPONENT_FACTORY( kritabumpmap, KritaBumpmapFactory( "krita" ) )

KritaBumpmap::KritaBumpmap(QObject *parent, const char *name, const QStringList &)
        : KParts::Plugin(parent, name)
{
    setInstance(KritaBumpmapFactory::instance());


    if (parent->inherits("KisFilterRegistry")) {
        KisFilterRegistry * manager = dynamic_cast<KisFilterRegistry *>(parent);
        manager->add(new KisFilterBumpmap());
    }
}

KritaBumpmap::~KritaBumpmap()
{
}

KisFilterBumpmap::KisFilterBumpmap() : KisFilter(id(), "map", i18n("&Bumpmap..."))
{
}

namespace {
    void convertRow(KisPaintDevice * orig, Q_UINT8 * row, Q_INT32 x, Q_INT32 y, Q_INT32 w,  Q_UINT8 * lut, Q_INT32 waterlevel)
    {
        KisColorSpace * csOrig = orig->colorSpace();

        KisHLineIteratorPixel origIt = orig->createHLineIterator(x, y, w, false);
        for (int i = 0; i < w; ++i) {
            row[0] = csOrig->intensity8(origIt.rawData());
            row[0] = lut[waterlevel + ((row[0] -  waterlevel) * csOrig->getAlpha(origIt.rawData())) / 255];

            ++row;
            ++origIt;
        }
    }

}

void KisFilterBumpmap::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* cfg, const QRect& rect)
{
    if (!src) return;
    if (!dst) return;
    if (!cfg) return;
    if (!rect.isValid()) return;
    if (rect.isNull()) return;
    if (rect.isEmpty()) return;

    KisBumpmapConfiguration * config = (KisBumpmapConfiguration*)cfg;

    Q_INT32 xofs, yofs; /// The x,y offset values
    Q_INT32 lx, ly;       /* X and Y components of light vector */
    Q_INT32 nz2, nzlz;    /* nz^2, nz*lz */
    Q_INT32 background;   /* Shade for vertical normals */
    double  compensation; /* Background compensation */
    Q_UINT8 lut[256];     /* Look-up table for modes */

    double azimuth;
    double elevation;
    Q_INT32 lz, nz;
    Q_INT32 i;
    double n;

    // ------------------ Prepare parameters

    /* Convert the offsets */
    xofs = -config->xofs;
    yofs = -config->yofs;

    /* Convert to radians */
    azimuth   = M_PI * config->azimuth / 180.0;
    elevation = M_PI * config->elevation / 180.0;

    /* Calculate the light vector */
    lx = (Q_INT32)(cos(azimuth) * cos(elevation) * 255.0);
    ly = (Q_INT32)(sin(azimuth) * cos(elevation) * 255.0);

    lz = (Q_INT32)(sin(elevation) * 255.0);

    /* Calculate constant Z component of surface normal */
    nz = (Q_INT32)((6 * 255) / config->depth);
    nz2  = nz * nz;
    nzlz = nz * lz;

    /* Optimize for vertical normals */
    background = lz;

    /* Calculate darkness compensation factor */
    compensation = sin(elevation);

    /* Create look-up table for map type */
    for (i = 0; i < 256; i++)
    {
        switch (config->type)
        {
        case SPHERICAL:
            n = i / 255.0 - 1.0;
            lut[i] = (int) (255.0 * sqrt(1.0 - n * n) + 0.5);
            break;

        case SINUSOIDAL:
            n = i / 255.0;
            lut[i] = (int) (255.0 *
                    (sin((-M_PI / 2.0) + M_PI * n) + 1.0) /
                    2.0 + 0.5);
            break;

        case LINEAR:
            default:
            lut[i] = i;
        }

        if (config->invert)
            lut[i] = 255 - lut[i];
    }


    // Crate a grayscale layer from the bumpmap layer.
    QRect bmRect;
    KisPaintDevice * bumpmap;

    if (!config->bumpmap.isNull() && src->image()) {
        KisLayerSP l = src->image()->findLayer(config->bumpmap);
        KisPaintDeviceSP bumplayer = 0;

        KisPaintLayer * pl = dynamic_cast<KisPaintLayer*>(l.data());
        if (pl) {
            bumplayer = pl->paintDevice();
        }
        else {
            KisGroupLayer * gl = dynamic_cast<KisGroupLayer*>(l.data());
            if (gl) {
                bumplayer = gl->projection(gl->extent());
            }
            else {
                KisAdjustmentLayer * al = dynamic_cast<KisAdjustmentLayer*>(l.data());
                if (al) {
                    bumplayer = al->cachedPaintDevice();
                }
            }
        }


        if (bumplayer) {
            bmRect = bumplayer->exactBounds();
            bumpmap = bumplayer.data();
        }
        else {
            bmRect = rect;
            bumpmap = src;
        }
     }
     else {
         bmRect = rect;
         bumpmap = src;
     }

     kdDebug(12345) << "KisFilterBumpmap::process: rect=" << rect << ", bumpmap rect=" << bmRect << "\n";
     
    setProgressTotalSteps(rect.height());

    // ---------------------- Load initial three bumpmap scanlines

    KisColorSpace * srcCs = src->colorSpace();
    QValueVector<KisChannelInfo *> channels = srcCs->channels();

    // One byte per pixel, converted from the bumpmap layer.
    Q_UINT8 * bm_row1 = new Q_UINT8[bmRect.width()];
    Q_UINT8 * bm_row2 = new Q_UINT8[bmRect.width()];
    Q_UINT8 * bm_row3 = new Q_UINT8[bmRect.width()];
    Q_UINT8 * tmp_row;

    // ------------------- Map the bumps
    Q_INT32 yofs1, yofs2, yofs3;
    
    // ------------------- Initialize offsets
    if (config->tiled) {
        yofs2 = MOD (yofs, bmRect.height());
        yofs1 = MOD (yofs2 - 1, bmRect.height());
        yofs3 = MOD (yofs2 + 1, bmRect.height());
    }
    else {
        yofs2 = 0;
        yofs1 = yofs2 - 1;
        yofs3 = yofs2 + 1;
    }
    convertRow(bumpmap, bm_row1, bmRect.x(), yofs1+bmRect.top(), bmRect.width(), lut, config->waterlevel);
    convertRow(bumpmap, bm_row2, bmRect.x(), yofs2+bmRect.top(), bmRect.width(), lut, config->waterlevel);
    convertRow(bumpmap, bm_row3, bmRect.x(), yofs3+bmRect.top(), bmRect.width(), lut, config->waterlevel);

    for (int y = rect.top(); y<=rect.bottom(); y++) {
        const Q_INT32 yBump = y+yofs;
        if(config->tiled || (bmRect.top()<=yBump && yBump<=bmRect.bottom()) ) {
            // Get the iterators
            KisHLineIteratorPixel dstIt = dst->createHLineIterator(rect.x(), y, rect.width(), true);
            KisHLineIteratorPixel srcIt = src->createHLineIterator(rect.x(), y, rect.width(), false);

            //while (x < sel_w || cancelRequested()) {
            while (!srcIt.isDone() && !cancelRequested()) {
                if (srcIt.isSelected()) {
                    
                    const Q_INT32 xBump = srcIt.x()+xofs;
                    Q_INT32 nx, ny;
                    // Calculate surface normal from bumpmap
                    if (config->tiled || bmRect.left() <= xBump && xBump <= bmRect.right()) {
                        
                        Q_INT32 xofs1, xofs2, xofs3;
                        if (config->tiled) {
                            xofs2 = MOD (xBump-bmRect.left(), bmRect.width());
                            xofs1 = MOD (xofs2 - 1, bmRect.width());
                            xofs3 = MOD (xofs2 + 1, bmRect.width());
                        } else {
                            xofs2 = MOD (xBump-bmRect.left(), bmRect.width());
                            xofs1 = ::max (xofs2 - 1, 0);
                            xofs3 = ::min (xofs2 + 1, bmRect.width());
                        }

                        nx = (bm_row1[xofs1] + bm_row2[xofs1] + bm_row3[xofs1] -
                              bm_row1[xofs3] - bm_row2[xofs3] - bm_row3[xofs3]);
                        ny = (bm_row3[xofs1] + bm_row3[xofs2] + bm_row3[xofs3] -
                              bm_row1[xofs1] - bm_row1[xofs2] - bm_row1[xofs3]);   
                    } else {
                        nx = 0;
                        ny = 0;
                    }

                    // Shade
                    Q_INT32 shade;
                    if ((nx == 0) && (ny == 0)) {
                        shade = background;
                    } else {
                        Q_INT32 ndotl = (nx * lx) + (ny * ly) + nzlz;
                        
                        if (ndotl < 0) {
                            shade = (Q_INT32)(compensation * config->ambient);
                        } else {
                            shade = (Q_INT32)(ndotl / sqrt(nx * nx + ny * ny + nz2));
                            shade = (Q_INT32)(shade + QMAX(0, (255 * compensation - shade)) * config->ambient / 255);
                        }
                    }

                    // Paint
                    srcCs->darken(srcIt.rawData(), dstIt.rawData(), shade, config->compensate, compensation, 1);
                }

                ++srcIt;
                ++dstIt;
            }

            // Go to the next row
            tmp_row = bm_row1;
            bm_row1 = bm_row2;
            bm_row2 = bm_row3;
            bm_row3 = tmp_row;

            yofs2++;
            if (yofs2 >= bmRect.height()) { yofs2 = 0; }

            if (config->tiled) {
                yofs3 = MOD (yofs2 + 1, bmRect.height());
            } else {
                yofs3 = yofs2 + 1;
            }
            convertRow(bumpmap, bm_row3, bmRect.x(), yofs3+bmRect.top(), bmRect.width(), lut, config->waterlevel);
        }

        incProgress();
    }

    delete [] bm_row1;
    delete [] bm_row2;
    delete [] bm_row3;
    setProgressDone();

}

KisFilterConfigWidget * KisFilterBumpmap::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev)
{
    KisBumpmapConfigWidget * w = new KisBumpmapConfigWidget(this, dev, parent);


    return w;
}

KisFilterConfiguration * KisFilterBumpmap::configuration(QWidget * w)
{

    KisBumpmapConfigWidget * widget = dynamic_cast<KisBumpmapConfigWidget *>(w);
    if (widget == 0) {
        return new KisBumpmapConfiguration();
    }
    else {
        return widget->config();
    }
}

KisFilterConfiguration * KisFilterBumpmap::configuration()
{
    return new KisBumpmapConfiguration();
}


KisBumpmapConfiguration::KisBumpmapConfiguration()
    : KisFilterConfiguration( "bumpmap", 1 )
{
    bumpmap = QString::null;
    azimuth = 135.0;
    elevation = 45.0;
    depth = 3;
    xofs = 0;
    yofs = 0;
    waterlevel = 0;
    ambient = 0;
    compensate = true;
    invert = false;
    tiled = true;
    type = krita::LINEAR;
}
void KisBumpmapConfiguration::fromXML(const QString & s)
{
    KisFilterConfiguration::fromXML( s );

    bumpmap = QString::null;
    azimuth = 135.0;
    elevation = 45.0;
    depth = 3;
    xofs = 0;
    yofs = 0;
    waterlevel = 0;
    ambient = 0;
    compensate = true;
    invert = false;
    tiled = true;
    type = krita::LINEAR;

    QVariant v;

    v = getProperty("bumpmap");
    if (v.isValid()) { bumpmap = v.asString(); }
    v = getProperty("azimuth");
    if (v.isValid()) { azimuth = v.asDouble(); }
    v = getProperty("elevation");
    if (v.isValid()) { elevation = v.asDouble();}
    v = getProperty("depth");
    if (v.isValid()) { depth = v.asDouble(); }
    v = getProperty("xofs");
    if (v.isValid()) { xofs = v.asInt(); }
    v = getProperty("yofs");
    if (v.isValid()) { yofs = v.asInt();}
    v = getProperty("waterlevel");
    if (v.isValid()) { waterlevel = v.asInt();}
    v = getProperty("ambient");
    if (v.isValid()) { ambient = v.asInt();}
    v = getProperty("compensate");
    if (v.isValid()) { compensate = v.asBool(); }
    v = getProperty("invert");
    if (v.isValid()) { invert = v.asBool(); }
    v = getProperty("tiled");
    if (v.isValid()) { tiled = v.asBool();}
    v = getProperty("type");
    if (v.isValid()) { type = (enumBumpmapType)v.asInt(); }

}


QString KisBumpmapConfiguration::toString()
{
    m_properties.clear();

    //setProperty("bumpmap", QVariant(bumpmap));
    setProperty("azimuth", QVariant(azimuth));
    setProperty("elevation", QVariant(elevation));
    setProperty("depth", QVariant(depth));
    setProperty("xofs", QVariant(xofs));
    setProperty("yofs", QVariant(yofs));
    setProperty("waterlevel", QVariant(waterlevel));
    setProperty("ambient", QVariant(ambient));
    setProperty("compensate", QVariant(compensate));
    setProperty("invert", QVariant(invert));
    setProperty("tiled", QVariant(tiled));
    setProperty("type", QVariant(type));

    return KisFilterConfiguration::toString();
}

KisBumpmapConfigWidget::KisBumpmapConfigWidget(KisFilter *, KisPaintDeviceSP dev, QWidget * parent, const char * name, WFlags f)
    : KisFilterConfigWidget(parent, name, f)
{
    m_page = new WdgBumpmap(this);
    QHBoxLayout * l = new QHBoxLayout(this);
    Q_CHECK_PTR(l);

    l->add(m_page);

    // Find all of the layers in the group
    if(dev->image() ) {
        KisGroupLayerSP root = dev->image()->rootLayer();
        for(KisLayerSP layer = root->firstChild(); layer; layer = layer->nextSibling())
        {
            m_page->cboxSourceLayer->insertItem(layer->name());
        }
    }

    // Connect all of the widgets to update signal
    connect( m_page->radioLinear, SIGNAL( toggled(bool)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->radioSpherical, SIGNAL( toggled(bool)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->radioSinusoidal, SIGNAL( toggled(bool)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->chkCompensate, SIGNAL( toggled(bool)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->chkInvert, SIGNAL( toggled(bool)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->chkTiled, SIGNAL( toggled(bool)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->dblAzimuth, SIGNAL( valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->dblElevation, SIGNAL( valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->dblDepth, SIGNAL( valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->intXOffset, SIGNAL( valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->intYOffset, SIGNAL( valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->intWaterLevel, SIGNAL( valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->intAmbient, SIGNAL( valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
}

KisBumpmapConfiguration * KisBumpmapConfigWidget::config()
{
    KisBumpmapConfiguration * cfg = new KisBumpmapConfiguration();
    cfg->bumpmap = m_page->cboxSourceLayer->currentText();
    cfg->azimuth = m_page->dblAzimuth->value();
    cfg->elevation = m_page->dblElevation->value();
    cfg->depth = m_page->dblDepth->value();
    cfg->xofs = m_page->intXOffset->value();
    cfg->yofs = m_page->intYOffset->value();
    cfg->waterlevel = m_page->intWaterLevel->value();
    cfg->ambient = m_page->intAmbient->value();
    cfg->compensate = m_page->chkCompensate->isChecked();
    cfg->invert = m_page->chkInvert->isChecked();
    cfg->tiled = m_page->chkTiled->isChecked();
    cfg->type = (enumBumpmapType)m_page->grpType->selectedId();

    return cfg;
}

void KisBumpmapConfigWidget::setConfiguration(KisFilterConfiguration * config)
{
    KisBumpmapConfiguration * cfg = dynamic_cast<KisBumpmapConfiguration*>(config);
    if (!cfg) return;

    // NOTE: maybe we should find the item instead?
    m_page->cboxSourceLayer->setCurrentText( cfg->bumpmap );
    m_page->dblAzimuth->setValue(cfg->azimuth);
    m_page->dblElevation->setValue(cfg->elevation);
    m_page->dblDepth->setValue(cfg->depth);
    m_page->intXOffset->setValue(cfg->xofs);
    m_page->intYOffset->setValue(cfg->yofs);
    m_page->intWaterLevel->setValue(cfg->waterlevel);
    m_page->intAmbient->setValue(cfg->ambient);
    m_page->chkCompensate->setChecked(cfg->compensate);
    m_page->chkInvert->setChecked(cfg->invert);
    m_page->chkTiled->setChecked(cfg->tiled);
    m_page->grpType->setButton(cfg->type);

}

#include "bumpmap.moc"
