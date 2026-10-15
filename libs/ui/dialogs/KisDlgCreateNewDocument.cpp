/*
 *  SPDX-FileCopyrightText: 2026 Luna Lovecraft <ciubix8514@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "KisDlgCreateNewDocument.h"
#include "KisImportExportManager.h"
#include "KisOpenPane.h"
#include "dialogs/kis_dlg_preferences.h"
#include "kis_custom_image_widget.h"
#include "kis_image_from_clipboard_widget.h"
#include "KisPart.h"


KisDlgCreateNewDocument::KisDlgCreateNewDocument(QWidget* parent)
    : KisOpenPane(parent, KisImportExportManager::supportedMimeTypes(KisImportExportManager::Import), QStringLiteral("templates/"))
{
    setWindowModality(Qt::WindowModal);
    setWindowTitle(i18n("Create new document"));

    KisConfig cfg(true);

    const QLatin1String customDocumentConfigSection("");
    const QLatin1String fromClipboardConfigSection("FromClipboard");

    addCustomDocumentWidget(
        new KisCustomImageWidget(this, i18n("Unnamed"), customDocumentConfigSection),
        i18n("Custom Document"),
        "Custom Document",
        "document-new");

   addCustomDocumentWidget(
        new KisImageFromClipboardWidget(this, i18n("Unnamed"), fromClipboardConfigSection),
        i18n("Create from Clipboard"),
        "Create from ClipBoard",
        "tab-new");

    connect(this, SIGNAL(documentSelected(KisDocument*)), KisPart::instance(), SLOT(startCustomDocument(KisDocument*)));
    connect(this, SIGNAL(openTemplate(QUrl)), KisPart::instance(), SLOT(openTemplate(QUrl)));
}

void KisDlgCreateNewDocument::SelectPage(Page page)
{
    switch (page) {
    case CreateNewDocument:
        selectItem("Custom Document");
        break;
    case CreateFromClipboard:
        selectItem("Create from ClipBoard");
        break;
    }
}
