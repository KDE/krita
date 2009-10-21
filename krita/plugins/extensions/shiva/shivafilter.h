/*
 * Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _SHIVA_FILTER_H_
#define _SHIVA_FILTER_H_

#include "filter/kis_filter.h"

namespace OpenShiva
{
class Source;
};

class ShivaFilter : public KisFilter
{
public:
    ShivaFilter(OpenShiva::Source* source);
    virtual ~ShivaFilter();

    using KisFilter::process;

    virtual void process(KisConstProcessingInformation src,
                         KisProcessingInformation dst,
                         const QSize& size,
                         const KisFilterConfiguration* config,
                         KoUpdater* progressUpdater
                        ) const;

    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, const KisImageWSP image) const;
private:
    OpenShiva::Source* m_source;
};

#endif
