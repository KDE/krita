/*
 *  SPDX-FileCopyrightText: 2023 Srirupa Datta <srirupa.sps@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef PAGE_BUNDLE_SAVER_H
#define PAGE_BUNDLE_SAVER_H

#include <QWizardPage>
#include <KoResourceBundle.h>
#include "KisBundleStorage.h"


namespace Ui {
class PageBundleSaver;
}

class PageBundleSaver : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageBundleSaver(KoResourceBundleSP bundle = nullptr, QWidget *parent = nullptr);
    ~PageBundleSaver();

    QString saveLocation() const;
    void showWarning();
    void removeWarning();

private Q_SLOTS:
    void selectSaveLocation();

public Q_SLOTS:
    void onCountUpdated();
    void onTagsUpdated();

private:
    Ui::PageBundleSaver *m_ui;
    KoResourceBundleSP m_bundle;
    QMap<QString, int> m_count;
    QSet<QString> m_loaded_tags;
    QMap<QString, int> m_loaded_count;
    QString m_resourceCount;
    QString m_tags;
    KisBundleStorage *m_bundleStorage;
};

#endif // PAGE_BUNDLE_SAVER_H
