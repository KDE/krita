/*
 *  SPDX-FileCopyrightText: 2014 Victor Lafon metabolic.ewilan @hotmail.fr
 *  SPDX-FileCopyrightText: 2020 Agata Cacko cacko.azh @gmail.com
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KOBUNDLECREATIONWIDGET_H
#define KOBUNDLECREATIONWIDGET_H

#include <KoDialog.h>

#include <KoResourceBundle.h>

namespace Ui
{
class WdgDlgCreateBundle;
}

class DlgCreateBundle : public KoDialog
{
    Q_OBJECT

public:
    explicit DlgCreateBundle(KoResourceBundleSP bundle = nullptr, QWidget *parent = 0);
    ~DlgCreateBundle() override;

    QString bundleName() const;
    QString authorName() const;
    QString email() const;
    QString website() const;
    QString license() const;
    QString description() const;
    QString saveLocation() const;
    QString previewImage() const;

private Q_SLOTS:

    void accept() override;
    void reject() override;

    void selectSaveLocation();
    void addSelected();
    void removeSelected();
    void resourceTypeSelected(int idx);
    void getPreviewImage();
    void saveToConfiguration(bool full);
    void slotEmbedTags();
    QVector<KisTagSP> getTagsForEmbeddingInResource(QVector<KisTagSP> resourceTags) const;


private:

    void putResourcesInTheBundle(KoResourceBundleSP bundle) const;
    void putMetaDataInTheBundle(KoResourceBundleSP bundle) const;
    QWidget *m_page;
    Ui::WdgDlgCreateBundle *m_ui;

    QList<int> m_selectedResourcesIds;
    QList<int> m_selectedTagIds;

    QString m_previewImage;

    KoResourceBundleSP m_bundle;
};

#endif // KOBUNDLECREATIONWIDGET_H
