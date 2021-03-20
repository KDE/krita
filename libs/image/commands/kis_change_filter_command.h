/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
                       KisFilterConfigurationSP configBefore,
                       KisFilterConfigurationSP configAfter)
            : KUndo2Command(kundo2_i18n("Change Filter")) {
        m_node = node;
        m_filterInterface = dynamic_cast<KisNodeFilterInterface*>(node.data());
        Q_ASSERT(m_filterInterface);

        KIS_SAFE_ASSERT_RECOVER(configBefore->hasLocalResourcesSnapshot()) {
            configBefore->createLocalResourcesSnapshot();
        }

        KIS_SAFE_ASSERT_RECOVER(configAfter->hasLocalResourcesSnapshot()) {
            configAfter->createLocalResourcesSnapshot();
        }

        m_configBefore = configBefore;
        m_configAfter = configAfter;
    }
public:
    void redo() override {
        m_filterInterface->setFilter(m_configAfter);
        m_node->setDirty();
    }

    void undo() override {
        m_filterInterface->setFilter(m_configBefore);
        m_node->setDirty();
    }

private:
    KisNodeSP m_node;
    KisNodeFilterInterface *m_filterInterface;

    KisFilterConfigurationSP m_configBefore;
    KisFilterConfigurationSP m_configAfter;
};
#endif
