
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

    // old
    void saveToConfiguration(bool full);
    void slotEmbedTags();
    QVector<KisTagSP> getTagsForEmbeddingInResource(QVector<KisTagSP> resourceTags) const;

public Q_SLOTS:

    // new
    void updateTitle(int id);

private:

    QWidget *m_page;
    Ui::WdgDlgCreateBundle *m_ui;

    // old
    bool putResourcesInTheBundle(KoResourceBundleSP bundle);
    void putMetaDataInTheBundle(KoResourceBundleSP bundle) const;
    QString createPrettyFilenameFromName(KoResourceSP resource) const;

    QList<int> m_selectedResourcesIds;
    QList<int> m_selectedTagIds;

    QString m_previewImage;
    KoResourceBundleSP m_bundle;

    // new
    PageResourceChooser *m_pageResourceChooser;
    PageTagChooser *m_pageTagChooser;
    PageMetadataInfo *m_pageMetadataInfo;
    PageBundleSaver *m_pageBundleSaver;

};

#endif // KOBUNDLECREATIONWIDGET_H
