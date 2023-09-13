/*
 *  SPDX-FileCopyrightText: 2023 Srirupa Datta <srirupa.sps@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PAGE_METADATA_INFO_H
#define PAGE_METADATA_INFO_H

#include <QWizardPage>
#include <KoResourceBundle.h>


namespace Ui {
class PageMetadataInfo;
}

class PageMetadataInfo : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageMetadataInfo(KoResourceBundleSP bundle = nullptr, QWidget *parent = nullptr);
    ~PageMetadataInfo();

    QString bundleName() const;
    QString authorName() const;
    QString email() const;
    QString website() const;
    QString license() const;
    QString description() const;
    QString previewImage() const;
    QImage thumbnail() const;
    void showWarning();
    void removeWarning();


private Q_SLOTS:
    void getPreviewImage();

private:
    Ui::PageMetadataInfo *m_ui;
    QString m_previewImage;
    QImage m_thumbnail;
    KoResourceBundleSP m_bundle;
};

#endif // PAGE_METADATA_INFO_H
