/*
 * SPDX-FileCopyrightText: 2005-2009 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2009 Peter Simonsson <peter.simonsson@gmail.com>
 * SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2022 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoToolBoxDocker_p.h"
#include "KoToolBox_p.h"
#include "KoToolBoxScrollArea_p.h"

#include <QLabel>
#include <QFontMetrics>
#include <QFrame>
#include <QAction>
#include <QMenu>

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

KoToolBoxDocker::KoToolBoxDocker(KoToolBox *toolBox)
    : QDockWidget(i18n("Toolbox"))
    , m_toolBox(toolBox)
    , m_scrollArea(new KoToolBoxScrollArea(toolBox, this))
{
    setWidget(m_scrollArea);

    QLabel *w = new QLabel(" ", this);
    w->setFrameShape(QFrame::StyledPanel);
    w->setFrameShadow(QFrame::Raised);
    w->setFrameStyle(QFrame::Panel | QFrame::Raised);
    QFont font = qApp->font();
    font.setPointSizeF(font.pointSizeF() * 0.9);
    int titleSize = QFontMetrics(font).height();
    w->setMinimumSize(titleSize, titleSize);
    setTitleBarWidget(w);

    KConfigGroup cfg =  KSharedConfig::openConfig()->group("KoToolBox");
    const int layoutDirUnchecked = cfg.readEntry<int>("layoutDir", Qt::LayoutDirectionAuto);
    switch (layoutDirUnchecked) {
    case Qt::LayoutDirectionAuto:
    case Qt::LeftToRight:
    case Qt::RightToLeft:
        m_layoutDir = static_cast<Qt::LayoutDirection>(layoutDirUnchecked);
        break;
    default:
        m_layoutDir = Qt::LayoutDirectionAuto;
        break;
    }
    updateLayoutDir();

    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(updateToolBoxOrientation(Qt::DockWidgetArea)));
    connect(this, SIGNAL(topLevelChanged(bool)),
            this, SLOT(updateFloating(bool)));
}

void KoToolBoxDocker::setCanvas(KoCanvasBase *canvas)
{
    Q_UNUSED(canvas);
}

void KoToolBoxDocker::unsetCanvas()
{
}

void KoToolBoxDocker::resizeEvent(QResizeEvent *event)
{
    QDockWidget::resizeEvent(event);
    if (isFloating()) {
        if (width() > height()) {
            setToolBoxOrientation(Qt::Horizontal);
        } else {
            setToolBoxOrientation(Qt::Vertical);
        }
    }
}

void KoToolBoxDocker::updateToolBoxOrientation(Qt::DockWidgetArea area)
{
    m_dockArea = area;
    updateLayoutDir();
    if (area == Qt::TopDockWidgetArea || area == Qt::BottomDockWidgetArea) {
        setToolBoxOrientation(Qt::Horizontal);
    } else {
        setToolBoxOrientation(Qt::Vertical);
    }
}

void KoToolBoxDocker::updateLayoutDir()
{
    if (m_layoutDir == Qt::LayoutDirectionAuto) {
        if (m_dockArea == Qt::RightDockWidgetArea) {
            m_scrollArea->setLayoutDirection(Qt::RightToLeft);
        } else if (m_dockArea == Qt::LeftDockWidgetArea) {
            m_scrollArea->setLayoutDirection(Qt::LeftToRight);
        } else {
            m_scrollArea->unsetLayoutDirection();
        }
    } else {
        m_scrollArea->setLayoutDirection(m_layoutDir);
    }
}

void KoToolBoxDocker::changeLayoutDir(Qt::LayoutDirection dir)
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group("KoToolBox");
    cfg.writeEntry<int>("layoutDir", dir);
    m_layoutDir = dir;
    updateLayoutDir();
}

void KoToolBoxDocker::setToolBoxOrientation(Qt::Orientation orientation)
{
    if (orientation == Qt::Horizontal) {
        setFeatures(features() | QDockWidget::DockWidgetVerticalTitleBar);
        m_scrollArea->setOrientation(Qt::Horizontal);
    } else {
        setFeatures(features() & ~QDockWidget::DockWidgetVerticalTitleBar);
        m_scrollArea->setOrientation(Qt::Vertical);
    }
}

void KoToolBoxDocker::updateFloating(bool v)
{
    m_toolBox->setFloating(v);
}

void KoToolBoxDocker::contextMenuEvent(QContextMenuEvent *event)
{
    if (!m_contextMenu) {
        m_contextMenu = new QMenu(this);
        m_contextMenu->addSection(i18n("Icon Size"));
        m_toolBox->setupIconSizeMenu(m_contextMenu);

        m_contextMenu->addSection(i18nc("Toolbox layout", "Layout"));
        QActionGroup *layoutActionGroup = new QActionGroup(m_contextMenu);

        QAction *layoutAuto = m_contextMenu->addAction(i18nc("@item:inmenu Toolbox layout direction", "&Automatic"));
        layoutAuto->setActionGroup(layoutActionGroup);
        layoutAuto->setCheckable(true);
        connect(layoutAuto, &QAction::triggered, this, [this]() {
            changeLayoutDir(Qt::LayoutDirectionAuto);
        });

        QAction *layoutLtr = m_contextMenu->addAction(i18nc("@item:inmenu Toolbox layout direction", "&Left-to-right"));
        layoutLtr->setActionGroup(layoutActionGroup);
        layoutLtr->setCheckable(true);
        connect(layoutLtr, &QAction::triggered, this, [this]() {
            changeLayoutDir(Qt::LeftToRight);
        });

        QAction *layoutRtl = m_contextMenu->addAction(i18nc("@item:inmenu Toolbox layout direction", "&Right-to-left"));
        layoutRtl->setActionGroup(layoutActionGroup);
        layoutRtl->setCheckable(true);
        connect(layoutRtl, &QAction::triggered, this, [this]() {
            changeLayoutDir(Qt::RightToLeft);
        });

        switch (m_layoutDir) {
        case Qt::LayoutDirectionAuto:
            layoutAuto->setChecked(true);
            break;
        case Qt::LeftToRight:
            layoutLtr->setChecked(true);
            break;
        case Qt::RightToLeft:
            layoutRtl->setChecked(true);
            break;
        }
    }

    m_contextMenu->exec(event->globalPos());
}
