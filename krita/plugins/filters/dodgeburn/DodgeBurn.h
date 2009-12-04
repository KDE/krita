/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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


#include "filter/kis_color_transformation_filter.h"

#include "kis_config_widget.h"


class KisFilterDodgeBurn : public KisColorTransformationFilter
{
public:
    enum Type {
      SHADOWS,
      MIDTONES,
      HIGHLIGHTS
    };
public:
    KisFilterDodgeBurn(const QString& id, const QString& prefix, const QString& name );
public:

    virtual KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfiguration* config) const;
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, const KisImageWSP image) const;
private:
    QString m_prefix;
};

class Ui_DodgeBurnConfigurationBaseWidget;

class KisDodgeBurnConfigWidget : public KisConfigWidget
{

public:
    KisDodgeBurnConfigWidget(QWidget * parent, const QString& id);
    virtual ~KisDodgeBurnConfigWidget();

    virtual KisPropertiesConfiguration * configuration() const;
    virtual void setConfiguration(const KisPropertiesConfiguration* config);
    QString m_id;
    Ui_DodgeBurnConfigurationBaseWidget * m_page;
};
