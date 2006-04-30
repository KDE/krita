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
#ifndef KIS_DLG_ADJ_LAYER_PROPS_H
#define KIS_DLG_ADJ_LAYER_PROPS_H

#include <kdialog.h>

class KLineEdit;

class KisFilter;
class KisPreviewWidget;
class KisFiltersListView;
class KisFilterConfiguration;
class KisImage;

/**
 * Create a new adjustment layer.
 */
class KisDlgAdjLayerProps : public KDialog
{
    Q_OBJECT

public:

    /**
     * Create a new adjustmentlayer dialog
     *
     * @param img the current image
     * @param layername the name of the adjustment layer
     * @param caption the caption for the dialog -- create or properties
     * @param create if true, set the dialog up for creating a new adj. layer, if false, edit the
     *               propeties of the current adj. layer
     * @param parent the widget parent of this dialog
     * @param name the QObject name, if any
     */
    KisDlgAdjLayerProps(KisAdjustmentLayerSP layer,
                          const QString & layerName,
                          const QString & caption,
                          QWidget *parent = 0,
                          const char *name = 0);

    KisFilterConfiguration * filterConfiguration() const;
    QString layerName() const;

protected slots:

    void slotNameChanged( const QString & );
    void slotConfigChanged();
    void refreshPreview();

private:
    KisImage * m_image;
    KisPreviewWidget * m_preview;
    KisFilterConfigWidget * m_currentConfigWidget;
    KisFilter* m_currentFilter;
    KisFilterConfiguration * m_currentConfiguration;
    KisAdjustmentLayer * m_layer;
    KLineEdit * m_layerName;
};

#endif // KIS_DLG_ADJ_LAYER_PROPS_H
