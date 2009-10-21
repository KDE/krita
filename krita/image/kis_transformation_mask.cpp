/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_transformation_mask.h"
#include "kis_selection.h"
#include "kis_paint_device.h"
#include "kis_transform_worker.h"
#include "kis_node_visitor.h"
#include "kis_filter_strategy.h"

#include "QRect"

KisTransformationMask::KisTransformationMask()
        : KisEffectMask()
        , m_xscale(0)
        , m_yscale(0)
        , m_xshear(0)
        , m_yshear(0)
        , m_rotation(0)
        , m_xtranslate(0)
        , m_ytranslate(0)
{
    Q_ASSERT(KisFilterStrategyRegistry::instance()->keys().count() > 0);
    m_filter = KisFilterStrategyRegistry::instance()->get(KisFilterStrategyRegistry::instance()->keys()[0]);
}

KisTransformationMask::~KisTransformationMask()
{
}

bool KisTransformationMask::allowAsChild(KisNodeSP node) const
{
    Q_UNUSED(node);
    return false;
}


KisTransformationMask::KisTransformationMask(const KisTransformationMask& rhs)
        : KisEffectMask(rhs)
{
    m_xscale = rhs.m_xscale;
    m_yscale = rhs.m_yscale;
    m_xshear = rhs.m_xshear;
    m_yshear = rhs.m_yshear;
    m_rotation = rhs.m_rotation;
    m_xtranslate = rhs.m_xtranslate;
    m_ytranslate = rhs.m_ytranslate;
    m_filter = rhs.m_filter;
}

void KisTransformationMask::apply(KisPaintDeviceSP projection, const QRect & rc) const
{
    Q_UNUSED(rc);

    /**
     * FIXME: What about current selection of the mask?
     */

    // Transform
    KisTransformWorker worker(projection, m_xscale, m_yscale, m_xshear, m_yshear, m_rotation, m_xtranslate, m_ytranslate, 0, m_filter);
    worker.run();
}

QRect KisTransformationMask::changeRect(const QRect &rect) const
{
    //FIXME: selections?
    return rect;
}

QRect KisTransformationMask::needRect(const QRect &rect) const
{
    //FIXME: selections?
    return rect;
}

KisTransformationSettingsCommand::KisTransformationSettingsCommand(KisTransformationMaskSP mask,
        const QString & old_name,
        double old_xscale,
        double old_yscale,
        double old_xshear,
        double old_yshear,
        double old_rotation,
        qint32 old_xtranslate,
        qint32 old_ytranslate,
        KisFilterStrategy * old_filter,
        const QString & new_name,
        double new_xscale,
        double new_yscale,
        double new_xshear,
        double new_yshear,
        double new_rotation,
        qint32 new_xtranslate,
        qint32 new_ytranslate,
        KisFilterStrategy * new_filter)
        : QUndoCommand(i18n("Transformation Mask Settings"))
        , m_mask(mask)
        , m_old_name(old_name)
        , m_old_xscale(old_xscale)
        , m_old_yscale(old_yscale)
        , m_old_xshear(old_xshear)
        , m_old_yshear(old_yshear)
        , m_old_rotation(old_rotation)
        , m_old_xtranslate(old_xtranslate)
        , m_old_ytranslate(old_ytranslate)
        , m_old_filter(old_filter)
        , m_new_name(new_name)
        , m_new_xscale(new_xscale)
        , m_new_yscale(new_yscale)
        , m_new_xshear(new_xshear)
        , m_new_yshear(new_yshear)
        , m_new_rotation(new_rotation)
        , m_new_xtranslate(new_xtranslate)
        , m_new_ytranslate(new_ytranslate)
        , m_new_filter(new_filter)
{
}

void KisTransformationSettingsCommand::redo()
{
    m_mask->setName(m_new_name);
    m_mask->setXScale(m_new_xscale);
    m_mask->setYScale(m_new_yscale);
    m_mask->setXShear(m_new_xshear);
    m_mask->setYShear(m_new_yshear);
    m_mask->setRotation(m_new_rotation);
    m_mask->setXTranslation(m_new_xtranslate);
    m_mask->setYTranslation(m_new_ytranslate);
    m_mask->setFilterStrategy(m_new_filter);
}

void KisTransformationSettingsCommand::undo()
{
    m_mask->setName(m_old_name);
    m_mask->setXScale(m_old_xscale);
    m_mask->setYScale(m_old_yscale);
    m_mask->setXShear(m_old_xshear);
    m_mask->setYShear(m_old_yshear);
    m_mask->setRotation(m_old_rotation);
    m_mask->setXTranslation(m_old_xtranslate);
    m_mask->setYTranslation(m_old_ytranslate);
    m_mask->setFilterStrategy(m_old_filter);
}

bool KisTransformationMask::accept(KisNodeVisitor &v)
{
    return v.visit(this);
}



#include "kis_transformation_mask.moc"
