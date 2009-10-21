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

#ifndef _SHIVA_GENERATOR_CONFIG_WIDGET_H_
#define _SHIVA_GENERATOR_CONFIG_WIDGET_H_

#include <kis_config_widget.h>
#include <QtShiva/SourceParametersWidget.h>

namespace OpenShiva
{
class Source;
}

class ShivaGeneratorConfigWidget : public KisConfigWidget
{
public:
    ShivaGeneratorConfigWidget(const OpenShiva::Source* _source, QWidget* parent = 0);
    ~ShivaGeneratorConfigWidget();

    virtual void setConfiguration(const KisPropertiesConfiguration*);
    virtual KisPropertiesConfiguration* configuration() const;
private:
    const OpenShiva::Source* m_source;
    QtShiva::SourceParametersWidget* m_widget;
};

#endif
