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

#include <kdialog.h>
#include <QLabel>

class QTimer;
class KisFilter;
class QListWidgetItem;
class QLabel;
class KisFilterConfiguration;
class QGroupBox;
class KisConfigWidget;
class KLineEdit;
class KisNodeFilterInterface;
#include "kis_types.h"
#include "ui_wdgfilternodecreation.h"

/**
 * Create a new adjustment layer.
 */
class KisDlgAdjustmentLayer : public KDialog
{

    Q_OBJECT

public:

    /**
     * Create a new adjustmentlayer dialog
     *
     * @param image the current image
     * @param layerName the name of the adjustment layer
     * @param paintDevice the paint device that is used as source for the preview
     * @param caption the caption for the dialog -- create or properties
     * @param parent the widget parent of this dialog
     * @param name the QObject name, if any
     */
    KisDlgAdjustmentLayer(KisNodeSP node,
                          KisNodeFilterInterface* nfi,
                          KisPaintDeviceSP paintDevice,
                          KisImageWSP image,
                          const QString & layerName,
                          const QString & caption,
                          QWidget *parent = 0,
                          const char *name = 0);

    KisFilterConfiguration * filterConfiguration() const;
    QString layerName() const;

protected slots:

    void slotNameChanged(const QString &);
    void slotConfigChanged();
private slots:
    void kickTimer();

private:

    KisNodeSP m_node;
    KisNodeFilterInterface* m_nodeFilterInterface;
    Ui::WdgFilterNodeCreation wdgFilterNodeCreation;
    KisFilterConfiguration* m_currentFilter;
    bool m_freezeName;
    QTimer * m_timer;
};

#endif
