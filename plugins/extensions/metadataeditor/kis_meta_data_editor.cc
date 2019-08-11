/*
 *  Copyright (c) 2007,2010 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
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

#include "kis_meta_data_editor.h"

#include <QDomDocument>
#include <QFile>

#include <klocalizedstring.h>
#include <KoResourcePaths.h>


#include <kis_debug.h>

#include <kis_meta_data_store.h>
#include <kis_meta_data_entry.h>
#include <kis_meta_data_value.h>
#include <kis_meta_data_schema.h>
#include <kis_meta_data_schema_registry.h>
#include <kis_icon.h>
#include "kis_entry_editor.h"
#include <QTableView>
#include "kis_meta_data_model.h"
#include <QHeaderView>

struct KisMetaDataEditor::Private {
    KisMetaData::Store* originalStore;
    KisMetaData::Store* store;
    QMultiHash<QString, KisEntryEditor*> entryEditors;
};


KisMetaDataEditor::KisMetaDataEditor(QWidget* parent, KisMetaData::Store* originalStore) :
        KPageDialog(parent), d(new Private)
{
    d->originalStore = originalStore;
    d->store = new KisMetaData::Store(*originalStore);

    QStringList files = KoResourcePaths::findAllResources("data", "kritaplugins/metadataeditor/*.xmlgui");

    QMap<QString, QWidget*> widgets;
    widgets["dublincore.ui"] = new WdgDublinCore(this);
    widgets["exif.ui"] = new WdgExif(this);

    Q_FOREACH (const QString & file, files) {

        QFile xmlFile(file);
        xmlFile.open(QFile::ReadOnly);
        QString errMsg;
        int errLine, errCol;

        QDomDocument document;
        if (!document.setContent(&xmlFile, false, &errMsg, &errLine, &errCol)) {
            dbgMetaData << "Error reading XML at line" << errLine << " column" << errCol << " :" << errMsg;
        }
        QDomElement rootElement = document.documentElement();
        if (rootElement.tagName() != "MetaDataEditor") {
            dbgMetaData << "Invalid XML file";
        }

        const QString uiFileName = rootElement.attribute("uiFile");
        const QString pageName = i18nc("metadata editor page", rootElement.attribute("name").toUtf8());
        const QString iconName = rootElement.attribute("icon");
        if (uiFileName == "") continue;

        QWidget *widget = widgets[uiFileName];

        QDomNodeList list = rootElement.childNodes();
        const int size = list.size();
        for (int i = 0; i < size; ++i) {
            QDomElement elem = list.item(i).toElement();
            if (elem.isNull() || elem.tagName() != "EntryEditor") continue;
            const QString editorName = elem.attribute("editorName");
            const QString schemaUri = elem.attribute("schemaUri");
            const QString entryName = elem.attribute("entryName");
            const QString editorSignal = '2' + elem.attribute("editorSignal");
            const QString propertyName = elem.attribute("propertyName");
            const QString structureField = elem.attribute("structureField");
            int arrayIndex = 0;
            if (elem.hasAttribute("arrayIndex")) {
                bool ok;
                arrayIndex = elem.attribute("arrayIndex", "0").toInt(&ok);
                if (!ok) arrayIndex = 0;
            }
            dbgMetaData << ppVar(editorName) << ppVar(arrayIndex);

            QWidget* metaDataEditorPage = widget->findChild<QWidget*>(editorName);
            if (metaDataEditorPage) {
                const KisMetaData::Schema* schema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(schemaUri);
                if (schema) {
                    if (!d->store->containsEntry(schema, entryName)) {
                        dbgMetaData << " Store does not have yet entry :" << entryName << " in" << schemaUri  << " ==" << schema->generateQualifiedName(entryName);
                    }
                    QString key = schema->generateQualifiedName(entryName);
                    KisEntryEditor* ee = new KisEntryEditor(metaDataEditorPage, d->store, key, propertyName, structureField, arrayIndex);
                    connect(metaDataEditorPage, editorSignal.toLatin1(), ee, SLOT(valueEdited()));
                    QList<KisEntryEditor*> otherEditors = d->entryEditors.values(key);
                    Q_FOREACH (KisEntryEditor* oe, otherEditors) {
                        connect(ee, SIGNAL(valueHasBeenEdited()), oe, SLOT(valueChanged()));
                        connect(oe, SIGNAL(valueHasBeenEdited()), ee, SLOT(valueChanged()));
                    }
                    d->entryEditors.insert(key, ee);
                } else {
                    dbgMetaData << "Unknown schema :" << schemaUri;
                }
            } else {
                dbgMetaData << "Unknown object :" << editorName;
            }
        }
        xmlFile.close();

        KPageWidgetItem *page = new KPageWidgetItem(widget, pageName);
        if (!iconName.isEmpty()) {
            page->setIcon(KisIconUtils::loadIcon(iconName));
        }
        addPage(page);
    }

    // Add the list page
    QTableView* tableView = new QTableView;
    KisMetaDataModel* model = new KisMetaDataModel(d->store);
    tableView->setModel(model);
    tableView->verticalHeader()->setVisible(false);
    tableView->resizeColumnsToContents();
    KPageWidgetItem *page = new KPageWidgetItem(tableView, i18n("List"));
    page->setIcon(KisIconUtils::loadIcon("format-list-unordered"));
    addPage(page);
}

KisMetaDataEditor::~KisMetaDataEditor()
{
    Q_FOREACH (KisEntryEditor* e, d->entryEditors) {
        delete e;
    }
    delete d->store;
    delete d;
}

void KisMetaDataEditor::accept()
{
    KPageDialog::accept();
    d->originalStore->copyFrom(d->store);
}

