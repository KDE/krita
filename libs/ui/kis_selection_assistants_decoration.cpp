/*
 *  SPDX-FileCopyrightText: 2025 Ross Rosales <ross.erosales@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_selection_assistants_decoration.h"

#include <limits>

#include <QList>
#include <QPointF>
#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <ktoggleaction.h>
#include <kis_algebra_2d.h>
#include "kis_debug.h"
#include "KisDocument.h"
#include "kis_canvas2.h"
#include "kis_canvas_resource_provider.h"
#include "kis_icon_utils.h"
#include "KisViewManager.h"
#include <KoCompositeOpRegistry.h>
#include "kis_tool_proxy.h"
#include "kis_selection_manager.h"
#include <QTransform>
#include <QPushButton>

#include <QPainter>
#include <QPainterPath>
#include <QApplication>

class KisSelectionManager;

struct KisSelectionAssistantsDecoration::Private {
    Private()
    {}
    KisCanvas2 * m_canvas = 0;
    QPushButton *buttonSelectAll = nullptr;
    QPushButton *buttonDeselect = nullptr;
    QPushButton *buttonCopyToNewLayer = nullptr;
    KisSelectionManager *selectionManager = nullptr;
    KisViewManager *m_viewManager = nullptr;
};



KisSelectionAssistantsDecoration::KisSelectionAssistantsDecoration() :
    d(new Private)
{
}

KisSelectionAssistantsDecoration::~KisSelectionAssistantsDecoration()
{
    delete d;
}

void KisSelectionAssistantsDecoration::setViewManager(KisViewManager* viewManager) {
    d->m_viewManager = viewManager;
    d->selectionManager = viewManager->selectionManager();
}

void KisSelectionAssistantsDecoration::drawDecoration(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter, KisCanvas2* canvas, bool m_selectionActionBar)
{
    Q_UNUSED(updateRect);
    QWidget *canvasWidget = dynamic_cast<QWidget*>(canvas->canvasWidget());

    if (!d->buttonSelectAll) {
            d->buttonSelectAll = new QPushButton();
            d->buttonSelectAll->setFixedSize(25, 25);
            d->buttonSelectAll->setToolTip("select all");
            d->buttonSelectAll->setStyleSheet("background-color: rgba(255,255,255,180); border: 1px solid gray;");

            connect(d->buttonSelectAll, &QPushButton::clicked, d->selectionManager, &KisSelectionManager::selectAll);
        }

    if (!d->buttonDeselect) {
        d->buttonDeselect = new QPushButton();
        d->buttonDeselect->setFixedSize(25, 25);
        d->buttonDeselect->setToolTip("deselect");
        d->buttonDeselect->setStyleSheet("background-color: rgba(255,255,255,180); border: 1px solid gray;");

        connect(d->buttonDeselect, &QPushButton::clicked, d->selectionManager, &KisSelectionManager::deselect);
    }

    if (!d->buttonCopyToNewLayer) {
        d->buttonCopyToNewLayer = new QPushButton();
        d->buttonCopyToNewLayer->setFixedSize(25, 25);
        d->buttonCopyToNewLayer->setToolTip("Copy To New Layer");
        d->buttonCopyToNewLayer->setStyleSheet("background-color: rgba(255,255,255,180); border: 1px solid gray;");

        connect(d->buttonCopyToNewLayer, &QPushButton::clicked, d->selectionManager, &KisSelectionManager::copySelectionToNewLayer);
    }

    if (canvasWidget && d->buttonSelectAll && m_selectionActionBar) {
           d->buttonSelectAll->setParent(canvasWidget);
           d->buttonSelectAll->move(200, 512);
           d->buttonSelectAll->show();
       } else if (d->buttonSelectAll) {
           d->buttonSelectAll->hide();
       }

    if (canvasWidget && d->buttonDeselect && m_selectionActionBar) {
            d->buttonDeselect->setParent(canvasWidget);
            d->buttonDeselect->move(225, 512);
            d->buttonDeselect->show();
        } else if (d->buttonDeselect) {
            d->buttonDeselect->hide();
        }

    if (canvasWidget && d->buttonCopyToNewLayer && m_selectionActionBar) {
            d->buttonCopyToNewLayer->setParent(canvasWidget);
            d->buttonCopyToNewLayer->move(250, 512);
            d->buttonCopyToNewLayer->show();
        } else if (d->buttonCopyToNewLayer) {
            d->buttonCopyToNewLayer->hide();
        }

    if (!m_selectionActionBar) {
        return;
    }

    QPainterPath bgPath;
    bgPath.addRoundedRect(QRectF(200, 512, 128, 67), 6, 6); // placeholder position and size
    gc.fillPath(bgPath, Qt::darkGray);

    QPainterPath dragRect;
    int width = 240;
    int height = 67;
    dragRect.addRect(QRectF(300, 512, width, height));
    gc.fillPath(bgPath.intersected(dragRect),Qt::lightGray);

    QPainterPath dragRectDots;
    QColor dragDecorationDotsColor(50,50,50,255);
    int dotSize = 2;
    dragRectDots.addEllipse(3,2.5,dotSize,dotSize);
    dragRectDots.addEllipse(3,7.5,dotSize,dotSize);
    dragRectDots.addEllipse(3,-2.5,dotSize,dotSize);
    dragRectDots.addEllipse(3,-7.5,dotSize,dotSize);
    dragRectDots.addEllipse(-3,2.5,dotSize,dotSize);
    dragRectDots.addEllipse(-3,7.5,dotSize,dotSize);
    dragRectDots.addEllipse(-3,-2.5,dotSize,dotSize);
    dragRectDots.addEllipse(-3,-7.5,dotSize,dotSize);
    dragRectDots.translate(315, 542);

    gc.fillPath(dragRectDots,dragDecorationDotsColor);
}
