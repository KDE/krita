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

#include "ShivaGeneratorConfigWidget.h"

#include <QGridLayout>
#include <OpenShiva/Source.h>
#include <GTLCore/String.h>

#include "filter/kis_filter_configuration.h"
#include "QVariantValue.h"
#include <OpenShiva/Metadata.h>

ShivaGeneratorConfigWidget::ShivaGeneratorConfigWidget(const OpenShiva::Source* _source, QWidget* parent) : KisConfigWidget(parent), m_source(_source), m_widget(new QtShiva::SourceParametersWidget(this))
{
    m_widget->setSource(m_source);
    QGridLayout* gridLayout = new QGridLayout(this);
    gridLayout->addWidget(m_widget, 0, 0, 1, 1);
}

ShivaGeneratorConfigWidget::~ShivaGeneratorConfigWidget()
{
}

void ShivaGeneratorConfigWidget::setConfiguration(const KisPropertiesConfiguration* config)
{
    QMap<QString, QVariant> map = config->getProperties();
    for (QMap<QString, QVariant>::iterator it = map.begin(); it != map.end(); ++it) {
        const GTLCore::Metadata::Entry* entry = m_source->metadata()->parameter(it.key().toAscii().data());
        if (entry and entry->asParameterEntry()) {
            GTLCore::Value val = qvariantToValue(it.value(), entry->asParameterEntry()->valueType());
            if (val.isValid()) {
                m_widget->setParameter(it.key().toAscii().data(), val);
            }
        }
    }
}

KisPropertiesConfiguration* ShivaGeneratorConfigWidget::configuration() const
{
    KisFilterConfiguration* config = new KisFilterConfiguration(m_source->name().c_str(), 1);

    for (std::map<GTLCore::String, GTLCore::Value>::const_iterator it = m_widget->parameters().begin();
            it != m_widget->parameters().end(); ++it) {
        config->setProperty(it->first.c_str(), valueToQVariant(it->second));
    }
    return config;
}

