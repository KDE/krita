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
#include <kdebug.h>
#include <kfiledialog.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <kis_action_recorder.h>
#include <kis_config.h>
#include <kis_cursor.h>
#include <kis_global.h>
#include <kis_image.h>
#include <kis_recorded_action.h>
#include <kis_recorded_action_factory_registry.h>
#include <kis_types.h>
#include <kis_view2.h>

typedef KGenericFactory<BigBrotherPlugin> BigBrotherPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritabigbrother, BigBrotherPluginFactory( "krita" ) )


BigBrotherPlugin::BigBrotherPlugin(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
{
    if ( parent->inherits("KisView2") )
    {
        m_view = (KisView2*) parent;

        setComponentData(BigBrotherPluginFactory::componentData());

        setXMLFile(KStandardDirs::locate("data","kritaplugins/bigbrother.rc"), true);

        // Replay recording action
        KAction* action  = new KAction(i18n("Replay"), this);
        actionCollection()->addAction("Recording_Replay", action );
        connect(action, SIGNAL(triggered()), this, SLOT(slotReplay()));
        // Save recorded action
        action  = new KAction(i18n("Save"), this);
        actionCollection()->addAction("Recording_Save", action );
        connect(action, SIGNAL(triggered()), this, SLOT(slotSave()));
        // Save recorded action
        action  = new KAction(i18n("Open"), this);
        actionCollection()->addAction("Recording_Open", action );
        connect(action, SIGNAL(triggered()), this, SLOT(slotOpen()));
    }
}

BigBrotherPlugin::~BigBrotherPlugin()
{
    m_view = 0;
}

void BigBrotherPlugin::slotReplay()
{
    KisActionRecorder* actionRecorder = m_view->image()->actionRecorder();
    QList<KisRecordedAction*> actions = actionRecorder->actions();
    for( QList<KisRecordedAction*>::iterator it = actions.begin();
         it != actions.end(); ++it)
    {
        (*it)->play();
    }
}

void BigBrotherPlugin::slotSave()
{
    QString filename = KFileDialog::getSaveFileName(KUrl(), "*.krarec|Recorded actions (*.krarec)", m_view);
    if(not filename.isNull())
    {
        QDomDocument doc;
        QDomElement e = doc.createElement("RecordedActions");
        
        KisActionRecorder* actionRecorder = m_view->image()->actionRecorder();
        QList<KisRecordedAction*> actions = actionRecorder->actions();
        for( QList<KisRecordedAction*>::iterator it = actions.begin();
            it != actions.end(); ++it)
        {
            QDomElement eAct = doc.createElement("RecordedAction");
            (*it)->toXML(doc, eAct);
            e.appendChild(eAct);
        }
        
        doc.appendChild(e);
        QFile f(filename);
        f.open( QIODevice::WriteOnly);
        QTextStream stream(&f);
        doc.save(stream,2);
        f.close();
    }
}

void BigBrotherPlugin::slotOpen()
{
    QString filename = KFileDialog::getOpenFileName(KUrl(), "*.krarec|Recorded actions (*.krarec)", m_view);
    if(not filename.isNull())
    {
        QDomDocument doc;
        QFile f(filename);
        if(f.exists())
        {
            kDebug() << f.open( QIODevice::ReadOnly);
            QString err;
            int line, col;
            if(not doc.setContent(&f, &err, &line, &col))
            {
                // TODO error message
                kDebug() << err << " line = " << line << " col = " << col;
                f.close();
                return;
            }
            f.close();
            QDomElement docElem = doc.documentElement();
            if(not docElem.isNull() and docElem.tagName() == "RecordedActions")
            {
                QDomNode node = docElem.firstChild();
                while(not node.isNull()) {
                    QDomElement elt = node.toElement(); // try to convert the node to an element.
                    if(not elt.isNull() and elt.tagName() == "RecordedAction") {
                        QString id = elt.attribute("id", "");
                        if(not id.isNull())
                        {
                            kDebug() << "Reconstruct : " << id << endl; // the node really is an element.
                            KisRecordedActionFactory* raf = KisRecordedActionFactoryRegistry::instance()->get(id);
                            KisRecordedAction* ra = raf->fromXML( m_view->image(), elt);
                            ra->play();
                            delete ra;
                        } else {
                            kDebug() << "Invalid recorded action: null id";
                        }
                    } else {
                        kDebug() << "Unknown element " << elt.tagName() << (elt.tagName() == "RecordedAction");
                    }
                    node = node.nextSibling();
                }
            } else {
                // TODO error message
            }
        } else {
            kDebug() << "Unexistant file";
        }
    }
}

#include "bigbrother.moc"
