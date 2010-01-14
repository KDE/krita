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

#include "bigbrother.h"
#include <stdlib.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <kfiledialog.h>
#include <kpluginfactory.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <recorder/kis_action_recorder.h>
#include <kis_config.h>
#include <kis_cursor.h>
#include <kis_debug.h>
#include <kis_global.h>
#include <kis_image.h>
#include <recorder/kis_play_info.h>
#include <recorder/kis_recorded_action.h>
#include <recorder/kis_recorded_action_factory_registry.h>
#include <kis_types.h>
#include <kis_view2.h>

#include "actionseditor/kis_actions_editor.h"
#include "actionseditor/kis_actions_editor_dialog.h"

K_PLUGIN_FACTORY(BigBrotherPluginFactory, registerPlugin<BigBrotherPlugin>();)
K_EXPORT_PLUGIN(BigBrotherPluginFactory("krita"))


BigBrotherPlugin::BigBrotherPlugin(QObject *parent, const QVariantList &)
        : KParts::Plugin(parent), m_recorder(0)
{
    if (parent->inherits("KisView2")) {
        m_view = (KisView2*) parent;

        setComponentData(BigBrotherPluginFactory::componentData());

        setXMLFile(KStandardDirs::locate("data", "kritaplugins/bigbrother.rc"), true);

        KAction* action = 0;
        // Open and play action
        action  = new KAction(KIcon("media-playback-start"), i18n("Open and play..."), this);
        actionCollection()->addAction("Macro_Open_Play", action);
        connect(action, SIGNAL(triggered()), this, SLOT(slotOpenPlay()));
        // Open and edit action
        action  = new KAction(KIcon("document-edit"), i18n("Open and edit..."), this);
        actionCollection()->addAction("Macro_Open_Edit", action);
        connect(action, SIGNAL(triggered()), this, SLOT(slotOpenEdit()));
        // Save recorded action
        action  = new KAction(i18n("Save all actions"), this);
        actionCollection()->addAction("Recording_Global_Save", action);
        connect(action, SIGNAL(triggered()), this, SLOT(slotSave()));
        // Start recording action
        m_startRecordingMacroAction = new KAction(KIcon("media-record"), i18n("Start recording macro"), this);
        actionCollection()->addAction("Recording_Start_Recording_Macro", m_startRecordingMacroAction);
        connect(m_startRecordingMacroAction, SIGNAL(triggered()), this, SLOT(slotStartRecordingMacro()));
        // Save recorded action
        m_stopRecordingMacroAction  = new KAction(KIcon("media-playback-stop"), i18n("Stop recording actions"), this);
        actionCollection()->addAction("Recording_Stop_Recording_Macro", m_stopRecordingMacroAction);
        connect(m_stopRecordingMacroAction, SIGNAL(triggered()), this, SLOT(slotStopRecordingMacro()));
        m_stopRecordingMacroAction->setEnabled(false);
    }
}

BigBrotherPlugin::~BigBrotherPlugin()
{
    m_view = 0;
    delete m_recorder;
}

void BigBrotherPlugin::slotSave()
{
    saveMacro(m_view->image()->actionRecorder(), KUrl());
}

void BigBrotherPlugin::slotOpenPlay()
{
    KisMacro* m = openMacro();
    if (!m) return;
    dbgPlugins << "Play the macro";
    m->play(KisPlayInfo(m_view->image(), m_view->activeNode()));
    dbgPlugins << "Finished";
    delete m;
}


void BigBrotherPlugin::slotOpenEdit()
{
    KUrl url;
    KisMacro* m = openMacro(&url);
    if (!m) return;
    KisActionsEditorDialog aed(m_view);

    aed.actionsEditor()->setMacro(m);

    if (aed.exec() == QDialog::Accepted) {
        saveMacro(m, url);
    }

    delete m;
}

void BigBrotherPlugin::slotStartRecordingMacro()
{
    dbgPlugins << "Start recording macro";
    if (m_recorder) return;
    // Alternate actions
    m_startRecordingMacroAction->setEnabled(false);
    m_stopRecordingMacroAction->setEnabled(true);

    // Create recorder
    m_recorder = new KisMacro();
    connect(m_view->image()->actionRecorder(), SIGNAL(addedAction(const KisRecordedAction&)),
            m_recorder, SLOT(addRecordedAction(const KisRecordedAction&)));
}

void BigBrotherPlugin::slotStopRecordingMacro()
{
    dbgPlugins << "Stop recording macro";
    if (!m_recorder) return;
    // Alternate actions
    m_startRecordingMacroAction->setEnabled(true);
    m_stopRecordingMacroAction->setEnabled(false);
    // Save the macro
    saveMacro(m_recorder, KUrl());
    // Delete recorder
    delete m_recorder;
    m_recorder = 0;
}

KisMacro* BigBrotherPlugin::openMacro(KUrl* url)
{

    Q_UNUSED(url);

    QString filename = KFileDialog::getOpenFileName(KUrl(), "*.krarec|Recorded actions (*.krarec)", m_view);
    if (!filename.isNull()) {
        QDomDocument doc;
        QFile f(filename);
        if (f.exists()) {
            dbgPlugins << f.open(QIODevice::ReadOnly);
            QString err;
            int line, col;
            if (!doc.setContent(&f, &err, &line, &col)) {
                // TODO error message
                dbgPlugins << err << " line = " << line << " col = " << col;
                f.close();
                return 0;
            }
            f.close();
            QDomElement docElem = doc.documentElement();
            if (!docElem.isNull() && docElem.tagName() == "RecordedActions") {
                dbgPlugins << "Load the macro";
                KisMacro* m = new KisMacro();
                m->fromXML(docElem);
                return m;
            } else {
                // TODO error message
            }
        } else {
            dbgPlugins << "Unexistant file : " << filename;
        }
    }
    return 0;
}

void BigBrotherPlugin::saveMacro(const KisMacro* macro, const KUrl& url)
{
    QString filename = KFileDialog::getSaveFileName(url, "*.krarec|Recorded actions (*.krarec)", m_view);
    if (!filename.isNull()) {
        QDomDocument doc;
        QDomElement e = doc.createElement("RecordedActions");

        macro->toXML(doc, e);

        doc.appendChild(e);
        QFile f(filename);
        f.open(QIODevice::WriteOnly);
        QTextStream stream(&f);
        doc.save(stream, 2);
        f.close();
    }
}

#include "bigbrother.moc"
