/*
 * Copyright (c) 2011 Cyrille Berger <cberger@cberger.net>
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

#include "kis_color_source_option_widget.h"

#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>

#include <KoID.h>

#include "kis_color_source_option.h"
#include "kis_color_source.h"
#include <kis_paint_device.h>


struct KisColorSourceOptionWidget::Private {
    KisColorSourceOption option;
    QMap<QString, QRadioButton*> id2radio;
};

KisColorSourceOptionWidget::KisColorSourceOptionWidget() : KisPaintOpOption(i18n("Source"), KisPaintOpOption::colorCategory(), true ), d(new Private)
{
    m_checkable = false;
    QWidget* configurationWidget = new QWidget;
  
    QGroupBox* groupBox = new QGroupBox(configurationWidget);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    QVBoxLayout* verticalLayout = new QVBoxLayout(groupBox);
    
    foreach(const KoID& id, KisColorSourceOption::sourceIds())
    {
        QRadioButton* radioButton = new QRadioButton(groupBox);
        radioButton->setText(id.name());
        d->id2radio[id.id()] = radioButton;
        connect(radioButton, SIGNAL(toggled(bool)), SLOT(sourceChanged()));
        verticalLayout->addWidget(radioButton);
    }
    QVBoxLayout* verticalLayout_2 = new QVBoxLayout(configurationWidget);
    verticalLayout_2->setMargin(0);
    verticalLayout_2->addWidget(groupBox);
    verticalLayout_2->addStretch();

    setConfigurationPage(configurationWidget);
  
}

KisColorSourceOptionWidget::~KisColorSourceOptionWidget()
{
    delete d;
}

void KisColorSourceOptionWidget::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    d->option.writeOptionSetting(setting);
}

void KisColorSourceOptionWidget::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    d->option.readOptionSetting(setting);
    QRadioButton* rb = d->id2radio.value(d->option.colorSourceTypeId());
    if(rb)
    {
        rb->setChecked(true);
    }
}

void KisColorSourceOptionWidget::sourceChanged()
{
    for(QMap<QString, QRadioButton*>::iterator it = d->id2radio.begin(); it != d->id2radio.end(); ++it)
    {
        if(it.value()->isChecked())
        {
            d->option.setColorSourceType(it.key());
            break;
        }
    }
    emit sigSettingChanged();
}
