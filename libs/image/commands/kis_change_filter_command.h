/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_CHANGE_FILTER_COMMAND_H
#define KIS_CHANGE_FILTER_COMMAND_H

#include <kundo2command.h>
#include <QRect>
#include "kis_types.h"
#include <klocalizedstring.h>
#include "filter/kis_filter_configuration.h"
#include "kis_node.h"
#include "kis_node_filter_interface.h"

#include "filter/kis_filter_registry.h"
#include "filter/kis_filter.h"
#include "generator/kis_generator_registry.h"
#include "generator/kis_generator.h"


class KisChangeFilterCmd : public KUndo2Command
{

public:
    KisChangeFilterCmd(KisNodeSP node,
                       const QString &filterNameBefore,
                       const QString &xmlBefore,
                       const QString &filterNameAfter,
                       const QString &xmlAfter,
                       bool useGeneratorRegistry)
            : KUndo2Command(kundo2_i18n("Change Filter")) {
        m_node = node;
        m_filterInterface = dynamic_cast<KisNodeFilterInterface*>(node.data());
        Q_ASSERT(m_filterInterface);

        m_useGeneratorRegistry = useGeneratorRegistry;

        m_xmlBefore = xmlBefore;
        m_xmlAfter = xmlAfter;
        m_filterNameBefore = filterNameBefore;
        m_filterNameAfter = filterNameAfter;
    }
public:
    void redo() override {
        m_filterInterface->setFilter(createConfiguration(m_filterNameAfter, m_xmlAfter));
        m_node->setDirty();
    }

    void undo() override {
        m_filterInterface->setFilter(createConfiguration(m_filterNameBefore, m_xmlBefore));
        m_node->setDirty();
    }

private:
    KisFilterConfigurationSP createConfiguration(const QString &name, const QString &data)
    {
        KisFilterConfigurationSP config;

        if (m_useGeneratorRegistry) {
            KisGeneratorSP generator = KisGeneratorRegistry::instance()->value(name);
            config = generator->factoryConfiguration();
        } else {
            KisFilterSP filter = KisFilterRegistry::instance()->value(name);
            config = filter->factoryConfiguration();
        }

        config->fromXML(data);
        return config;
}

private:
    KisNodeSP m_node;
    KisNodeFilterInterface *m_filterInterface;

    bool m_useGeneratorRegistry;

    QString m_xmlBefore;
    QString m_xmlAfter;
    QString m_filterNameBefore;
    QString m_filterNameAfter;
};
#endif
