/* This file is part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_statusbar.h"

#include <QLabel>
#include <QFontMetrics>
#include <QToolButton>
#include <QPushButton>
#include <QAction>
#include <QToolTip>
#include <QStatusBar>

#include <ksqueezedtextlabel.h>
#include <klocalizedstring.h>
#include <kformat.h>

#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoToolManager.h>
#include <KoViewConverter.h>

#include <KisUsageLogger.h>

#include <kis_icon_utils.h>

#include <kis_types.h>
#include <kis_image.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_selection_manager.h>
#include "kis_memory_statistics_server.h"

#include "KisView.h"
#include "KisDocument.h"
#include "KisViewManager.h"
#include "canvas/kis_canvas2.h"
#include "kis_progress_widget.h"
#include "kis_zoom_manager.h"

#include "KisMainWindow.h"
#include "kis_config.h"

#include "widgets/KisMemoryReportButton.h"

enum {
    IMAGE_SIZE_ID,
    POINTER_POSITION_ID
};

KisStatusBar::KisStatusBar(KisViewManager *viewManager)
    : m_viewManager(viewManager)
    , m_imageView(0)
    , m_statusBar(0)
{
}

void KisStatusBar::setup()
{
    m_selectionStatus = new QToolButton();
    m_selectionStatus->setObjectName("selection status");
    m_selectionStatus->setIconSize(QSize(16,16));
    m_selectionStatus->setAutoRaise(true);
    m_selectionStatus->setEnabled(false);
    updateSelectionIcon();

    m_statusBar = m_viewManager->mainWindow()->statusBar();

    connect(m_selectionStatus, SIGNAL(clicked()), m_viewManager->selectionManager(), SLOT(slotToggleSelectionDecoration()));
    connect(m_viewManager->selectionManager(), SIGNAL(displaySelectionChanged()), SLOT(updateSelectionToolTip()));
    connect(m_viewManager->mainWindow(), SIGNAL(themeChanged()), this, SLOT(updateSelectionIcon()));

    addStatusBarItem(m_selectionStatus);
    m_selectionStatus->setVisible(false);

    m_statusBarStatusLabel = new KSqueezedTextLabel();
    m_statusBarStatusLabel->setObjectName("statsBarStatusLabel");
    m_statusBarStatusLabel->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    m_statusBarStatusLabel->setContentsMargins(5, 5, 5, 5);
    connect(KoToolManager::instance(), SIGNAL(changedStatusText(QString)),
            m_statusBarStatusLabel, SLOT(setText(QString)));
    addStatusBarItem(m_statusBarStatusLabel, 2);
    m_statusBarStatusLabel->setVisible(false);

    m_statusBarProfileLabel = new KSqueezedTextLabel();
    m_statusBarProfileLabel->setObjectName("statsBarProfileLabel");
    m_statusBarProfileLabel->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    m_statusBarProfileLabel->setContentsMargins(5, 5, 5, 5);
    addStatusBarItem(m_statusBarProfileLabel, 3);
    m_statusBarProfileLabel->setVisible(false);

    m_progress = new KisProgressWidget();
    m_progress->setObjectName("ProgressBar");
    addStatusBarItem(m_progress);
    m_progress->setVisible(false);
    connect(m_progress, SIGNAL(sigCancellationRequested()), this, SIGNAL(sigCancellationRequested()));

    m_progressUpdater.reset(new KisProgressUpdater(m_progress, m_progress->progressProxy()));
    m_progressUpdater->setAutoNestNames(true);

    m_memoryReportBox = new KisMemoryReportButton();
    m_memoryReportBox->setObjectName("memoryReportBox");
    m_memoryReportBox->setFlat(true);
    m_memoryReportBox->setContentsMargins(5, 5, 5, 5);
    m_memoryReportBox->setMinimumWidth(120);
    addStatusBarItem(m_memoryReportBox);
    m_memoryReportBox->setVisible(false);

    connect(m_memoryReportBox, SIGNAL(clicked()), SLOT(showMemoryInfoToolTip()));

    m_pointerPositionLabel = new QLabel(QString());
    m_pointerPositionLabel->setObjectName("pointerPositionLabel");
    m_pointerPositionLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_pointerPositionLabel->setMinimumWidth(100);
    m_pointerPositionLabel->setContentsMargins(5,5, 5, 5);
    addStatusBarItem(m_pointerPositionLabel);
    m_pointerPositionLabel->setVisible(false);

    connect(KisMemoryStatisticsServer::instance(),
            SIGNAL(sigUpdateMemoryStatistics()),
            SLOT(imageSizeChanged()));
}

KisStatusBar::~KisStatusBar()
{
}

void KisStatusBar::setView(QPointer<KisView> imageView)
{
    if (m_imageView == imageView) {
        return;
    }

    if (m_imageView) {
        m_imageView->disconnect(this);
        removeStatusBarItem(m_imageView->zoomManager()->zoomActionWidget());
        m_imageView = 0;
    }

    if (imageView) {
        m_imageView = imageView;

        connect(m_imageView, SIGNAL(sigColorSpaceChanged(const KoColorSpace*)),
                this, SLOT(updateStatusBarProfileLabel()));
        connect(m_imageView, SIGNAL(sigProfileChanged(const KoColorProfile*)),
                this, SLOT(updateStatusBarProfileLabel()));
        connect(m_imageView, SIGNAL(sigSizeChanged(QPointF,QPointF)),
                this, SLOT(imageSizeChanged()));
        updateStatusBarProfileLabel();
        addStatusBarItem(m_imageView->zoomManager()->zoomActionWidget());
    }

    imageSizeChanged();
}

void KisStatusBar::addStatusBarItem(QWidget *widget, int stretch, bool permanent)
{
    StatusBarItem sbItem(widget);
    if (permanent) {
        m_statusBar->addPermanentWidget(widget, stretch);
    }
    else {
        m_statusBar->addWidget(widget, stretch);
    }
    widget->setVisible(true);
    m_statusBarItems.append(sbItem);
}

void KisStatusBar::removeStatusBarItem(QWidget *widget)
{
    int i = 0;
    Q_FOREACH(const StatusBarItem& sbItem, m_statusBarItems) {
        if (sbItem.widget() == widget) {
            break;
        }
        i++;
    }

    if (i < m_statusBarItems.count()) {
        m_statusBar->removeWidget(m_statusBarItems[i].widget());
        m_statusBarItems.remove(i);
    }
}

void KisStatusBar::hideAllStatusBarItems()
{
    Q_FOREACH(const StatusBarItem& sbItem, m_statusBarItems) {
        sbItem.hide();
    }
}

void KisStatusBar::showAllStatusBarItems()
{
    Q_FOREACH(const StatusBarItem& sbItem, m_statusBarItems) {
        sbItem.show();
    }
}

void KisStatusBar::documentMousePositionChanged(const QPointF &pos)
{
    if (!m_imageView) return;

    QPoint pixelPos = m_imageView->image()->documentToImagePixelFloored(pos);

    pixelPos.setX(qBound(0, pixelPos.x(), m_viewManager->image()->width() - 1));
    pixelPos.setY(qBound(0, pixelPos.y(), m_viewManager->image()->height() - 1));
    m_pointerPositionLabel->setText(QString("%1, %2").arg(pixelPos.x()).arg(pixelPos.y()));
}

void KisStatusBar::imageSizeChanged()
{
    updateMemoryStatus();

    QString sizeText;
    KisImageWSP image = m_imageView ? m_imageView->image() : 0;
    if (image) {
        qint32 w = image->width();
        qint32 h = image->height();
        sizeText = QString("%1 &x %2 (%3)").arg(w).arg(h).arg(m_shortMemoryTag);
    } else {
        sizeText = m_shortMemoryTag;
    }

    m_memoryReportBox->setIcon(m_memoryStatusIcon);
    m_memoryReportBox->setText(sizeText);
    m_memoryReportBox->setToolTip(m_longMemoryTag);
}

void KisStatusBar::updateSelectionIcon()
{
    QIcon icon;
    if (!m_viewManager->selectionManager()->displaySelection()) {
        icon = KisIconUtils::loadIcon("selection-mode_invisible");
    } else if (m_viewManager->selectionManager()->showSelectionAsMask()) {
        icon = KisIconUtils::loadIcon("selection-mode_mask");
    } else /* if (!m_view->selectionManager()->showSelectionAsMask()) */ {
        icon = KisIconUtils::loadIcon("selection-mode_ants");
    }
    m_selectionStatus->setIcon(icon);
}

