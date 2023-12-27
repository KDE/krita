/*
 *  SPDX-FileCopyrightText: 2023 Srirupa Datta <srirupa.sps@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PAGE_TAG_CHOOSER_H
#define PAGE_TAG_CHOOSER_H

#include <QWizardPage>
#include <KoResourceBundle.h>
#include "KisTagSelectionWidget.h"
#include <KisTag.h>
#include "KisBundleStorage.h"


namespace Ui {
class PageTagChooser;
}

class PageTagChooser : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageTagChooser(KoResourceBundleSP bundle = nullptr, QWidget *parent = nullptr);
    ~PageTagChooser();

    QList<int> selectedTagIds();
    void updateTags(bool flag, QString tag);

Q_SIGNALS:
    void tagsUpdated();

private Q_SLOTS:

    void addSelected(KisTagSP tagSP);
    void removeSelected(KisTagSP tagSP);

private:
    Ui::PageTagChooser *m_ui;
    KoResourceBundleSP m_bundle;

    QList<int> m_selectedTagIds;
    KisBundleStorage *m_bundleStorage;
};

#endif // PAGE_TAG_CHOOSER_H
