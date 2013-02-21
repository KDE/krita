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

#include "shivafilter.h"

#include <QMutex>

#include <KoUpdater.h>

#include <kis_paint_device.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <ShivaGeneratorConfigWidget.h>
#include "PaintDeviceImage.h"
#include "QVariantValue.h"
#include <OpenShiva/Kernel.h>
#include <OpenShiva/Source.h>
#include <GTLFragment/Metadata.h>
#include <GTLCore/Region.h>

#include "UpdaterProgressReport.h"
#include <kis_gtl_lock.h>

extern QMutex* shivaMutex;

ShivaFilter::ShivaFilter(OpenShiva::Source* kernel) : KisFilter(KoID(kernel->name().c_str(), kernel->name().c_str()), categoryOther(), kernel->name().c_str()), m_source(kernel)
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(false);
    setSupportsIncrementalPainting(false);
}

ShivaFilter::~ShivaFilter()
{
}

KisConfigWidget* ShivaFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, const KisImageWSP image) const
{
    Q_UNUSED(dev);
    Q_UNUSED(image);
    return new ShivaGeneratorConfigWidget(m_source, parent);
}

void ShivaFilter::process(KisPaintDeviceSP dev,
                          const QRect& size,
                          const KisFilterConfiguration* config,
                          KoUpdater* progressUpdater
                         ) const
{
    Q_UNUSED(progressUpdater);
    QPoint dstTopLeft = size.topLeft();

    UpdaterProgressReport* report = 0;
    if (progressUpdater) {
        progressUpdater->setRange(0, size.height());
        report = new UpdaterProgressReport(progressUpdater);
    }
    
    Q_ASSERT(!dev.isNull());
//     Q_ASSERT(config);
    // TODO support for selection
    OpenShiva::Kernel kernel;
    kernel.setSource(*m_source);
    if (config) {
        QMap<QString, QVariant> map = config->getProperties();
        for (QMap<QString, QVariant>::iterator it = map.begin(); it != map.end(); ++it) {
            dbgPlugins << it.key() << " " << it.value();
            const GTLCore::Metadata::Entry* entry = kernel.metadata()->parameter(it.key().toLatin1().constData());
            if (entry && entry->asParameterEntry()) {
                GTLCore::Value val = qvariantToValue(it.value(), entry->asParameterEntry()->type());
                if(val.isValid())
                {
                    kernel.setParameter(it.key().toLatin1().constData(), val);
                }
            }
        }
    }
    
    kernel.setParameter(OpenShiva::Kernel::IMAGE_WIDTH, float(dev->defaultBounds()->bounds().width()));
    kernel.setParameter(OpenShiva::Kernel::IMAGE_HEIGHT, float(dev->defaultBounds()->bounds().height()));
    
    KisGtlLocker gtlLocker;
    {
        dbgPlugins << "Compile: " << m_source->name().c_str();
        QMutexLocker l(shivaMutex);
        kernel.compile();
    }
    if (kernel.isCompiled()) {
        ConstPaintDeviceImage pdisrc(dev);
        PaintDeviceImage pdi(dev);
        std::list< const GTLCore::AbstractImage* > inputs;
        GTLCore::RegionI region(dstTopLeft.x(), dstTopLeft.y() , size.width(), size.height());
        inputs.push_back(&pdisrc);
        dbgPlugins << "Run: " << m_source->name().c_str() << " " <<  dstTopLeft << " " << size;
        kernel.evaluatePixels(region, inputs, &pdi, report );

        }
}
