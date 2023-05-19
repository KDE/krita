
#ifndef KOBUNDLECREATIONWIDGET_H
#define KOBUNDLECREATIONWIDGET_H

#include <KoDialog.h>
#include <QWizard>
#include "wdg_side.h"
#include <QWidget>


#include "page_resource_chooser.h"
#include "page_tag_chooser.h"
#include "page_metadata_info.h"
#include "page_bundle_saver.h"

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

    // old
    QString bundleName() const;
    QString authorName() const;
    QString email() const;
    QString website() const;
    QString license() const;
    QString description() const;
    QString saveLocation() const;
    QString previewImage() const;

private Q_SLOTS:

    // void next() override;
    /* void reject() override;*/

    // old
    void selectSaveLocation();
    void addSelected();
    void removeSelected();
    void resourceTypeSelected(int idx);
    void getPreviewImage();
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
    PageResourceChooser *pageResourceChooser;
    PageTagChooser *pageTagChooser;
    PageMetadataInfo *pageMetadataInfo;
    PageBundleSaver *pageBundleSaver;

};

#endif // KOBUNDLECREATIONWIDGET_H
