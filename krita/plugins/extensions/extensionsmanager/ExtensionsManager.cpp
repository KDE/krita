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

#include "ExtensionsManager.h"

#include <QApplication>
#include <QFile>
#include <QDomDocument>

#include <kglobal.h>
#include <kurl.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <klocalizedstring.h>

#include <KoStore.h>

#include <kis_debug.h>

#include "Extension.h"
#include <ui_ExtensionInformationWidget.h>

ExtensionsManager::ExtensionsManager()
{
}

ExtensionsManager::~ExtensionsManager()
{
    dbgRegistry << "deleting ExtensionsManager";
}

ExtensionsManager* ExtensionsManager::instance()
{

    K_GLOBAL_STATIC(ExtensionsManager, s_instance);
    return s_instance;
}

QList<Extension*> ExtensionsManager::installedExtension()
{
    return m_installedExtension;
}

bool ExtensionsManager::installExtension(const KUrl& uri)
{
    if (!KIO::NetAccess::exists(uri, KIO::NetAccess::SourceSide, qApp->activeWindow())) {
        return false;
    }

    // We're not set up to handle asynchronous loading at the moment.
    QString tmpFile;

    if (KIO::NetAccess::download(uri, tmpFile, qApp->activeWindow())) {
        KUrl uriTF;
        uriTF.setPath(tmpFile);

        // open the file
        bool result = false;
        QFile *fp = new QFile(uriTF.toLocalFile());
        if (fp->exists()) {
            result =  installExtension(fp);
        }
        delete fp;
        KIO::NetAccess::removeTempFile(tmpFile);
        return result;
    }
    return false;
}

template<class _T_>
class Keeper
{
public:
    Keeper(_T_* t) : m_t(t) {}
    ~Keeper() {
        delete m_t;
    }
    void release() {
        m_t = 0;
    }
private:
    _T_* m_t;
};

bool ExtensionsManager::installExtension(QIODevice* _device)
{
    KoStore* store = KoStore::createStore(_device, KoStore::Read, "", KoStore::Zip);
    Keeper<KoStore> storek(store);
    // Check that 'manifest.xml' and 'source.tar.bz2' are present
    bool hasManifestXML = store->hasFile("manifest.xml");
    bool hasSource = store->hasFile("source.tar.bz2");
    if (!hasManifestXML || !hasSource) {
        KMessageBox::error(0, i18n("Invalid extension, missing 'manifest.xml' or 'source.tar.bz2'"));
        return false;
    }
    // Attempt to open the 'manifest.xml' file
    if (!store->open("manifest.xml")) {
        KMessageBox::error(0, i18n("Failed to open 'manifest.xml'."));
        return false;
    }
    // Parse the 'manifest.xml'
    QDomDocument doc;
    int line;
    QString errMsg;
    if (!doc.setContent(store->device(), &errMsg, &line)) {
        KMessageBox::error(0, i18n("Failed to parse 'manifest.xml' : %1 at line: %2", errMsg, line));
        return false;
    }
    // Create the extension
    Extension* extension = new Extension;
    Keeper<Extension> extensionk(extension);
    QDomNode n = doc.firstChild();
    QDomElement e = n.toElement();
    if (e.isNull() || e.tagName() != "manifest") {
        KMessageBox::error(0, i18n("Invalid 'manifest.xml' : should contain a <manifest> tag."));
        return false;
    }
    extension->parse(e);
    // Test if extension contains valid information
    if (extension->name().isEmpty() || extension->description().isEmpty() || extension->version().isEmpty()) {
        KMessageBox::error(0, i18n("Missing information in 'manifest.xml'."));
    }
    // Close the 'manifest.xml'
    store->close();
    // Show a dialog with package information
    Ui::ExtensionInformationWidget eiw;
    QWidget* widget = new QWidget;
    eiw.setupUi(widget);
    eiw.labelName->setText(extension->name());
    eiw.labelDescription->setText(extension->description());
    eiw.labelVersion->setText(extension->version());
    KDialog dialog;
    dialog.setMainWidget(widget);
    dialog.setButtonText(KDialog::Ok, i18n("Continue"));
    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }
    // Start the installation
    // Cleanup
    return false;
}
