/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_exposure_visitor.h"
#include <KoColorSpace.h>
#include <KoColorProfile.h>

#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_paint_device.h"
#include "generator/kis_generator_layer.h"

KisExposureVisitor::KisExposureVisitor(double exposure) : m_exposure(exposure)
{

}

void KisExposureVisitor::setExposureToProfile(KoColorProfile* profile)
{
    profile->setProperty("exposure", m_exposure);
}

bool KisExposureVisitor::visit(KisExternalLayer * e)
{
    Q_UNUSED(e);
    return true;
}
bool KisExposureVisitor::visit(KisPaintLayer *layer)
{
    setExposureToProfile(layer->original()->colorSpace()->profile());
    layer->setDirty();
    return true;
}

bool KisExposureVisitor::visit(KisGeneratorLayer *layer)
{
    setExposureToProfile(layer->original()->colorSpace()->profile());
    layer->setDirty();
    return true;
}

bool KisExposureVisitor::visit(KisGroupLayer *layer)
{
    setExposureToProfile(layer->original()->colorSpace()->profile());
    layer->setDirty();
    KisLayerSP child = dynamic_cast<KisLayer*>(layer->firstChild().data());
    while (child) {
        child->accept(*this);
        child = dynamic_cast<KisLayer*>(child->nextSibling().data());
    }
    return true;
}
bool KisExposureVisitor::visit(KisAdjustmentLayer* layer)
{
    Q_UNUSED(layer);
    return true;
}