void KisStatusBar::updateMemoryStatus()
{
    KisMemoryStatisticsServer::Statistics stats =
            KisMemoryStatisticsServer::instance()
            ->fetchMemoryStatistics(m_imageView ? m_imageView->image() : 0);
    const KFormat format;

    const QString imageStatsMsg =
            i18nc("tooltip on statusbar memory reporting button (image stats)",
                  "Image size:\t %1\n"
                  "  - layers:\t\t %2\n"
                  "  - projections:\t %3\n"
                  "  - instant preview:\t %4\n",
                  format.formatByteSize(stats.imageSize),
                  format.formatByteSize(stats.layersSize),
                  format.formatByteSize(stats.projectionsSize),
                  format.formatByteSize(stats.lodSize));

    const QString memoryStatsMsg =
            i18nc("tooltip on statusbar memory reporting button (total stats)",
                  "Memory used:\t %1 / %2\n"
                  "  image data:\t %3 / %4\n"
                  "  pool:\t\t %5 / %6\n"
                  "  undo data:\t %7\n"
                  "\n"
                  "Swap used:\t %8",
                  format.formatByteSize(stats.totalMemorySize),
                  format.formatByteSize(stats.totalMemoryLimit),

                  format.formatByteSize(stats.realMemorySize),
                  format.formatByteSize(stats.tilesHardLimit),

                  format.formatByteSize(stats.poolSize),
                  format.formatByteSize(stats.tilesPoolLimit),

                  format.formatByteSize(stats.historicalMemorySize),
                  format.formatByteSize(stats.swapSize));

    QString longStats = imageStatsMsg + "\n" + memoryStatsMsg;

    QString shortStats = format.formatByteSize(stats.imageSize);
    QIcon icon;
    const qint64 warnLevel = stats.tilesHardLimit - stats.tilesHardLimit / 8;

    if (stats.imageSize > warnLevel ||
            stats.realMemorySize > warnLevel) {

        if (!m_memoryWarningLogged) {
            m_memoryWarningLogged = true;
            KisUsageLogger::log(QString("WARNING: %1 is running out of memory:%2\n").arg(m_imageView->document()->url().toLocalFile()).arg(longStats));
        }

        icon = KisIconUtils::loadIcon("warning");
        QString suffix =
                i18nc("tooltip on statusbar memory reporting button",
                      "\n\nWARNING:\tOut of memory! Swapping has been started.\n"
                      "\t\tPlease configure more RAM for Krita in Settings dialog");
        longStats += suffix;


    }

    m_shortMemoryTag = shortStats;
    m_longMemoryTag = longStats;
    m_memoryStatusIcon = icon;

    m_memoryReportBox->setMaximumMemory(stats.totalMemoryLimit);
    m_memoryReportBox->setCurrentMemory(stats.totalMemorySize);
    m_memoryReportBox->setImageWeight(stats.imageSize);

    emit memoryStatusUpdated();
}

