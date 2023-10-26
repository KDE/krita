/*
 *  SPDX-FileCopyrightText: 2023 Srirupa Datta <srirupa.sps@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PAGE_RESOURCE_CHOOSER_H
#define PAGE_RESOURCE_CHOOSER_H

#include "wdg_resource_preview.h"
#include "ResourceListViewModes.h"

#include <QPainter>
#include <QWizardPage>

#include <KoResourceBundle.h>
#include "KisResourceItemListView.h"
#include <KisResourceModel.h>
#include <KisResourceItemDelegate.h>
#include "KisResourceItemListWidget.h"
#include "KisBundleStorage.h"

namespace Ui {
class PageResourceChooser;
}

class PageResourceChooser : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageResourceChooser(KoResourceBundleSP bundle = nullptr, QWidget *parent = nullptr);
    ~PageResourceChooser();

Q_SIGNALS:
    void countUpdated();

private Q_SLOTS:
    void slotResourcesSelectionChanged(QModelIndex selected);
    void slotresourceTypeSelected(int);
    void slotRemoveSelected(bool);
    void slotViewThumbnail();
    void slotViewDetails();

public:
    QPixmap imageToIcon(const QImage &img, Qt::AspectRatioMode aspectRatioMode);
    QList<int> getSelectedResourcesIds();
    void updateCount(bool);
    void updateResources(QString resourceType, int count);


private:
    Ui::PageResourceChooser *m_ui;
    WdgResourcePreview *m_wdgResourcePreview;
    QList<int> m_selectedResourcesIds;
    QList<QString> m_existingResources;
    QMap<QString, QList<QPair<QString, QImage>>> m_existingResourcesImageMap;

    KoResourceBundleSP m_bundle;
    ListViewMode m_mode;
    KisResourceItemDelegate *m_kisResourceItemDelegate;
    KisResourceItemListWidget *m_resourceItemWidget;

    KisBundleStorage *m_bundleStorage;
};

#endif // PAGE_RESOURCE_CHOOSER_H
