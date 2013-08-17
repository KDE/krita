/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_gmic_plugin.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <kactioncollection.h>

#include <kis_view2.h>
#include <kis_action.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_layer.h>
#include <kis_image_signal_router.h>
#include <kis_processing_applicator.h>
#include "kis_statusbar.h"
#include "widgets/kis_progress_widget.h"

#include <KoColorSpace.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>


#include "kis_gmic.h"
#include "dlg_gmic.h"

K_PLUGIN_FACTORY(KisGmicPluginFactory, registerPlugin<KisGmicPlugin>();)
K_EXPORT_PLUGIN(KisGmicPluginFactory("krita"))

#include "gmic.h"
#include "kis_gmic_parser.h"
#include "Component.h"
#include "kis_gmic_filter_model.h"
#include "kis_gmic_widget.h"
#include "kis_gmic_processing_visitor.h"

KisGmicPlugin::KisGmicPlugin(QObject *parent, const QVariantList &)
        : KisViewPlugin(parent, "kritaplugins/gmic.rc"),m_gmicWidget(0)
{
    qDebug() << "\tGMIC\tLoading GMIC";
    KisAction *action  = new KisAction(i18n("Apply G'Mic Action..."), this);
    action->setActivationFlags(KisAction::ACTIVE_LAYER);
    action->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE);
    addAction("gmic", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotGmic()));
}

KisGmicPlugin::~KisGmicPlugin()
{
    delete m_gmicWidget;
}

void KisGmicPlugin::slotGmic()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisLayerSP layer = m_view->activeLayer();
    if (!layer) return;

    KisGmicParser parser("gmic_def.gmic");
    Component * root = parser.createFilterTree();
    KisGmicFilterModel * model = new KisGmicFilterModel(root);
    m_gmicWidget = new KisGmicWidget(model);

    connect(m_gmicWidget, SIGNAL(sigApplyCommand(KisGmicFilterSetting*)),this, SLOT(slotApplyGmicCommand(KisGmicFilterSetting*)));
    m_gmicWidget->show();
}


void KisGmicPlugin::slotApplyGmicCommand(KisGmicFilterSetting* setting)
{
    KisImageWSP image = m_view->image();

    if (image)
    {
        QString actionName;
        KisNodeSP node;

        if (setting->inputLayerMode() == ACTIVE_LAYER)
        {
            actionName = i18n("Gmic: Active Layer");
            node = m_view->activeNode();
        }
        else
        {
            KMessageBox::sorry(m_gmicWidget, i18n("Sorry, this input mode is not implemented"), i18n("Krita"));
            return;
        }

        qDebug() << "Input mode" << setting->inputLayerMode();
        qDebug() << "Output mode" << setting->outputMode();

        if (setting->outputMode() != IN_PLACE)
        {
            KMessageBox::sorry(m_gmicWidget,QString("Sorry, this output mode is not implemented"),"Krita");
            return;
        }

        KisImageSignalVector emitSignals;
        emitSignals << ModifiedSignal;

        KisProcessingApplicator applicator(m_view->image(), node,
                                       KisProcessingApplicator::RECURSIVE,
                                       emitSignals, actionName);


        KisProcessingVisitorSP visitor = new KisGmicProcessingVisitor(setting->gmicCommand(), m_view);
        applicator.applyVisitor(visitor, KisStrokeJobData::CONCURRENT);
        applicator.end();
    }
}


#include "kis_gmic_plugin.moc"
