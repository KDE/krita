/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_dlg_file_layer.h"

#include <QLineEdit>
#include <QCheckBox>
#include <QStandardPaths>

#include <klocalizedstring.h>

#include <KoFileDialog.h>
#include <KisApplication.h>
#include <KisImportExportManager.h>
#include <kis_file_name_requester.h>
#include <kis_config_widget.h>
#include <kis_paint_device.h>
#include <kis_transaction.h>
#include <kis_node.h>
#include <kis_file_layer.h>
#include <kis_filter_strategy.h>

KisDlgFileLayer::KisDlgFileLayer(const QString &basePath, const QString & name, QWidget * parent)
    : KoDialog(parent)
    , m_basePath(basePath)
{
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    QWidget * page = new QWidget(this);
    dlgWidget.setupUi(page);
    QStringList mimes = KisImportExportManager::supportedMimeTypes(KisImportExportManager::Import);
    dlgWidget.wdgUrlRequester->setMimeTypeFilters(mimes);
    setMainWidget(page);

    //dlgWidget.wdgUrlRequester->setBasePath(m_basePath);
    dlgWidget.wdgUrlRequester->setStartDir(m_basePath);

    dlgWidget.txtLayerName->setText(name);

    connect(dlgWidget.wdgUrlRequester, SIGNAL(textChanged(QString)),
            SLOT(slotNameChanged(QString)));

    dlgWidget.cmbFilter->setIDList(KisFilterStrategyRegistry::instance()->listKeys());
    dlgWidget.cmbFilter->setCurrent("Bicubic");
    dlgWidget.cmbFilter->setToolTip(i18nc("@info:tooltip",
                                "<p>Select filtering mode:\n"
                                "<ul>"
                                "<li><b>Nearest neighbor</b> for pixel art. Does not produce new color.</li>"
                                "<li><b>Bilinear</b> for areas with uniform color to avoid artifacts.</li>"
                                "<li><b>Bicubic</b> for smoother results.</li>"
                                "<li><b>Lanczos3</b> for sharp results. May produce aerials.</li>"
                                "</ul></p>"));
    connect(dlgWidget.radioDontScale, SIGNAL(toggled(bool)),
            this, SLOT(slotMethodChanged(bool)));

    enableButtonOk(false);
}

void KisDlgFileLayer::slotNameChanged(const QString & text)
{
    enableButtonOk(!text.isEmpty());
}

void KisDlgFileLayer::slotMethodChanged(const bool & value)
{
    dlgWidget.cmbFilter->setDisabled(value);
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

QString KisDlgFileLayer::scalingFilter() const
{
    return dlgWidget.cmbFilter->currentItem().id();
}

void KisDlgFileLayer::setFileName(QString fileName)
{
    dlgWidget.wdgUrlRequester->setFileName(fileName);
}

void KisDlgFileLayer::setScalingMethod(KisFileLayer::ScalingMethod method)
{
    dlgWidget.radioDontScale->setChecked(false);
    dlgWidget.radioScaleToImageSize->setChecked(false);
    dlgWidget.radioScalePPI->setChecked(false);
    if (method == KisFileLayer::None) {
        dlgWidget.radioDontScale->setChecked(true);
    } else if (method == KisFileLayer::ToImageSize) {
        dlgWidget.radioScaleToImageSize->setChecked(true);
    } else {
        dlgWidget.radioScalePPI->setChecked(true);
    }
}

void KisDlgFileLayer::setScalingFilter(QString filter)
{
    dlgWidget.cmbFilter->setCurrent(filter);
}

QString KisDlgFileLayer::fileName() const
{
    QString path = dlgWidget.wdgUrlRequester->fileName();
    QFileInfo fi(path);
    if (fi.isSymLink()) {
        path = fi.symLinkTarget();
        fi = QFileInfo(path);
    }
    if (!m_basePath.isEmpty() && fi.isAbsolute()) {
        QDir directory(m_basePath);
        path = directory.relativeFilePath(path);
    }

    return path;
}

