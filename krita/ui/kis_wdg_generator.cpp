/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_wdg_generator.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>
#include <QString>

#include <kis_paint_device.h>
#include <generator/kis_generator.h>
#include <generator/kis_generator_registry.h>
#include <filter/kis_filter_config_widget.h>
#include <filter/kis_filter_configuration.h>
#include <KoColorSpaceRegistry.h>

#include "ui_wdggenerators.h"

class KisGeneratorItem : public QListWidgetItem
{
public:

    KisGeneratorItem(const QString & text, QListWidget * parent = 0, int type = Type )
        : QListWidgetItem(text, parent, type)
        {
        }
    
    KisGeneratorSP generator;

};

class KisWdgGenerator::Private
{

public:
    Private()
        : centralWidget(0)
        {
        }

    QWidget * centralWidget; // Active generator settings widget
    KisGeneratorSP currentGenerator;
    Ui_WdgGenerators uiWdgGenerators;
    KisPaintDeviceSP dev;
};

KisWdgGenerator::KisWdgGenerator(QWidget * parent)
    : QWidget(parent)
    , d(new Private())
{
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8(0), "tmp");
    init(dev);
}

KisWdgGenerator::KisWdgGenerator( QWidget * parent, KisPaintDeviceSP dev)
    : QWidget(parent)
    , d(new Private())
{
    init(dev);
}

KisWdgGenerator::~KisWdgGenerator()
{
}

void KisWdgGenerator::setPaintdevice(KisPaintDeviceSP dev)
{
    d->dev = dev;
}

void KisWdgGenerator::init(KisPaintDeviceSP dev)
{
    d->dev = dev;
    d->uiWdgGenerators.setupUi( this );
    KisGeneratorRegistry * registry = KisGeneratorRegistry::instance();
    qDebug() << registry->count();
    
    foreach(QString key, registry->keys())
    {
        KisGeneratorSP generator = registry->get(key);
        Q_ASSERT(generator);
        KisGeneratorItem * item = new KisGeneratorItem(generator->name(),
                                                d->uiWdgGenerators.lstGenerators,
                                                QListWidgetItem::UserType + 1);
        item->generator = generator;
    }
    connect(d->uiWdgGenerators.lstGenerators, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(slotGeneratorActivated(QListWidgetItem*)));

    slotGeneratorActivated(d->uiWdgGenerators.lstGenerators->currentItem());
}



void KisWdgGenerator::setConfiguration(KisFilterConfiguration * config)
{
    for(int i = 0; i < d->uiWdgGenerators.lstGenerators->count(); ++i) {
        KisGeneratorItem * item = static_cast<KisGeneratorItem*>(d->uiWdgGenerators.lstGenerators->item(i));
        if (item->generator->id() == config->name()) {
            // match!
            slotGeneratorActivated(item);
            KisFilterConfigWidget * wdg = dynamic_cast<KisFilterConfigWidget*>(d->centralWidget);
            if (wdg) {
                wdg->setConfiguration(config);
            }
            return;
        }
    }
}

KisFilterConfiguration * KisWdgGenerator::configuration()
{
    KisFilterConfigWidget * wdg = dynamic_cast<KisFilterConfigWidget*>(d->centralWidget);
    if (wdg) {
        return wdg->configuration();
    }
    else {
        return 0;
    }

}

void KisWdgGenerator::slotGeneratorActivated(QListWidgetItem* i)
{
    if (!i) {
        d->centralWidget = new QLabel( i18n("No configuration options."),
                                       d->uiWdgGenerators.centralWidgetHolder );
        return;
    }

    KisGeneratorItem * item = static_cast<KisGeneratorItem*>(i);
    d->currentGenerator = item->generator;
    
    delete d->centralWidget;
    
    KisFilterConfigWidget* widget =
        d->currentGenerator->createConfigurationWidget( d->uiWdgGenerators.centralWidgetHolder,
                                                        d->dev );
        
    if( !widget )
    { // No widget, so display a label instead
        d->centralWidget = new QLabel( i18n("No configuration options."),
                                       d->uiWdgGenerators.centralWidgetHolder );
    }
    else {
        d->centralWidget = widget;
        widget->setConfiguration( d->currentGenerator->defaultConfiguration( d->dev ) );
        //connect(d->currentFilterConfigurationWidget, SIGNAL(sigPleaseUpdatePreview()), SLOT(updatePreview()));
    }

}

