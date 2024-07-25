/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "TouchDockerWidget.h"
#include "ui_TouchDockerWidget.h"

#include <kis_canvas2.h>
#include <KisViewManager.h>
#include <kis_action_manager.h>
#include <kis_canvas_controller.h>
#include <kactioncollection.h>
#include <kis_action.h>


TouchDockerWidget::TouchDockerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TouchDockerWidget)
{
    ui->setupUi(this);
}

TouchDockerWidget::~TouchDockerWidget()
{
    delete ui;
}

void TouchDockerWidget::setCanvas(KisCanvas2 *canvas)
{
    if (!canvas) {
        unsetCanvas();
        return;
    }

    auto action = [canvas] (const QString &id) {
        QAction *action = canvas->viewManager()->actionManager()->actionByName(id);
        if (!action) {
            return canvas->canvasController()->actionCollection()->action(id);
        }

        return action;
    };

    ui->btnOpen->setAssociatedAction(action("file_open"));
    ui->btnSave->setAssociatedAction(action("file_save"));
    ui->btnSaveAs->setAssociatedAction(action("file_save_as"));

    ui->btnUndo->setAssociatedAction(action("edit_undo"));
    ui->btnRedo->setAssociatedAction(action("edit_redo"));

    ui->btnRotateLeft->setAssociatedAction(action("rotate_canvas_left"));
    ui->btnResetCanvas->setAssociatedAction(action("reset_canvas_rotation"));
    ui->btnRotateRight->setAssociatedAction(action("rotate_canvas_right"));

    ui->btnZoomUp->setAssociatedAction(action("view_zoom_in"));
    ui->btnZoomDown->setAssociatedAction(action("view_zoom_out"));


    ui->btnOpacityUp->setAssociatedAction(action("increase_opacity"));
    ui->btnOpacityDown->setAssociatedAction(action("decrease_opacity"));

    ui->btnBrightnessUp->setAssociatedAction(action("make_brush_color_lighter"));
    ui->btnBrightnessDown->setAssociatedAction(action("make_brush_color_darker"));

    ui->btnBrushSizeUp->setAssociatedAction(action("increase_brush_size"));
    ui->btnBrushSizeDown->setAssociatedAction(action("decrease_brush_size"));

    ui->btnPreviousPreset->setAssociatedAction(action("previous_preset"));
    ui->btnClear->setAssociatedAction(action("clear"));

}

void TouchDockerWidget::unsetCanvas()
{
    ui->btnOpen->setAssociatedAction(nullptr);
    ui->btnSave->setAssociatedAction(nullptr);
    ui->btnSaveAs->setAssociatedAction(nullptr);

    ui->btnUndo->setAssociatedAction(nullptr);
    ui->btnRedo->setAssociatedAction(nullptr);

    ui->btnRotateLeft->setAssociatedAction(nullptr);
    ui->btnResetCanvas->setAssociatedAction(nullptr);
    ui->btnRotateRight->setAssociatedAction(nullptr);

    ui->btnZoomUp->setAssociatedAction(nullptr);
    ui->btnZoomDown->setAssociatedAction(nullptr);


    ui->btnOpacityUp->setAssociatedAction(nullptr);
    ui->btnOpacityDown->setAssociatedAction(nullptr);

    ui->btnBrightnessUp->setAssociatedAction(nullptr);
    ui->btnBrightnessDown->setAssociatedAction(nullptr);

    ui->btnPreviousPreset->setAssociatedAction(nullptr);
    ui->btnClear->setAssociatedAction(nullptr);
}
