/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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

#include "metadataeditor.h"

#include <stdlib.h>

#include <QDialog>
#include <QUiLoader>
#include <QVBoxLayout>

#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_view2.h"

#include "kis_meta_data_store.h"
#include "kis_meta_data_entry.h"
#include "kis_meta_data_value.h"
#include "kis_meta_data_schema.h"

#include "kis_entry_editor.h"

typedef KGenericFactory<metadataeditorPlugin> metadataeditorPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritametadataeditor, metadataeditorPluginFactory( "krita" ) )

metadataeditorPlugin::metadataeditorPlugin(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
{
    if ( parent->inherits("KisView2") )
    {
        m_view = (KisView2*) parent;

        setComponentData(metadataeditorPluginFactory::componentData());

        setXMLFile(KStandardDirs::locate("data","kritaplugins/metadataeditor.rc"), true);

        KAction *action  = new KAction(i18n("&Edit metadata"), this);
        actionCollection()->addAction("metadataeditor", action );
        connect(action, SIGNAL(triggered()), this, SLOT(slotMyAction()));
    }
/*
    m_metaDataStore = new KisMetaData::Store();
    const KisMetaData::Schema* dcSchema = m_metaDataStore->createSchema( KisMetaData::Schema::DublinCoreSchema);
    const KisMetaData::Schema* dcHistory = m_metaDataStore->createSchema( "http://history/", "history");
    const KisMetaData::Schema* dcHow =m_metaDataStore->createSchema( "http://how/", "how");
    m_metaDataStore->addEntry( KisMetaData::Entry("name", dcSchema, KisMetaData::Value("tester")));
    m_metaDataStore->addEntry( KisMetaData::Entry("description", dcSchema, KisMetaData::Value("Test of metadata editing")) );
    m_metaDataStore->addEntry( KisMetaData::Entry("when", dcHistory, KisMetaData::Value(QDate())) );
    m_metaDataStore->addEntry( KisMetaData::Entry("many", dcHow, KisMetaData::Value(42)) );
*/
}

metadataeditorPlugin::~metadataeditorPlugin()
{
    m_view = 0;
}

struct ConnectionInfo {
    QString editorName;
    QString qualifiedName;
    QString editorSignal;
    QString propertyName;
};

void metadataeditorPlugin::slotMyAction()
{
    QUiLoader loader;
    QFile file(KStandardDirs::locate("data","kritaplugins/metadataeditor/testeditor.ui"));
    file.open(QFile::ReadOnly);
    QDialog *dialog = dynamic_cast<QDialog*>(loader.load(&file, m_view));
    file.close();
    QList<ConnectionInfo> connectionInfos;
    {
        ConnectionInfo ci;
        ci.editorName = "editName";
        ci.qualifiedName = "dc:name";
        ci.editorSignal = "2textEdited(const QString&)";
        ci.propertyName = "text";
        connectionInfos.push_back(ci);
    }
    {
        ConnectionInfo ci;
        ci.editorName = "editDescription";
        ci.qualifiedName = "dc:description";
        ci.editorSignal = "2textChanged()";
        ci.propertyName = "plainText";
        connectionInfos.push_back(ci);
    }
    {
        ConnectionInfo ci;
        ci.editorName = "editDate";
        ci.qualifiedName = "history:when";
        ci.editorSignal = "2editingFinished()";
        ci.propertyName = "date";
        connectionInfos.push_back(ci);
    }
    {
        ConnectionInfo ci;
        ci.editorName = "editNumber";
        ci.qualifiedName = "how:many";
        ci.editorSignal = "2editingFinished()";
        ci.propertyName = "value";
        connectionInfos.push_back(ci);
    }
    
    
    foreach(ConnectionInfo ci, connectionInfos)
    {
        QWidget* obj = dialog->findChild<QWidget*>(ci.editorName);
        KisMetaData::Value& value = m_metaDataStore->getEntry(ci.qualifiedName).value();
        KisEntryEditor* ee = new KisEntryEditor( obj, &value, ci.propertyName);
        connect( obj, ci.editorSignal.toAscii(), ee, SLOT(valueChanged()) );
    }
    
    dialog->exec();
}

#include "metadataeditor.moc"
