/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISDLGAdjustMENTLAYER_H
#define KISDLGAdjustMENTLAYER_H

#include <KoDialog.h>

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
     * Create a new adjustment layer dialog
     *
     * @param node the name of the adjustment node
     * @param nfi filter interface
     * @param paintDevice the paint device that is used as source for the preview
     * @param layerName the name of the layer
     * @param caption the caption for the dialog -- create or properties
     * @param view the view manager
     * @param parent the widget parent of this dialog
     * @param initialFilterConfig the configuration to preload when editing an
     *        existing filter node; leave null for new nodes.
     */
    KisDlgAdjustmentLayer(KisNodeSP node,
                          KisNodeFilterInterface* nfi,
                          KisPaintDeviceSP paintDevice,
                          const QString & layerName,
                          const QString & caption,
                          KisViewManager *view,
                          QWidget *parent = 0,
                          KisFilterConfigurationSP initialFilterConfig = KisFilterConfigurationSP());
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
    QString m_previousFilterName;

    void updateLayerNameForNewFilter(const QString &newFilterName);

};

#endif
