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
            widget.simple->setChecked(config->getBool("Simple", false));
            widget.localContrast->setValue(config->getDouble("LocalContrastThreshold", 0.5));
            if(config->getInt("Equation", 2) == 2)
            {
                widget.equation->setCurrentIndex(0);
            } else {
                widget.equation->setCurrentIndex(1);
            }
        }
        virtual KisPropertiesConfiguration* configuration() const
        {
            KisPropertiesConfiguration* config = new KisPropertiesConfiguration();
            config->setProperty("Simple", widget.simple->isChecked());
            config->setProperty("LocalContrastThreshold", widget.localContrast->value());
            if(widget.equation->currentIndex() == 0)
                config->setProperty("Equation", 2);
            else
                config->setProperty("Equation", 4);
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
    bool simple = config->getBool("Simple", false);
    double lC = config->getDouble("LocalContrastThreshold", 0.5);
    int eqn = config->getInt("Equation", 2);
    if(eqn != 2 or eqn !=4) eqn = 2;
}
