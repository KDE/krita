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
#include <KisOptionButtonStrip.h>
#include <kis_icon_utils.h>
#include <KoGroupButton.h>


struct KisToolKnifeOptionsWidget::Private {
    Private()
    {
    }

    Ui_KisToolKnifeOptionsWidget *ui {nullptr};
    KoGroupButton* buttonModeAddGutter {nullptr};
    KoGroupButton* buttonModeRemoveGutter {nullptr};
    KoGroupButton* buttonModeMoveGutterEndPoint {nullptr};
    //KoGroupButton* m_buttonModeAddGutter {nullptr};



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
        return ui->customGapWidth->value();
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

    int getWidthForType(KisToolKnifeOptionsWidget::GapWidthType type) {
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
        if (buttonModeAddGutter && buttonModeAddGutter->isChecked()) {
            return KisToolKnifeOptionsWidget::AddGutter;
        } else if (buttonModeRemoveGutter && buttonModeRemoveGutter->isChecked()) {
            return KisToolKnifeOptionsWidget::RemoveGutter;
        } else if (buttonModeMoveGutterEndPoint && buttonModeMoveGutterEndPoint->isChecked()) {
            return KisToolKnifeOptionsWidget::MoveGutterEndPoint;
        } else {
            // default
            return KisToolKnifeOptionsWidget::AddGutter;
        }
    }

};

KisToolKnifeOptionsWidget::KisToolKnifeOptionsWidget(KisCanvasResourceProvider */*provider*/, QWidget *parent)
    : QWidget(parent),
      m_d(new Private)
{
    m_d->ui = new Ui_KisToolKnifeOptionsWidget();
    m_d->ui->setupUi(this);
    //m_d->ui->optionButtonStripToolMode->addButton(KisIconUtils::loadIcon("tool_comic_panel_scissors"));
    //KisOptionButtonStrip *optionButtonStripToolMode =
    //    new KisOptionButtonStrip;
    m_d->buttonModeAddGutter = m_d->ui->optionButtonStripToolMode->addButton(
        KisIconUtils::loadIcon("tool_comic_panel_scissors"));
    m_d->buttonModeRemoveGutter = m_d->ui->optionButtonStripToolMode->addButton(
        KisIconUtils::loadIcon("tool_comic_panel_zipper"));
    m_d->buttonModeMoveGutterEndPoint = m_d->ui->optionButtonStripToolMode->addButton(
        KisIconUtils::loadIcon("tool_comic_panel_move_point"));

    m_d->buttonModeAddGutter->setChecked(true);
    m_d->buttonModeAddGutter->setMinimumSize(QSize(36, 36));
    m_d->buttonModeRemoveGutter->setMinimumSize(QSize(36, 36));
    m_d->buttonModeAddGutter->setIconSize(QSize(28, 28));
    m_d->buttonModeRemoveGutter->setIconSize(QSize(28, 28));
    m_d->buttonModeMoveGutterEndPoint->setMinimumSize(QSize(36, 36));
    m_d->buttonModeMoveGutterEndPoint->setIconSize(QSize(28, 28));



    //m_d->ui->mainLayout->addWidget(optionButtonStripToolMode);
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




