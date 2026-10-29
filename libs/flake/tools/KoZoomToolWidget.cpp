/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Martin Pfeiffer <hubipete@gmx.net>
 * SPDX-FileCopyrightText: 2026 Arkady Flury <arkady.flury@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoZoomToolWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include "KoCanvasBase.h"
#include <KoCanvasController.h>
#include <KoIcon.h>
#include "KoZoomTool.h"

#include <kactioncollection.h>

#include "KisOptionButtonStrip.h"
#include "KoGroupButton.h"
#include "KisOptionCollectionWidget.h"


KoZoomToolWidget::KoZoomToolWidget(KoZoomTool* tool, QWidget* parent)
    : QWidget(parent)
    , m_tool(tool)
{
    setupUi(this);
    KisOptionButtonStrip *zoomModeButtonStripAction = new KisOptionButtonStrip;
    zoomModeButtonStripAction->addButton(
        KisIconUtils::loadIcon("list-add"));
    zoomModeButtonStripAction->addButton(
        KisIconUtils::loadIcon("minus"));

    zoomModeButtonStripAction->button(0)->setChecked(true);


    zoomModeButtonStripAction->button(0)->setToolTip(
        i18n("Zoom in"));
    zoomModeButtonStripAction->button(1)->setToolTip(
        i18n("Zoom out"));

    KisOptionCollectionWidgetWithHeader *zoomMode =
        new KisOptionCollectionWidgetWithHeader(
            i18n("Mode"));

    zoomMode->setPrimaryWidget(zoomModeButtonStripAction);
    qobject_cast<QVBoxLayout*>(layout())->insertWidget(0,zoomMode);


    connect(zoomModeButtonStripAction,
            QOverload<int, bool>::of(&KisOptionButtonStrip::buttonToggled),
            [this](int index, int checked) {
                if (checked) {
                    m_tool->setZoomInMode(!bool(index));
                }
            });

    KisKActionCollection *actionCollection = m_tool->canvas()->canvasController()->actionCollection();

    QAction *action = actionCollection->action("zoom_to_fit");
    btnZoomToFit->setDefaultAction(action);

    action = actionCollection->action("zoom_to_fit_width");
    btnZoomToFitH->setDefaultAction(action);

    action = actionCollection->action("zoom_to_fit_height");
    btnZoomToFitV->setDefaultAction(action);
}

KoZoomToolWidget::~KoZoomToolWidget()
{
}
