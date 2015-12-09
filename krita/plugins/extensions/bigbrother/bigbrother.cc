/*
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "bigbrother.h"
#include <cstdlib>
#include <unistd.h>

#include <kis_action.h>

#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <KoIcon.h>
#include <kis_icon.h>
#include <KoUpdater.h>
#include <KoResourceServerProvider.h>
#include <KoFileDialog.h>

#include <kis_config.h>
#include <kis_cursor.h>
#include <kis_debug.h>
#include <kis_global.h>
#include <kis_image.h>
#include <kis_resource_server_provider.h>
#include <kis_types.h>
#include <KisViewManager.h>
#include <KoPattern.h>
#include <recorder/kis_action_recorder.h>
#include <recorder/kis_macro.h>
#include <recorder/kis_macro_player.h>
#include <recorder/kis_play_info.h>
#include <recorder/kis_recorded_action_factory_registry.h>
#include <recorder/kis_recorded_action.h>
#include <recorder/kis_recorded_action_load_context.h>
#include <recorder/kis_recorded_action_save_context.h>

#include "actionseditor/kis_actions_editor.h"
#include "actionseditor/kis_actions_editor_dialog.h"

#include <QDesktopServices>
#include <QApplication>


K_PLUGIN_FACTORY_WITH_JSON(BigBrotherPluginFactory, "kritabigbrother.json", registerPlugin<BigBrotherPlugin>();)

class RecordedActionSaveContext : public KisRecordedActionSaveContext {
    public:
        virtual void saveGradient(const KoAbstractGradient* ) {}
        virtual void savePattern(const KoPattern* ) {}
};

class RecordedActionLoadContext : public KisRecordedActionLoadContext {
    public:
        virtual KoAbstractGradient* gradient(const QString& name) const
        {
            return KoResourceServerProvider::instance()->gradientServer()->resourceByName(name);
        }
        virtual KoPattern* pattern(const QString& name) const
        {
            return KoResourceServerProvider::instance()->patternServer()->resourceByName(name);
        }
};

BigBrotherPlugin::BigBrotherPlugin(QObject *parent, const QVariantList &)
        : KisViewPlugin(parent)
        , m_recorder(0)
{
    if (parent->inherits("KisViewManager")) {
        m_view = (KisViewManager*) parent;

        // Open and play action
        KisAction* action  = createAction("Macro_Open_Play");
        connect(action, SIGNAL(triggered()), this, SLOT(slotOpenPlay()));

        // Open and edit action
        action = createAction("Macro_Open_Edit");
        connect(action, SIGNAL(triggered()), this, SLOT(slotOpenEdit()));

        // Start recording action
        m_startRecordingMacroAction = createAction("Recording_Start_Recording_Macro");
        connect(m_startRecordingMacroAction, SIGNAL(triggered()), this, SLOT(slotStartRecordingMacro()));

        // Save recorded action
        m_stopRecordingMacroAction = createAction("Recording_Stop_Recording_Macro");
        connect(m_stopRecordingMacroAction, SIGNAL(triggered()), this, SLOT(slotStopRecordingMacro()));
        m_stopRecordingMacroAction->setEnabled(false);
    }
}

BigBrotherPlugin::~BigBrotherPlugin()
{
    m_view = 0;
    delete m_recorder;
}

void BigBrotherPlugin::slotOpenPlay()
{
    KisMacro* m = openMacro();
    dbgKrita << m;
    if (!m) return;
    dbgPlugins << "Play the macro";
    KoProgressUpdater* updater = m_view->createProgressUpdater();
    updater->start(1, i18n("Playing back macro"));
    KisMacroPlayer player(m, KisPlayInfo(m_view->image(), m_view->activeNode()), updater->startSubtask());
    player.start();
    while(player.isRunning())
    {
        QApplication::processEvents();
    }
    dbgPlugins << "Finished";
    delete m;
}


void BigBrotherPlugin::slotOpenEdit()
{
    KisMacro *macro = openMacro();
    if (!macro) return;
    KisActionsEditorDialog aed(m_view->mainWindow());

    aed.actionsEditor()->setMacro(macro);

    if (aed.exec() == QDialog::Accepted) {
        saveMacro(macro);
    }

    delete macro;
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
            m_recorder, SLOT(addAction(const KisRecordedAction&)));
}

void BigBrotherPlugin::slotStopRecordingMacro()
{
    dbgPlugins << "Stop recording macro";
    if (!m_recorder) return;
    // Alternate actions
    m_startRecordingMacroAction->setEnabled(true);
    m_stopRecordingMacroAction->setEnabled(false);
    // Save the macro
    saveMacro(m_recorder);
    // Delete recorder
    delete m_recorder;
    m_recorder = 0;
}

KisMacro* BigBrotherPlugin::openMacro()
{
    QStringList mimeFilter;
    mimeFilter << "*.krarec|Recorded actions (*.krarec)";

    KoFileDialog dialog(m_view->mainWindow(), KoFileDialog::OpenFile, "OpenDocument");
    dialog.setCaption(i18n("Open Macro"));
    dialog.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation));
    dialog.setNameFilter(i18n("Recorded actions (*.krarec)"));
    QString filename = dialog.filename();
    RecordedActionLoadContext loadContext;

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
                m->fromXML(docElem, &loadContext);
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

void BigBrotherPlugin::saveMacro(const KisMacro* macro)
{
    KoFileDialog dialog(m_view->mainWindow(), KoFileDialog::SaveFile, "krita/bigbrother");
    dialog.setCaption(i18n("Save Macro"));
    dialog.setNameFilter(i18n("Recorded actions (*.krarec)"));

    QString filename = dialog.filename();

    if (!filename.isNull()) {
        QDomDocument doc;
        QDomElement e = doc.createElement("RecordedActions");
        RecordedActionSaveContext context;
        macro->toXML(doc, e, &context);

        doc.appendChild(e);
        QFile f(filename);
        f.open(QIODevice::WriteOnly);
        QTextStream stream(&f);
        stream.setCodec("UTF-8");
        doc.save(stream, 2);
        f.close();
    }
}

#include "bigbrother.moc"
