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

#include "kis_types.h"
#include <kdialog.h>

#include "ui_wdglayerproperties.h"

class QWidget;
class KoCompositeOp;
class KoColorSpace;
class QBitArray;
class KisChannelFlagsWidget;
class KisView2;
class KisDoc2;

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
class KisDlgLayerProperties : public KDialog
{
    Q_OBJECT

public:
    KisDlgLayerProperties(KisLayerSP layer, KisView2 *view, KisDoc2 *doc, QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0);

    virtual ~KisDlgLayerProperties();

private:

    bool haveChanges() const;
    QString getName() const;
    qint32 getOpacity() const;
    QString getCompositeOp() const;

    /**
     * @return a bit array of channel flags in the order in which the
     * channels appear in the pixel, not in the list of KoChannelInfo
     * objects from the colorspace.
     */
    QBitArray getChannelFlags() const;

public slots:
    void updatePreview();

protected slots:
    void slotNameChanged(const QString &);
    void applyNewProperties();
    void cleanPreviewChanges();
    
private:
    KisLayerSP m_layer;
    KisView2 *m_view;
    KisDoc2 *m_doc;
    WdgLayerProperties * m_page;
    KisChannelFlagsWidget * m_channelFlags;
    
    struct Private;
    Private * const d;
};

#endif // KIS_DLG_LAYER_PROPERTIES_H_

