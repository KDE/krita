/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_generator_layer.h"

#include <klocale.h>
#include "kis_debug.h"

#include "kis_selection.h"
#include "filter/kis_filter_configuration.h"
#include "kis_processing_information.h"
#include "generator/kis_generator_registry.h"
#include "generator/kis_generator.h"
#include "kis_node_visitor.h"

class KisGeneratorLayer::Private
{
public:
    KisFilterConfiguration * filterConfig;
};


KisGeneratorLayer::KisGeneratorLayer(KisImageWSP img,
                                     const QString &name,
                                     KisFilterConfiguration *kfc,
                                     KisSelectionSP selection)
        : KisSelectionBasedLayer(img, name, selection),
        m_d(new Private())
{
    m_d->filterConfig = kfc;
    update();
}

KisGeneratorLayer::KisGeneratorLayer(const KisGeneratorLayer& rhs)
        : KisSelectionBasedLayer(rhs),
        m_d(new Private())
{
    m_d->filterConfig = new KisFilterConfiguration(*rhs.m_d->filterConfig);
}

KisGeneratorLayer::~KisGeneratorLayer()
{
    delete m_d->filterConfig;
    delete m_d;
}

KisFilterConfiguration * KisGeneratorLayer::filter() const
{
    Q_ASSERT(m_d->filterConfig);
    return m_d->filterConfig;
}


void KisGeneratorLayer::setFilter(KisFilterConfiguration *filterConfig)
{
    Q_ASSERT(filterConfig);
    m_d->filterConfig = filterConfig;
    update();
}


void KisGeneratorLayer::update()
{

    KisGeneratorSP f = KisGeneratorRegistry::instance()->value(m_d->filterConfig->name());
    if (!f) return;

    QRect processRect = exactBounds();

    resetCache(f->colorSpace());
    KisPaintDeviceSP originalDevice = original();

    KisProcessingInformation dstCfg(originalDevice,
                                    processRect.topLeft(),
                                    selection());

    m_d->filterConfig->setChannelFlags(channelFlags());
    f->generate(dstCfg, processRect.size(), m_d->filterConfig);
}

bool KisGeneratorLayer::accept(KisNodeVisitor & v)
{
    return v.visit(this);
}

QIcon KisGeneratorLayer::icon() const
{
    return KIcon("view-filter");
}

KoDocumentSectionModel::PropertyList KisGeneratorLayer::sectionModelProperties() const
{
    KoDocumentSectionModel::PropertyList l = KisLayer::sectionModelProperties();
    l << KoDocumentSectionModel::Property(i18n("Generator"),
                                          KisGeneratorRegistry::instance()->value(m_d->filterConfig->name())->name());
    return l;
}


#include "kis_generator_layer.moc"
