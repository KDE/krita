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

#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include "kis_icon_utils.h"

#include <kis_types.h>
#include <kis_image.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_selection_manager.h>
#include "kis_memory_statistics_server.h"

#include <KisView.h>
#include "KisViewManager.h"
#include "canvas/kis_canvas2.h"
#include "kis_progress_widget.h"
#include "kis_zoom_manager.h"

#include <KoToolManager.h>
#include <KoViewConverter.h>
#include <KisMainWindow.h>

enum {
    IMAGE_SIZE_ID,
    POINTER_POSITION_ID
};

KisStatusBar::KisStatusBar(KisViewManager * view)
        : m_view(view)
        , m_imageView(0)
{
}

void KisStatusBar::setup()
{
    m_selectionStatus = new QToolButton();
    m_selectionStatus->setIconSize(QSize(16,16));
    m_selectionStatus->setAutoRaise(true);
    m_selectionStatus->setEnabled(false);   
    updateSelectionIcon();

    connect(m_selectionStatus, SIGNAL(clicked()), m_view->selectionManager(), SLOT(slotToggleSelectionDecoration()));
    connect(m_view->selectionManager(), SIGNAL(displaySelectionChanged()), SLOT(updateSelectionToolTip()));
    connect(m_view->mainWindow(), SIGNAL(themeChanged()), this, SLOT(updateSelectionIcon()));

    m_view->addStatusBarItem(m_selectionStatus);

    m_statusBarStatusLabel = new KSqueezedTextLabel();
    m_statusBarStatusLabel->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    m_statusBarStatusLabel->setContentsMargins(5, 5, 5, 5);
    connect(KoToolManager::instance(), SIGNAL(changedStatusText(const QString &)),
            m_statusBarStatusLabel, SLOT(setText(const QString &)));
    m_view->addStatusBarItem(m_statusBarStatusLabel, 2);

    m_statusBarProfileLabel = new KSqueezedTextLabel();
    m_statusBarProfileLabel->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    m_statusBarProfileLabel->setContentsMargins(5, 5, 5, 5);
    m_view->addStatusBarItem(m_statusBarProfileLabel, 3);

    m_progress = new KisProgressWidget();
    m_view->addStatusBarItem(m_progress);

    m_memoryReportBox = new QPushButton();
    m_memoryReportBox->setFlat(true);
    m_memoryReportBox->setContentsMargins(5, 5, 5, 5);
    m_memoryReportBox->setMinimumWidth(120);
    m_view->addStatusBarItem(m_memoryReportBox);

    connect(m_memoryReportBox, SIGNAL(clicked()), SLOT(showMemoryInfoToolTip()));

    m_pointerPositionLabel = new QLabel(QString());
    m_pointerPositionLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_pointerPositionLabel->setMinimumWidth(100);
    m_pointerPositionLabel->setContentsMargins(5,5, 5, 5);
    m_view->addStatusBarItem(m_pointerPositionLabel);
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
        m_view->removeStatusBarItem(m_imageView->zoomManager()->zoomActionWidget());
        m_imageView = 0;
    }
    if (imageView) {
        m_imageView = imageView;

        connect(m_imageView, SIGNAL(sigColorSpaceChanged(const KoColorSpace*)),
                this, SLOT(updateStatusBarProfileLabel()));
        connect(m_imageView, SIGNAL(sigProfileChanged(const KoColorProfile*)),
                this, SLOT(updateStatusBarProfileLabel()));
        connect(m_imageView, SIGNAL(sigSizeChanged(const QPointF&, const QPointF&)),
                this, SLOT(imageSizeChanged()));
        updateStatusBarProfileLabel();
        m_view->addStatusBarItem(m_imageView->zoomManager()->zoomActionWidget());
    }

    imageSizeChanged();
}

