/*
 *  SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2011 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    KisDlgLayerProperties(KisNodeList nodes, KisViewManager *view, QWidget *parent = 0, const char *name = 0, Qt::WindowFlags f = Qt::WindowFlags());

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

