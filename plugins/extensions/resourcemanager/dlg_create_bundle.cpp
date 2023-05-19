/*
 *  SPDX-FileCopyrightText: 2014 Victor Lafon metabolic.ewilan @hotmail.fr
 *  SPDX-FileCopyrightText: 2020 Agata Cacko cacko.azh @gmail.com
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "dlg_create_bundle.h"
#include "wdg_side.h"


#include "ui_wdgdlgcreatebundle.h"
#include "page_resource_chooser.h"
#include "page_tag_chooser.h"
#include "page_metadata_info.h"
#include "page_bundle_saver.h"

#include <QDebug>
#include <QFileInfo>
#include <QGridLayout>
#include <QHash>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QProcessEnvironment>
#include <QStack>
#include <QStandardPaths>
#include <QTableWidget>
#include <QVBoxLayout>

#include <KisImportExportManager.h>
#include <KoDocumentInfo.h>
#include <KoFileDialog.h>
#include <kis_config.h>
#include <kis_icon.h>
#include <KoResource.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <KoResource.h>
#include <kstandardguiitem.h>

#include <kis_workspace_resource.h>
#include <brushengine/kis_paintop_preset.h>
#include <dlg_embed_tags.h>
#include <KisGlobalResourcesInterface.h>
#include <KisResourceTypeModel.h>
#include "KisBundleStorage.h"

#include <wdgtagselection.h>


#define ICON_SIZE 48

DlgCreateBundle::DlgCreateBundle(KoResourceBundleSP bundle, QWidget *parent)
    : QWizard(parent)
    , m_ui(new Ui::WdgDlgCreateBundle)
    , m_bundle(bundle)
{
    m_ui->setupUi(this);

    // new
    WdgSide *wdgSide = new WdgSide();

    pageResourceChooser = new PageResourceChooser();
    pageTagChooser = new PageTagChooser();
    pageMetadataInfo = new PageMetadataInfo();
    pageBundleSaver = new PageBundleSaver();

    setPage(1, pageResourceChooser);
    setPage(2, pageTagChooser);
    setPage(3, pageMetadataInfo);
    setPage(4, pageBundleSaver);

    setSideWidget(wdgSide);
    setButtonText(QWizard::FinishButton, "Save");

    connect(this, SIGNAL(currentIdChanged(int)), wdgSide, SLOT(focusLabel(int)));
    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(updateTitle(int)));

    KisResourceTypeModel resourceTypesModel;
    for (int i = 0; i < resourceTypesModel.rowCount(); i++) {
        QModelIndex idx = resourceTypesModel.index(i, 0);
        QString resourceType = resourceTypesModel.data(idx, Qt::UserRole + KisResourceTypeModel::ResourceType).toString();
        m_count.insert(resourceType, 0);
    }

    connect(m_pageResourceChooser, SIGNAL(countUpdated()), m_pageBundleSaver, SLOT(onCountUpdated()));
    connect(m_pageTagChooser, SIGNAL(tagsUpdated()), m_pageBundleSaver, SLOT(onTagsUpdated()));
}

DlgCreateBundle::~DlgCreateBundle()
{
    delete m_ui;
}

void DlgCreateBundle::updateTitle(int id)
{
    QString title = "Create Resource Bundle";

    switch(currentId()) {
        case 1: title = "Choose Resources"; break;
        case 2: title = "Choose Tags"; break;
        case 3: title = "Enter Bundle Details"; break;
        case 4: title = "Enter Save Location"; break;
    }

    setWindowTitle(title);
}

QString DlgCreateBundle::bundleName() const
{
    QString s;
    return s;
}

QString DlgCreateBundle::authorName() const
{
    QString s;
    return s;
}

QString DlgCreateBundle::email() const
{
    QString s;
    return s;
}

QString DlgCreateBundle::website() const
{
    QString s;
    return s;
}

QString DlgCreateBundle::license() const
{
    QString s;
    return s;
}

QString DlgCreateBundle::description() const
{
    QString s;
    return s;
}

QString DlgCreateBundle::saveLocation() const
{
    QString s;
    return s;
}

QString DlgCreateBundle::previewImage() const
{
    QString s;
    return s;
}

QVector<KisTagSP> DlgCreateBundle::getTagsForEmbeddingInResource(QVector<KisTagSP> resourceTags) const
{
    QVector<KisTagSP> tagsToEmbed;
    return tagsToEmbed;
}

bool DlgCreateBundle::putResourcesInTheBundle(KoResourceBundleSP bundle)
{
    return true;
}

void DlgCreateBundle::putMetaDataInTheBundle(KoResourceBundleSP bundle) const
{
}

QString DlgCreateBundle::createPrettyFilenameFromName(KoResourceSP resource) const
{
    QString s;
    return s;
}

void DlgCreateBundle::saveToConfiguration(bool full)
{

}

void DlgCreateBundle::slotEmbedTags()
{
}

QPixmap imageToIcon(const QImage &img, Qt::AspectRatioMode aspectRatioMode) {
    QPixmap pixmap(ICON_SIZE, ICON_SIZE);
    return pixmap;
}

void DlgCreateBundle::resourceTypeSelected(int idx)
{
}

void DlgCreateBundle::getPreviewImage()
{

}