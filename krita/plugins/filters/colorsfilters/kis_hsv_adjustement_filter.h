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


#ifndef _KIS_HSV_ADJUSTEMENT_FILTER_H_
#define _KIS_HSV_ADJUSTEMENT_FILTER_H_

#include <QList>

#include "kis_filter.h"
#include "kis_filter_config_widget.h"
#include "ui_wdg_hsv_adjustement.h"

class QWidget;
class KoColorTransformation;

/**
 * This class affect Intensity Y of the image
 */
class KisHSVAdjustementFilter : public KisFilter
{

public:

    KisHSVAdjustementFilter();

public:

    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev) const;
    void process(KisFilterConstantProcessingInformation src,
                 KisFilterProcessingInformation dst,
                 const QSize& size,
                 const KisFilterConfiguration* config,
                 KoUpdater* progressUpdater = 0
        ) const;
    static inline KoID id() { return KoID("hsvadjustement", i18n("HSV Adjustement")); }
    virtual KisFilterConfiguration* factoryConfiguration(const KisPaintDeviceSP) const;
    
};


class KisHSVConfigWidget : public KisFilterConfigWidget {

public:
    KisHSVConfigWidget(QWidget * parent, Qt::WFlags f = 0 );
    virtual ~KisHSVConfigWidget();

    virtual KisFilterConfiguration * configuration() const;
    virtual void setConfiguration( KisFilterConfiguration * config );
    Ui_WdgHSVAdjustement * m_page;
};

#endif
