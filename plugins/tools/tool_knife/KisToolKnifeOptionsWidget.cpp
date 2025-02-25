/*
 *  SPDX-FileCopyrightText: 2025 Agata Cacko
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisToolKnifeOptionsWidget.h"

#include "ui_KisToolKnifeOptionsWidget.h"

#include <KoColorSpaceRegistry.h>
#include "KisPaletteModel.h"

#include "kis_config.h"
#include <resources/KoColorSet.h>
#include "kis_canvas_resource_provider.h"


struct KisToolKnifeOptionsWidget::Private {
    Private()
    {
    }

    Ui_KisToolKnifeOptionsWidget *ui {nullptr};

    int getThickGapWidth(void)
    {
        return ui->thickGapWidth->value();
    }

    int getThinGapWidth(void)
    {
        return ui->thinGapWidth->value();
    }

    int getSpecialGapWidth(void)
    {
        return ui->specialGapWidth->value();
    }

    KisToolKnifeOptionsWidget::GapWidthType getWidthType()
    {
        if (ui->thickGapWidthRadioButton->isChecked()) {
            return KisToolKnifeOptionsWidget::Thick;
        } else if (ui->thinGapWidthRadioButton->isChecked()) {
            return KisToolKnifeOptionsWidget::Thin;
        } else { // ui->specialGapWidthRadioButton->isChecked()
            return KisToolKnifeOptionsWidget::Special;
        }
    }

    int getWidthForType(GapWidthType type) {
        switch(type) {
            case KisToolKnifeOptionsWidget::Thick:
                return getThickGapWidth();
            case KisToolKnifeOptionsWidget::Thin:
                return getThinGapWidth();
            case KisToolKnifeOptionsWidget::Special:
                return getSpecialGapWidth();
            default:
                return getSpecialGapWidth();
        }
    }

    KisToolKnifeOptionsWidget::ToolMode getToolMode() {
        // TODO: obviously gonna be removed later,
        // replaced with buttons with icons like the Transform Tool has
        if (ui->comboBox->currentText() == "Add a gutter") {
            return KisToolKnifeOptionsWidget::AddGutter;
        } else {
            return KisToolKnifeOptionsWidget::RemoveGutter;
        }
    }

};

KisToolKnifeOptionsWidget::KisToolKnifeOptionsWidget(KisCanvasResourceProvider */*provider*/, QWidget *parent)
    : QWidget(parent),
      m_d(new Private)
{
    m_d->ui = new Ui_KisToolKnifeOptionsWidget();
    m_d->ui->setupUi(this);
}

KisToolKnifeOptionsWidget::~KisToolKnifeOptionsWidget()
{
    delete m_d->ui;
    m_d->ui = nullptr;
}

int KisToolKnifeOptionsWidget::getThickGapWidth()
{
    return m_d->getThickGapWidth();
}

int KisToolKnifeOptionsWidget::getThinGapWidth()
{
    return m_d->getThinGapWidth();
}

int KisToolKnifeOptionsWidget::getSpecialGapWidth()
{
    return m_d->getSpecialGapWidth();
}

KisToolKnifeOptionsWidget::GapWidthType KisToolKnifeOptionsWidget::getWidthType()
{
    return m_d->getWidthType();
}

int KisToolKnifeOptionsWidget::getCurrentWidth()
{
    return getWidthForType(getWidthType());
}

int KisToolKnifeOptionsWidget::getWidthForType(GapWidthType type)
{
    return m_d->getWidthForType(type);

}

KisToolKnifeOptionsWidget::ToolMode KisToolKnifeOptionsWidget::getToolMode()
{
    return m_d->getToolMode();
}




