/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
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
#ifndef KIS_DLG_LAYER_PROPERTIES_H_
#define KIS_DLG_LAYER_PROPERTIES_H_

#include <QList>
#include <QCheckBox>
#include <QScopedPointer>


#include "kis_types.h"
#include <KoDialog.h>

#include "ui_wdglayerproperties.h"


class QWidget;
class QBitArray;
class KisViewManager;
class KisDocument;

class WdgLayerProperties : public QWidget, public Ui::WdgLayerProperties
{
    Q_OBJECT

public:
    WdgLayerProperties(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

/**
 * KisDlgLayerProperties is a dialogue for displaying and modifying information on a KisLayer.
 * The dialog is non modal by default and uses a timer to check for user changes to the
 * configuration, showing a preview of them.
 */
class KisDlgLayerProperties : public KoDialog
{
    Q_OBJECT

public:
    KisDlgLayerProperties(KisNodeList nodes, KisViewManager *view, QWidget *parent = 0, const char *name = 0, Qt::WindowFlags f = 0);

    ~KisDlgLayerProperties() override;

protected Q_SLOTS:
    void updatePreview();

    void slotCompositeOpValueChangedInternally();
    void slotCompositeOpValueChangedExternally();

    void slotColorLabelValueChangedInternally();
    void slotColorLabelValueChangedExternally();

    void slotOpacityValueChangedInternally();
    void slotOpacityValueChangedExternally();

    void slotNameValueChangedInternally();
    void slotNameValueChangedExternally();

    void slotPropertyValueChangedInternally();
    void slotFlagsValueChangedInternally();

private:
    struct Private;
    const QScopedPointer<Private> d;
};

#endif // KIS_DLG_LAYER_PROPERTIES_H_

