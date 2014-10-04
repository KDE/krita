/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2013
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_dlg_file_layer.h"

#include <QLineEdit>
#include <QCheckBox>
#include <QDesktopServices>

#include <klocale.h>

#include <KoFileDialog.h>
#include <KoApplication.h>
#include <KoFilterManager.h>

#include <kis_config_widget.h>
#include <kis_paint_device.h>
#include <kis_transaction.h>
#include <kis_node.h>
#include <kis_file_layer.h>

KisDlgFileLayer::KisDlgFileLayer(const QString &basePath, const QString & name, QWidget * parent)
    : KDialog(parent)
    , m_basePath(basePath)
    , m_customName(false)
    , m_freezeName(false)
{
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    QWidget * page = new QWidget(this);
    dlgWidget.setupUi(page);
    setMainWidget(page);

    dlgWidget.txtLayerName->setText(name);
    connect(dlgWidget.txtLayerName, SIGNAL(textChanged(const QString &)),
            this, SLOT(slotNameChanged(const QString &)));
    connect(dlgWidget.bnGetFileName, SIGNAL(clicked()), SLOT(slotSelectFile()));
}

void KisDlgFileLayer::slotNameChanged(const QString & text)
{
    if (m_freezeName)
        return;

    m_customName = !text.isEmpty();
    enableButtonOk(m_customName);
}

QString KisDlgFileLayer::layerName() const
{
    return dlgWidget.txtLayerName->text();
}

KisFileLayer::ScalingMethod KisDlgFileLayer::scaleToImageResolution() const
{
    if (dlgWidget.radioDontScale->isChecked()) {
        return KisFileLayer::None;
    }
    else if (dlgWidget.radioScaleToImageSize->isChecked()) {
        return KisFileLayer::ToImageSize;
    }
    else {
        return KisFileLayer::ToImagePPI;
    }
}

QString KisDlgFileLayer::fileName() const
{
    return dlgWidget.txtFileName->text();
}

void KisDlgFileLayer::slotSelectFile()
{
    KoFileDialog dialog(this, KoFileDialog::OpenFile, "OpenDocument");
    dialog.setCaption(i18n("Select file to use as dynamic file layer."));
    dialog.setDefaultDir(m_basePath.isEmpty() ? QDesktopServices::storageLocation(QDesktopServices::PicturesLocation) : m_basePath);
    dialog.setMimeTypeFilters(KoFilterManager::mimeFilter("application/x-krita", KoFilterManager::Import));
    QString url = dialog.url();
    if (m_basePath.isEmpty()) {
        dlgWidget.txtFileName->setText(url);
    }
    else {
        QDir d(m_basePath);
        dlgWidget.txtFileName->setText(d.relativeFilePath(url));
    }
}

#include "kis_dlg_file_layer.moc"
