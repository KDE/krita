/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KISDLGAdjustMENTLAYER_H
#define KISDLGAdjustMENTLAYER_H

#include <kdialogbase.h>

class KisFilter;
class QIconViewItem;
class QLabel;
class QHBoxLayout;
class KisPreviewWidget;
class KisFiltersListView;
class KisFilterConfiguration;
class KisImage;
class QGroupBox;

/**
 * Create a new adjustment layer.
 */
class KisDlgAdjustmentLayer : public KDialogBase
{

    Q_OBJECT

public:
    
    KisDlgAdjustmentLayer(KisImage * img,
                          const QString & layerName,
                          const QString & caption,
                          QWidget *parent = 0,
                          const char *name = 0);

    virtual ~KisDlgAdjustmentLayer() {};

    KisFilterConfiguration * filterConfiguration() const;
    QString layerName() const;

protected slots:
    
    void slotNameChanged( const QString & );
    void slotConfigChanged();
    void refreshPreview();
    void selectionHasChanged ( QIconViewItem * item );

private:
    KisImage * m_image;
    KisFiltersListView * m_filtersList;
    KisPreviewWidget * m_preview;
    QGroupBox * m_configWidgetHolder;
    QWidget * m_currentConfigWidget;
    KisFilter* m_currentFilter;
    KLineEdit * m_layerName;
    QLabel* m_labelNoConfigWidget;
};

#endif
