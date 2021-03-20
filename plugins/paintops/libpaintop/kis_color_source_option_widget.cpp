/*
 * SPDX-FileCopyrightText: 2011 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_color_source_option_widget.h"

#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>

#include <KoID.h>

#include "kis_color_source_option.h"
#include "kis_color_source.h"
#include <kis_paint_device.h>
#include <brushengine/kis_paintop_lod_limitations.h>


struct KisColorSourceOptionWidget::Private {
    KisColorSourceOption option;
    QMap<QString, QRadioButton*> id2radio;
};

KisColorSourceOptionWidget::KisColorSourceOptionWidget()
    : KisPaintOpOption(KisPaintOpOption::COLOR, true)
    , d(new Private)
{
    m_checkable = false;
    QWidget* configurationWidget = new QWidget;

    QGroupBox* groupBox = new QGroupBox(configurationWidget);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    QVBoxLayout* verticalLayout = new QVBoxLayout(groupBox);

    Q_FOREACH (const KoID & id, KisColorSourceOption::sourceIds()) {
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

    setObjectName("KisColorSourceOptionWidget");

}

KisColorSourceOptionWidget::~KisColorSourceOptionWidget()
{
    delete d;
}

void KisColorSourceOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    d->option.writeOptionSetting(setting);
}

void KisColorSourceOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    d->option.readOptionSetting(setting);
    QRadioButton* rb = d->id2radio.value(d->option.colorSourceTypeId());
    if (rb) {
        rb->setChecked(true);
    }
}

void KisColorSourceOptionWidget::lodLimitations(KisPaintopLodLimitations *l) const
{
    if (d->option.type() == KisColorSourceOption::TOTAL_RANDOM) {
        l->limitations << KoID("source-total-random", i18nc("PaintOp instant preview limitation", "Source -> Total Random"));
    } else if (d->option.type() == KisColorSourceOption::PATTERN) {
        l->blockers << KoID("source-pattern", i18nc("PaintOp instant preview limitation", "Source -> Pattern"));
    } else if (d->option.type() == KisColorSourceOption::PATTERN_LOCKED) {
        l->blockers << KoID("source-pattern-locked", i18nc("PaintOp instant preview limitation", "Source -> Pattern Locked"));
    }
}


void KisColorSourceOptionWidget::sourceChanged()
{
    for (QMap<QString, QRadioButton*>::iterator it = d->id2radio.begin(); it != d->id2radio.end(); ++it) {
        if (it.value()->isChecked()) {
            d->option.setColorSourceType(it.key());
            break;
        }
    }
    emitSettingChanged();
}
