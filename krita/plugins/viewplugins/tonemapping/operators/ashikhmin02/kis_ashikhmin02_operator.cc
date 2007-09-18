/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2 of the License.
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

#include "kis_ashikhmin02_operator.h"

#include <kis_properties_configuration.h>

#include <kis_tone_mapping_operator_configuration_widget.h>

#include "ui_ashikhmin02_configuration_widget.h"

class KisAshikhmin02OperatorConfigurationWidget : public KisToneMappingOperatorConfigurationWidget{
    public:
        KisAshikhmin02OperatorConfigurationWidget(QWidget* wdg) : KisToneMappingOperatorConfigurationWidget(wdg)
        {
            widget.setupUi(this);
        }
        virtual void setConfiguration(KisPropertiesConfiguration* config)
        {
        }
        virtual KisPropertiesConfiguration* configuration() const
        {
            KisPropertiesConfiguration* config = new KisPropertiesConfiguration();
            return config;
        }
    private:
        Ui_Ashikhmin02OperatorConfigurationWidget widget;
};

KisAshikhmin02Operator::KisAshikhmin02Operator() : KisToneMappingOperator("ashikhminO2", i18n("Ashikhmin 02"))
{
}

KisToneMappingOperatorConfigurationWidget* KisAshikhmin02Operator::createConfigurationWidget(QWidget* wdg) const
{
    return new KisAshikhmin02OperatorConfigurationWidget(wdg);
}

void KisAshikhmin02Operator::toneMap(KisPaintDeviceSP, KisPropertiesConfiguration* config) const
{
    
}
