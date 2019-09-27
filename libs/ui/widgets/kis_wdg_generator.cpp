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
#include <QStringList>

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

struct KisWdgGenerator::Private
{

public:
    Private()
        : centralWidget(0), view(0) {
    }

    QWidget * centralWidget; // Active generator settings widget
    KisGeneratorSP currentGenerator;
    Ui_WdgGenerators uiWdgGenerators;
    KisPaintDeviceSP dev;
    QGridLayout *widgetLayout;
    KisViewManager *view;
};

KisWdgGenerator::KisWdgGenerator(QWidget * parent)
        : QWidget(parent)
        , d(new Private())
{
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8(0));
}

KisWdgGenerator::~KisWdgGenerator()
{
  delete d;
}

void KisWdgGenerator::initialize(KisViewManager *view)
{
    d->view = view;
    d->uiWdgGenerators.setupUi(this);
    d->widgetLayout = new QGridLayout(d->uiWdgGenerators.centralWidgetHolder);
    QStringList generatorNames = KisGeneratorRegistry::instance()->keys();
    generatorNames.sort();

    Q_FOREACH (const QString &generatorName, generatorNames) {

        KisGeneratorSP generator = KisGeneratorRegistry::instance()->get(generatorName);

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



void KisWdgGenerator::setConfiguration(const KisFilterConfigurationSP  config)
{
    for (int i = 0; i < d->uiWdgGenerators.lstGenerators->count(); ++i) {
        KisGeneratorItem * item = static_cast<KisGeneratorItem*>(d->uiWdgGenerators.lstGenerators->item(i));
        if (item->generator->id() == config->name()) {
            // match!
            slotGeneratorActivated(i);
            d->uiWdgGenerators.lstGenerators->setCurrentRow(i);
            KisConfigWidget * wdg = dynamic_cast<KisConfigWidget*>(d->centralWidget);
            if (wdg) {
                wdg->setConfiguration(config);
            }

            return;
        }
    }
}

KisFilterConfigurationSP KisWdgGenerator::configuration()
{
    KisConfigWidget * wdg = dynamic_cast<KisConfigWidget*>(d->centralWidget);
    if (wdg) {
        KisFilterConfigurationSP config = dynamic_cast<KisFilterConfiguration*>(wdg->configuration().data());
        if (config) {
            return config;
        }
    } else {
        return d->currentGenerator->defaultConfiguration();
    }
    return 0;
}

void KisWdgGenerator::slotGeneratorActivated(int row)
{
    KisGeneratorItem *item = dynamic_cast<KisGeneratorItem*>(d->uiWdgGenerators.lstGenerators->item(row));

    if (!item) {
        d->centralWidget = new QLabel(i18n("No configuration options."), d->uiWdgGenerators.centralWidgetHolder);
    }
    else {

        d->currentGenerator = item->generator;

        delete d->centralWidget;

        KisConfigWidget* widget =
            d->currentGenerator->createConfigurationWidget(d->uiWdgGenerators.centralWidgetHolder, d->dev, true);

        if (!widget) { // No widget, so display a label instead
            d->centralWidget = new QLabel(i18n("No configuration options."),
                                          d->uiWdgGenerators.centralWidgetHolder);
        } else {
            d->centralWidget = widget;

            connect( widget, SIGNAL(sigConfigurationUpdated()), this, SIGNAL(previewConfiguration()));

            widget->setView(d->view);
            widget->setConfiguration(d->currentGenerator->defaultConfiguration());
        }
    }
    d->widgetLayout->addWidget(d->centralWidget, 0 , 0);
    d->uiWdgGenerators.centralWidgetHolder->setMinimumSize(d->centralWidget->minimumSize());


}


