/*
 *  SPDX-FileCopyrightText: 2023 Srirupa Datta <srirupa.sps@gmail.com>
 */
#ifndef KOBUNDLECREATIONWIDGET_H
#define KOBUNDLECREATIONWIDGET_H

#include "page_bundle_saver.h"
#include "page_metadata_info.h"
#include "page_resource_chooser.h"
#include "page_tag_chooser.h"
#include "wdg_side.h"

#include <QWidget>
#include <QWizard>

#include <KoDialog.h>
#include <KoResourceBundle.h>
#include "KisBundleStorage.h"
#include <KisResourceStorage.h>


namespace Ui
{
class WdgDlgCreateBundle;
}

class DlgCreateBundle : public QWizard
{
    Q_OBJECT

public:
    explicit DlgCreateBundle(KoResourceBundleSP bundle = nullptr, QWidget *parent = 0);
    ~DlgCreateBundle() override;

    QMap<QString, int> m_count;
    QSet<QString> m_tags;

private Q_SLOTS:

    void accept() override;
    void reject() override;

    void saveToConfiguration(bool full);
    QVector<KisTagSP> getTagsForEmbeddingInResource(QVector<KisTagSP> resourceTags, QString resourceType) const;

public Q_SLOTS:

    void updateTitle(int id);

private:

    Ui::WdgDlgCreateBundle *m_ui;

    bool putResourcesInTheBundle(KoResourceBundleSP bundle);
    void putMetaDataInTheBundle(KoResourceBundleSP bundle) const;
    QString createPrettyFilenameFromName(KoResourceSP resource) const;

    QList<int> m_selectedResourcesIds;
    QList<int> m_selectedTagIds;

    QString m_previewImage;
    KoResourceBundleSP m_bundle;

    PageResourceChooser *m_pageResourceChooser;
    PageTagChooser *m_pageTagChooser;
    PageMetadataInfo *m_pageMetadataInfo;
    PageBundleSaver *m_pageBundleSaver;

    KisBundleStorage *m_bundleStorage;
    KisResourceStorageSP m_resourceStorage;
    QString m_storageID;
    bool m_storageAdded;
    QString m_bundleCreaterMode;
};

#endif // KOBUNDLECREATIONWIDGET_H
