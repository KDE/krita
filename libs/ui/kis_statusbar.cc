/* This file is part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include <QHBoxLayout>

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
#include <KisAngleSelector.h>
#include <kis_canvas_controller.h>
#include <kis_signals_blocker.h>

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

#ifdef Q_OS_ANDROID
    m_fullscreenToggle = new QToolButton;
    m_fullscreenToggle->setObjectName("Toggle Fullscreen");
    m_fullscreenToggle->setCheckable(false);
    m_fullscreenToggle->setToolTip(i18n("Toggle Fullscreen"));
    m_fullscreenToggle->setAutoRaise(true);
    m_fullscreenToggle->setIcon(KisIconUtils::loadIcon("zoom-horizontal"));
    addStatusBarItem(m_fullscreenToggle);
    m_fullscreenToggle->setVisible(true);
    connect(m_fullscreenToggle, SIGNAL(clicked()), m_viewManager, SLOT(slotToggleFullscreen()));
#endif

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

    m_extraWidgetsParent = new QFrame;
    m_extraWidgetsParent->setMinimumWidth(50);
    m_extraWidgetsParent->setObjectName("Extra Widgets Parent");
    m_extraWidgetsLayout = new QHBoxLayout;
    m_extraWidgetsLayout->setContentsMargins(0, 0, 0, 0);
    m_extraWidgetsLayout->setObjectName("Extra Widgets Layout");
    m_extraWidgetsParent->setLayout(m_extraWidgetsLayout);
    addStatusBarItem(m_extraWidgetsParent);

    m_memoryReportBox = new KisMemoryReportButton();
    m_memoryReportBox->setObjectName("memoryReportBox");
    m_memoryReportBox->setFlat(true);
    m_memoryReportBox->setContentsMargins(5, 5, 5, 5);
    m_memoryReportBox->setMinimumWidth(120);
    addStatusBarItem(m_memoryReportBox);
    m_memoryReportBox->setVisible(false);

    connect(m_memoryReportBox, SIGNAL(clicked()), SLOT(showMemoryInfoToolTip()));

    connect(KisMemoryStatisticsServer::instance(),
            SIGNAL(sigUpdateMemoryStatistics()),
            SLOT(imageSizeChanged()));

    m_canvasAngleSelector = new KisAngleSelector;
    m_canvasAngleSelector->setRange(-360.00, 360.0);
    m_canvasAngleSelector->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);
    m_canvasAngleSelector->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_ContextMenu);
    m_canvasAngleSelector->useFlatSpinBox(true);
    addStatusBarItem(m_canvasAngleSelector);

    connect(m_canvasAngleSelector, SIGNAL(angleChanged(qreal)), SLOT(slotCanvasAngleSelectorAngleChanged(qreal)));
    m_canvasAngleSelector->setVisible(false);
}

KisStatusBar::~KisStatusBar()
{
}

void KisStatusBar::setView(QPointer<KisView> imageView)
{
    if (m_imageView) {
        if (m_imageView->canvasBase()) {
            m_imageView->canvasBase()->canvasController()->proxyObject->disconnect(this);
        }
        m_imageView->disconnect(this);
        removeStatusBarItem(m_imageView->zoomManager()->zoomActionWidget());
        m_imageView = 0;
    }

    if (imageView) {
        m_imageView = imageView;
        m_canvasAngleSelector->setVisible(true);
        connect(m_imageView, SIGNAL(sigColorSpaceChanged(const KoColorSpace*)),
                this, SLOT(updateStatusBarProfileLabel()));
        connect(m_imageView, SIGNAL(sigProfileChanged(const KoColorProfile*)),
                this, SLOT(updateStatusBarProfileLabel()));
        connect(m_imageView, SIGNAL(sigSizeChanged(QPointF,QPointF)),
                this, SLOT(imageSizeChanged()));
        connect(m_imageView->canvasController()->proxyObject, SIGNAL(canvasOffsetXChanged(int)),
                this, SLOT(slotCanvasRotationChanged()));
        updateStatusBarProfileLabel();
        slotCanvasRotationChanged();
        addStatusBarItem(m_imageView->zoomManager()->zoomActionWidget());
    }
    else {
        m_canvasAngleSelector->setVisible(false);
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


void KisStatusBar::imageSizeChanged()
{
    updateMemoryStatus();

    QString sizeText;
    KisImageWSP image = m_imageView ? m_imageView->image() : 0;
    if (image) {
        qint32 w = image->width();
        qint32 h = image->height();
        sizeText = i18nc("@info:status width x height (file size)", "%1 &x %2 (%3)", w, h, m_shortMemoryTag);
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
            KisUsageLogger::log(QString("WARNING: %1 is running out of memory:%2\n").arg(m_imageView->document()->path()).arg(longStats));
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

void KisStatusBar::slotCanvasAngleSelectorAngleChanged(qreal angle)
{
    KisCanvas2 *canvas = m_viewManager->canvasBase();
    if (!canvas) return;

    KisCanvasController *canvasController = dynamic_cast<KisCanvasController*>(canvas->canvasController());
    if (canvasController) {
        canvasController->rotateCanvas(angle - canvas->rotationAngle());
    }
}

void KisStatusBar::slotCanvasRotationChanged()
{
    KisCanvas2 *canvas = m_viewManager->canvasBase();
    if (!canvas) return;

    KisSignalsBlocker l(m_canvasAngleSelector);
    m_canvasAngleSelector->setAngle(canvas->rotationAngle());
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
        m_statusBarProfileLabel->setText(i18nc("<color space> <image profile>", "%1  %2", image->colorSpace()->name(), image->profile()->name()));
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

void KisStatusBar::addExtraWidget(QWidget *widget)
{
    m_extraWidgetsLayout->addWidget(widget);
}

void KisStatusBar::removeExtraWidget(QWidget *widget)
{
    m_extraWidgetsLayout->removeWidget(widget);
}

void KisStatusBar::setStatusBarStatusLabelText(const QString &text)
{
    m_statusBarStatusLabel->setText(text);
}

