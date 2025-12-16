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

    int w = cfg.defImageWidth();
    int h = cfg.defImageHeight();
    const double resolution = cfg.defImageResolution();
    const QString colorModel = cfg.defColorModel();
    const QString colorDepth = cfg.defaultColorDepth();
    const QString colorProfile = cfg.defColorProfile();

    addCustomDocumentWidget(
        new KisCustomImageWidget(this, w, h, resolution, colorModel, colorDepth, colorProfile, i18n("Unnamed")),
        i18n("Custom Document"),
        "Custom Document",
        "document-new");

   addCustomDocumentWidget(
        new KisImageFromClipboardWidget(this, 0, 0, resolution, colorModel, colorDepth, colorProfile, i18n("Unnamed")),
        i18n("Create from Clipboard"),
        "Create from ClipBoard",
        "tab-new");

    connect(this, SIGNAL(documentSelected(KisDocument*)), KisPart::instance(), SLOT(startCustomDocument(KisDocument*)));
    connect(this, SIGNAL(openTemplate(QUrl)), KisPart::instance(), SLOT(openTemplate(QUrl)));
}
