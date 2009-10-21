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

#ifndef _SHIVAGENERATOR_H_
#define _SHIVAGENERATOR_H_

#include <kparts/plugin.h>
#include "generator/kis_generator.h"

class KisConfigWidget;

namespace OpenShiva
{
class Source;
};

class ShivaGenerator : public KisGenerator
{
public:

    ShivaGenerator(OpenShiva::Source* source);

    using KisGenerator::generate;

    void generate(KisProcessingInformation dst,
                  const QSize& size,
                  const KisFilterConfiguration* config,
                  KoUpdater* progressUpdater
                 ) const;
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, const KisImageWSP image) const;

private:
    OpenShiva::Source* m_source;
};

#endif
