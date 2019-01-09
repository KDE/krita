/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#include <KoDialog.h>
#include <QLabel>

class KisFilterConfiguration;
class KisNodeFilterInterface;
class KisViewManager;
#include "kis_types.h"
#include "ui_wdgfilternodecreation.h"

/**
 * Create a new adjustment layer.
 */
class KisDlgAdjustmentLayer : public KoDialog
{

    Q_OBJECT

public:

    /**
     * Create a new adjustmentlayer dialog
     *
     * @param node the name of the adjustment node
     * @param nfi filter interface
     * @param paintDevice the paint device that is used as source for the preview
     * @param layerName the name of the layer
     * @param caption the caption for the dialog -- create or properties
     * @param view the view manager
     * @param parent the widget parent of this dialog
     */
    KisDlgAdjustmentLayer(KisNodeSP node,
                          KisNodeFilterInterface* nfi,
                          KisPaintDeviceSP paintDevice,
                          const QString & layerName,
                          const QString & caption,
                          KisViewManager *view,
                          QWidget *parent = 0);
    ~KisDlgAdjustmentLayer() override;
    KisFilterConfigurationSP  filterConfiguration() const;
    QString layerName() const;

public Q_SLOTS:
    void adjustSize();

protected Q_SLOTS:
    void slotNameChanged(const QString &);
    void slotConfigChanged();
    void slotFilterWidgetSizeChanged();

private:
    KisNodeSP m_node;
    KisNodeFilterInterface *m_nodeFilterInterface;
    Ui::WdgFilterNodeCreation wdgFilterNodeCreation;
    KisFilterConfigurationSP m_currentFilter;
    bool m_customName;
    QString m_layerName;

};

#endif
