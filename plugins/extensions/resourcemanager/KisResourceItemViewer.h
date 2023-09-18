/*
 *  SPDX-FileCopyrightText: 2023 Srirupa Datta <srirupa.sps@gmail.com>
 */
#ifndef KISRESOURCEITEMVIEWER_H
#define KISRESOURCEITEMVIEWER_H

#include <QWidget>

#include "KisPopupButton.h"
#include "ResourceListViewModes.h"


enum class Viewer {TableAvailable, ResourceManager, TableSelected};

class KisResourceItemViewer : public KisPopupButton
{
    Q_OBJECT

public:
    explicit KisResourceItemViewer(Viewer type, QWidget *parent = nullptr);
    ~KisResourceItemViewer();

    void updateViewSettings();

private Q_SLOTS:
    void slotViewThumbnail();
    void slotViewDetails();

Q_SIGNALS:
    void onViewThumbnail();
    void onViewDetails();

private:

    ListViewMode m_mode;
    Viewer m_type;

};

#endif // KISRESOURCEITEMVIEWER_H
