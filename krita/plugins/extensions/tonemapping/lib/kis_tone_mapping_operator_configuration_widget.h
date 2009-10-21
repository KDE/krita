/*
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_TONE_MAPPING_OPERATOR_CONFIGURATION_WIDGET_H_
#define _KIS_TONE_MAPPING_OPERATOR_CONFIGURATION_WIDGET_H_

#include <QWidget>

class KisPropertiesConfiguration;

/**
 * Base class of configuration widget of tonemapping operators.
 */
class KisToneMappingOperatorConfigurationWidget : public QWidget
{
    Q_OBJECT
protected:
    KisToneMappingOperatorConfigurationWidget(QWidget * parent, Qt::WFlags f = 0);

public:
    virtual ~KisToneMappingOperatorConfigurationWidget();

    /**
    * @param config the configuration widget for this tonemapping operator.
    */
    virtual void setConfiguration(const KisPropertiesConfiguration* config) = 0;

    /**
    * @return the tonemapping operator configuration
    */
    virtual KisPropertiesConfiguration* configuration() const = 0;

signals:

    /**
    * Subclasses should emit this signal whenever the preview should be
    * be recalculated.
    */
    void sigConfigurationItemChanged();

};


#endif