void KisStatusBar::showMemoryInfoToolTip()
{
    QToolTip::showText(QCursor::pos(), m_memoryReportBox->toolTip(), m_memoryReportBox);
}

void KisStatusBar::updateSelectionToolTip()
{
    updateSelectionIcon();

    KisSelectionSP selection = m_viewManager->selection();
    if (selection) {
        m_selectionStatus->setEnabled(true);

        QRect r = selection->selectedExactRect();

        QString displayMode =
                !m_viewManager->selectionManager()->displaySelection() ?
                    i18n("Hidden") :
                    (m_viewManager->selectionManager()->showSelectionAsMask() ?
                         i18n("Mask") : i18n("Ants"));

        m_selectionStatus->setToolTip(
                    i18n("Selection: x = %1 y = %2 width = %3 height = %4\n"
                         "Display Mode: %5",
                         r.x(), r.y(), r.width(), r.height(), displayMode));
    } else {
        m_selectionStatus->setEnabled(false);
        m_selectionStatus->setToolTip(i18n("No Selection"));
    }
}

void KisStatusBar::setSelection(KisImageWSP image)
{
    Q_UNUSED(image);
    updateSelectionToolTip();
}

void KisStatusBar::setProfile(KisImageWSP image)
{
    if (m_statusBarProfileLabel == 0) {
        return;
    }

    if (!image) return;
    if (image->profile() == 0) {
        m_statusBarProfileLabel->setText(i18n("No profile"));
    } else {
        m_statusBarProfileLabel->setText(image->colorSpace()->name() + "  " + image->profile()->name());
    }

}

void KisStatusBar::setHelp(const QString &t)
{
    Q_UNUSED(t);
}

void KisStatusBar::updateStatusBarProfileLabel()
{
    if (!m_imageView) return;

    setProfile(m_imageView->image());
}

KoProgressUpdater *KisStatusBar::progressUpdater()
{
    return m_progressUpdater.data();
}


