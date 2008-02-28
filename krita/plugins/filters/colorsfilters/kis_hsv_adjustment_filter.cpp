/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; version 2
 * of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "kis_hsv_adjustment_filter.h"


#include <KoProgressUpdater.h>

#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <filter/kis_filter_processing_information.h>

KisHSVAdjustmentFilter::KisHSVAdjustmentFilter()
    : KisFilter( id(), CategoryAdjust, i18n("&HSV Adjustment..."))
{
    setSupportsPainting(true);
    setSupportsPreview(true);
    setSupportsIncrementalPainting(false);
}

KisFilterConfigWidget * KisHSVAdjustmentFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev) const
{
    Q_UNUSED(dev);
    return new KisHSVConfigWidget(parent);
}

void KisHSVAdjustmentFilter::process(KisFilterConstProcessingInformation srcInfo,
                 KisFilterProcessingInformation dstInfo,
                 const QSize& size,
                 const KisFilterConfiguration* config,
                 KoUpdater* progressUpdater
        ) const
{
    const KisPaintDeviceSP src = srcInfo.paintDevice();
    KisPaintDeviceSP dst = dstInfo.paintDevice();
    QPoint dstTopLeft = dstInfo.topLeft();
    QPoint srcTopLeft = srcInfo.topLeft();
    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);
    QHash<QString, QVariant> params;
    if(config)
    {
        params["h"] = config->getInt("h", 0) / 180.0;
        params["s"] = config->getInt("s", 0) * 0.01;
        params["v"] = config->getInt("v", 0) * 0.01;
    }
    KoColorTransformation* transfo = src->colorSpace()->createColorTransformation( "hsv_adjustment", params);
    if( !transfo )
    {
        kError() << "hsv_adjustment transformation is unavailable, go check your installation";
        return;
    }
    int count = 0;
    int cost = size.width() * size.height();
    if( cost == 0 ) cost = 1;
    KisHLineConstIteratorPixel srcIt = src->createHLineConstIterator(srcTopLeft.x(), srcTopLeft.y(), size.width());
    KisHLineIteratorPixel dstIt = dst->createHLineIterator(dstTopLeft.x(), dstTopLeft.y(), size.width());

    for ( int row = 0; row < size.height(); ++row ) {
        while ( !srcIt.isDone() ) {
            if ( srcIt.isSelected() ) {
                transfo->transform( srcIt.oldRawData(), dstIt.rawData(), 1);
            }
            ++srcIt;
            ++dstIt;
            if(progressUpdater) progressUpdater->setProgress( (++count) / cost);

        }
        srcIt.nextRow();
        dstIt.nextRow();
    }
           
    delete transfo;
}

KisFilterConfiguration* KisHSVAdjustmentFilter::factoryConfiguration(const KisPaintDeviceSP) const
{
    return new KisFilterConfiguration(id().id(), 0);
}

KisHSVConfigWidget::KisHSVConfigWidget(QWidget * parent, Qt::WFlags f ) : KisFilterConfigWidget(parent, f)
{
    m_page = new Ui_WdgHSVAdjustment();
    m_page->setupUi(this);
    connect(m_page->hue, SIGNAL(valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect(m_page->value, SIGNAL(valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect(m_page->saturation, SIGNAL(valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
}

KisHSVConfigWidget::~KisHSVConfigWidget()
{
    delete m_page;
}

KisFilterConfiguration * KisHSVConfigWidget::configuration() const
{
    KisFilterConfiguration* c = new KisFilterConfiguration(KisHSVAdjustmentFilter::id().id(), 0);
    c->setProperty( "h", m_page->hue->value() );
    c->setProperty( "s", m_page->saturation->value() );
    c->setProperty( "v", m_page->value->value() );
    return c;
}

void KisHSVConfigWidget::setConfiguration( KisFilterConfiguration * config )
{
    m_page->hue->setValue( config->getInt( "h", 0 ) );
    m_page->saturation->setValue( config->getInt( "s", 0 ) );
    m_page->value->setValue( config->getInt("v", 0 ) );
}
