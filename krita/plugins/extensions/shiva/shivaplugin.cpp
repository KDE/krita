/*
 * Copyright (c) 2008-2009 Cyrille Berger <cberger@cberger.net>
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

#include "shivaplugin.h"

#include <kis_debug.h>
#include <kgenericfactory.h>
#include <kstandarddirs.h>

#include <generator/kis_generator_registry.h>

#include <OpenShiva/SourcesCollection.h>

#include "shivagenerator.h"

typedef KGenericFactory<ShivaPlugin> ShivaPluginFactory;
K_EXPORT_COMPONENT_FACTORY(kritashiva, ShivaPluginFactory("krita"))

ShivaPlugin::ShivaPlugin(QObject *parent, const QStringList &)
        : KParts::Plugin(parent)
{
    setComponentData( ShivaPluginFactory::componentData());
    KisGeneratorRegistry * manager = KisGeneratorRegistry::instance();
    Q_ASSERT(manager);
    OpenShiva::SourcesCollection* kc = new OpenShiva::SourcesCollection();
    std::list< OpenShiva::Source* > kernels = kc->sources(OpenShiva::Source::GeneratorKernel);
    
    foreach( OpenShiva::Source* kernel, kernels )
    {
    // kDebug() << kernel->metadataCompilationErrors().c_str() << " " << kernel->isCompiled() ;
        manager->add(new ShivaGenerator( kernel ));
    }
}

ShivaPlugin::~ShivaPlugin()
{
}

