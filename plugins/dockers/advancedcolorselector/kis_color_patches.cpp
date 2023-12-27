/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *  SPDX-FileCopyrightText: 2023 Sharaf Zaman <shzam@sdf.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_color_patches.h"

#include <QApplication>
#include <QLayout>
#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include "kis_canvas2.h"
#include "KoCanvasResourceProvider.h"
#include "KisColorPatchesTableView.h"
#include "kis_display_color_converter.h"

KisColorPatches::KisColorPatches(QString configPrefix, QWidget *parent)
    : KisColorSelectorBase(parent)
    , m_configPrefix(configPrefix)
    , m_colorPatchesView(new KisColorPatchesTableView(configPrefix, parent))
{
    updateSettings();
}

void KisColorPatches::mouseReleaseEvent(QMouseEvent* event)
{
    event->ignore();
    KisColorSelectorBase::mouseReleaseEvent(event);
    if (event->isAccepted() || !rect().contains(event->pos()))
        return;

    if (!m_canvas) return;

    boost::optional<KoColor> isColor = m_colorPatchesView->colorPatchAt(event->globalPos());
    if (!isColor) {
        return;
    }

    KoColor color = *isColor;
    if (event->button() == Qt::LeftButton) {
        m_canvas->resourceManager()->setForegroundColor(color);
    } else if (event->button() == Qt::RightButton) {
        m_canvas->resourceManager()->setBackgroundColor(color);
    }
    event->accept();
}

void KisColorPatches::mousePressEvent(QMouseEvent *event)
{
    boost::optional<KoColor> isColor = m_colorPatchesView->colorPatchAt(event->globalPos());
    if (!isColor) {
        return;
    }

    KoColor color = *isColor;

    KisColorSelectorBase::mousePressEvent(event);
    if (event->isAccepted())
        return;

    updateColorPreview(color);
    event->accept();
}

int KisColorPatches::patchCount() const
{
    return m_colorPatchesView->patchCount();
}

void KisColorPatches::setCanvas(KisCanvas2 *canvas)
{
    KisColorSelectorBase::setCanvas(canvas);
}

void KisColorPatches::unsetCanvas()
{
    KisColorSelectorBase::unsetCanvas();
}

void KisColorPatches::addColorPatch(const KoColor &color)
{
    m_colorPatchesView->addColorPatch(color);
}

void KisColorPatches::setAdditionalButtons(QList<QWidget*> buttonList)
{
    for (int i = 0; i < buttonList.size(); i++) {
        buttonList.at(i)->setParent(this);
    }
    m_buttonList = buttonList;
}

void KisColorPatches::updateSettings()
{
    KisColorSelectorBase::updateSettings();

    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");
    m_colorPatchesView->reloadWidgetConfig();

    QBoxLayout::Direction layoutDirection;
    if (cfg.readEntry(m_configPrefix + "Alignment", false)) {
        m_direction = Vertical;
        layoutDirection = QBoxLayout::TopToBottom;
    } else {
        m_direction = Horizontal;
        layoutDirection = QBoxLayout::LeftToRight;
    }

    QBoxLayout *boxLayout = dynamic_cast<QBoxLayout*>(layout());
    if (!boxLayout) {
        boxLayout = new QBoxLayout(layoutDirection, this);
        boxLayout->setContentsMargins(0, 0, 0, 0);
        setLayout(boxLayout);
        layout()->addWidget(m_colorPatchesView);
    } else if (boxLayout->direction() != layoutDirection) {
        boxLayout->setDirection(layoutDirection);
    }


    if (isPopup()) {
        if (m_direction == Vertical) {
            setMinimumWidth(m_colorPatchesView->width());
            setMaximumWidth(m_colorPatchesView->width());
        } else {
            setMinimumHeight(m_colorPatchesView->height());
            setMaximumHeight(m_colorPatchesView->height());
        }
    }

    for (int i = 0; i < m_buttonList.size(); i++) {
        m_buttonList.at(i)->setGeometry(QRect(QPoint(0, 0), m_colorPatchesView->cellSize()));
    }

    setPopupBehaviour(false, false);
    update();
}


void KisColorPatches::setColors(const QList<KoColor> &colors)
{
    m_colorPatchesView->setColors(colors);
}

QList<KoColor> KisColorPatches::colors() const
{
    return m_colorPatchesView->colors();
}