void KisStatusBar::documentMousePositionChanged(const QPointF &pos)
{
    if (!m_imageView) return;

    QPoint pixelPos = m_imageView->image()->documentToIntPixel(pos);

    pixelPos.setX(qBound(0, pixelPos.x(), m_view->image()->width() - 1));
    pixelPos.setY(qBound(0, pixelPos.y(), m_view->image()->height() - 1));
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
        sizeText = QString("%1 x %2 (%3)").arg(w).arg(h).arg(m_shortMemoryTag);
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
    if (!m_view->selectionManager()->displaySelection()) {
        icon = KisIconUtils::loadIcon("selection-mode_invisible");
    } else if (m_view->selectionManager()->showSelectionAsMask()) {
        icon = KisIconUtils::loadIcon("selection-mode_mask");
    } else /* if (!m_view->selectionManager()->showSelectionAsMask()) */ {
        icon = KisIconUtils::loadIcon("selection-mode_ants");
    }
    m_selectionStatus->setIcon(icon);
}

inline QString formatSize(qint64 size)
{
    qint64 K = 1024;
    QString suffix = i18nc("very shortened \'byte\' suffix (for statusbar)", "b");

    if (size > K) {
        size /= K;
        suffix = i18nc("very shortened KiB suffix (for statusbar)", "K");
    }

    if (size > K) {
        size /= K;
        suffix = i18nc("very shortened MiB suffix (for statusbar)", "M");
    }

    if (size > K) {
        size /= K;
        suffix = i18nc("very shortened GiB suffix (for statusbar)", "G");
    }

    if (size > K) {
        size /= K;
        suffix = i18nc("very shortened TiB suffix (for statusbar)", "T");
    }

    return QString("%2%3").arg(size).arg(suffix);
}

void KisStatusBar::updateMemoryStatus()
{
    KisMemoryStatisticsServer::Statistics stats =
        KisMemoryStatisticsServer::instance()
        ->fetchMemoryStatistics(m_imageView ? m_imageView->image() : 0);

    QString longStats =
        i18nc("tooltip on statusbar memory reporting button",
              "Image size:\t %1\n"
              "\n"
              "Memory used:\t %2 / %3\n"
              "  image data:\t %4 / %5\n"
              "  pool:\t\t %6 / %7\n"
              "  undo data:\t %8\n"
              "\n"
              "Swap used:\t %9",
              formatSize(stats.imageSize),

              formatSize(stats.totalMemorySize),
              formatSize(stats.totalMemoryLimit),

              formatSize(stats.realMemorySize),
              formatSize(stats.tilesHardLimit),

              formatSize(stats.poolSize),
              formatSize(stats.tilesPoolLimit),

              formatSize(stats.historicalMemorySize),
              formatSize(stats.swapSize));

    QString shortStats = formatSize(stats.imageSize);
    QIcon icon;
    qint64 warnLevel = stats.tilesHardLimit - stats.tilesHardLimit / 8;

    if (stats.imageSize > warnLevel ||
        stats.realMemorySize > warnLevel) {

        icon = KisIconUtils::loadIcon("dialog-warning");
        QString suffix =
            i18nc("tooltip on statusbar memory reporting button",
                  "\n\nWARNING:\tOut of memory! Swapping has been started.\n"
                  "\t\tPlease configure more RAM for Krita in Settings dialog");
        longStats += suffix;
    }

    m_shortMemoryTag = shortStats;
    m_longMemoryTag = longStats;
    m_memoryStatusIcon = icon;
}

void KisStatusBar::showMemoryInfoToolTip()
{
    QToolTip::showText(QCursor::pos(), m_memoryReportBox->toolTip(), m_memoryReportBox);
}

void KisStatusBar::updateSelectionToolTip()
{
    updateSelectionIcon();

    KisSelectionSP selection = m_view->selection();
    if (selection) {
        m_selectionStatus->setEnabled(true);

        QRect r = selection->selectedExactRect();

        QString displayMode =
            !m_view->selectionManager()->displaySelection() ?
            i18n("Hidden") :
            (m_view->selectionManager()->showSelectionAsMask() ?
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


KisProgressWidget* KisStatusBar::progress()
{
    return m_progress;
}


