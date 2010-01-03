/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef BUMPMAP_H
#define BUMPMAP_H

#include <QWidget>

#include <kparts/plugin.h>

#include <kis_types.h>
#include <filter/kis_filter.h>
#include "kis_config_widget.h"

#include "ui_wdgbumpmap.h"

class KisNodeModel;

class BumpmapWidget : public QWidget, public Ui::WdgBumpmap
{
    Q_OBJECT

public:
    BumpmapWidget(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

namespace krita
{

enum enumBumpmapType {
    LINEAR = 0,
    SPHERICAL = 1,
    SINUSOIDAL = 2
};

}

using namespace krita;


class KritaBumpmap : public QObject
{
public:
    KritaBumpmap(QObject *parent, const QStringList &);
    virtual ~KritaBumpmap();
};


/**
 * First stab at a bumpmapping filter. For now, this is taken both
 * from the Gimp source and the code from emboss.c:
 *             ANSI C code from the article
 *             "Fast Embossing Effects on Raster Image Data"
 *             by John Schlag, jfs@kerner.com
 *             in "Graphics Gems IV", Academic Press, 1994
 *
 * XXX: make sure we save the layer by name and restore it on loading
 *      adj. layers and filter masks from the layer stack. Maybe do that
 *      afterwards?
 */
class KisFilterBumpmap : public KisFilter
{
public:
    KisFilterBumpmap();
public:

    using KisFilter::process;

    void process(KisConstProcessingInformation src,
                 KisProcessingInformation dst,
                 const QSize& size,
                 const KisFilterConfiguration* config,
                 KoUpdater* progressUpdater
                ) const;

    bool supportsAdjustmentLayers() const {
        return false;
    }

    virtual KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, const KisImageWSP image = 0) const;
    virtual KisFilterConfiguration* factoryConfiguration(const KisPaintDeviceSP) const;
};

class KisBumpmapConfigWidget : public KisConfigWidget
{

    Q_OBJECT

public:
    KisBumpmapConfigWidget(const KisPaintDeviceSP dev, const KisImageWSP image, QWidget * parent, Qt::WFlags f = 0);
    virtual ~KisBumpmapConfigWidget() {}

    void setConfiguration(const KisPropertiesConfiguration* config);
    KisPropertiesConfiguration* configuration() const;

    BumpmapWidget * m_page;

private:

    KisPaintDeviceSP m_device;
    KisImageWSP m_image;
    KisNodeModel * m_model;
};

#endif
