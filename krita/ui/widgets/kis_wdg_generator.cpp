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

#include "widgets/kis_wdg_generator.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>
#include <QString>
#include <QGridLayout>

#include <kis_image.h>
#include <kis_paint_device.h>
#include <generator/kis_generator.h>
#include <generator/kis_generator_registry.h>
#include <kis_config_widget.h>
#include <filter/kis_filter_configuration.h>
#include <KoColorSpaceRegistry.h>

#include "ui_wdggenerators.h"

class KisGeneratorItem : public QListWidgetItem
{
public:

    KisGeneratorItem(const QString & text, QListWidget * parent = 0, int type = Type)
            : QListWidgetItem(text, parent, type) {
    }

    KisGeneratorSP generator;

};

class KisWdgGenerator::Private
{

public:
    Private()
            : centralWidget(0) {
    }

    QWidget * centralWidget; // Active generator settings widget
    KisGeneratorSP currentGenerator;
    Ui_WdgGenerators uiWdgGenerators;
    KisPaintDeviceSP dev;
    QGridLayout *widgetLayout;
};

KisWdgGenerator::KisWdgGenerator(QWidget * parent)
        : QWidget(parent)
        , d(new Private())
{
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8(0));
    init(dev);
}

KisWdgGenerator::KisWdgGenerator(QWidget * parent, KisPaintDeviceSP dev)
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
    d->uiWdgGenerators.setupUi(this);
    d->widgetLayout = new QGridLayout(d->uiWdgGenerators.centralWidgetHolder);
    QList<KisGeneratorSP> generators = KisGeneratorRegistry::instance()->values();

    foreach(const KisGeneratorSP generator, generators) {
        Q_ASSERT(generator);
        KisGeneratorItem * item = new KisGeneratorItem(generator->name(),
                d->uiWdgGenerators.lstGenerators,
                QListWidgetItem::UserType + 1);
        item->generator = generator;
    }
    connect(d->uiWdgGenerators.lstGenerators, SIGNAL(currentRowChanged(int)),
            this, SLOT(slotGeneratorActivated(int)));

    if (d->uiWdgGenerators.lstGenerators->count() > 0) {
        d->uiWdgGenerators.lstGenerators->setCurrentRow(0);
    }
}



void KisWdgGenerator::setConfiguration(const KisFilterConfiguration * config)
{
    for (int i = 0; i < d->uiWdgGenerators.lstGenerators->count(); ++i) {
        KisGeneratorItem * item = static_cast<KisGeneratorItem*>(d->uiWdgGenerators.lstGenerators->item(i));
        if (item->generator->id() == config->name()) {
            // match!
            slotGeneratorActivated(i);
            KisConfigWidget * wdg = dynamic_cast<KisConfigWidget*>(d->centralWidget);
            if (wdg) {
                wdg->setConfiguration(config);
            }
            return;
        }
    }
}

KisFilterConfiguration * KisWdgGenerator::configuration()
{
    KisConfigWidget * wdg = dynamic_cast<KisConfigWidget*>(d->centralWidget);
    if (wdg) {
        KisFilterConfiguration * config
        = dynamic_cast<KisFilterConfiguration*>(wdg->configuration());
        if (config) {
            return config;
        }
    } else {
        return d->currentGenerator->defaultConfiguration(0);
    }
    return 0;
}

void KisWdgGenerator::slotGeneratorActivated(int row)
{
    KisGeneratorItem * item = dynamic_cast<KisGeneratorItem*>(d->uiWdgGenerators.lstGenerators->item(row));

    if (!item) {
        d->centralWidget = new QLabel(i18n("No configuration options."),
                                      d->uiWdgGenerators.centralWidgetHolder);
    } else {


        d->currentGenerator = item->generator;

        delete d->centralWidget;

        KisConfigWidget* widget =
            d->currentGenerator->createConfigurationWidget(d->uiWdgGenerators.centralWidgetHolder,
                    d->dev);

        if (!widget) { // No widget, so display a label instead
            d->centralWidget = new QLabel(i18n("No configuration options."),
                                          d->uiWdgGenerators.centralWidgetHolder);
        } else {
            d->centralWidget = widget;
            widget->setConfiguration(d->currentGenerator->defaultConfiguration(d->dev));
        }
    }
    d->widgetLayout->addWidget(d->centralWidget, 0 , 0);
    d->uiWdgGenerators.centralWidgetHolder->setMinimumSize(d->centralWidget->minimumSize());


}

#include "kis_wdg_generator.moc"

