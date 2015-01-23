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
#include <QAction>

#include <ksqueezedtextlabel.h>
#include <kstatusbar.h>
#include <klocale.h>

#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include "KoIcon.h"

#include <kis_types.h>
#include <kis_image.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_selection_manager.h>

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
    m_selectionStatus = new QToolButton();
    m_selectionStatus->setIconSize(QSize(16,16));
    m_selectionStatus->setAutoRaise(true);
    m_selectionStatus->setEnabled(false);   
    updateSelectionIcon();

    connect(m_selectionStatus, SIGNAL(clicked()), view->selectionManager(), SLOT(slotToggleSelectionDecoration()));
    connect(view->selectionManager(), SIGNAL(displaySelectionChanged()), SLOT(updateSelectionToolTip()));
    connect(view->mainWindow(), SIGNAL(themeChanged()), this, SLOT(updateSelectionIcon()));

    view->addStatusBarItem(m_selectionStatus);

    // XXX: Use the KStatusbar fixed size labels!
    m_statusBarStatusLabel = new KSqueezedTextLabel();
    m_statusBarStatusLabel->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    m_statusBarStatusLabel->setContentsMargins(5, 5, 5, 5);
    connect(KoToolManager::instance(), SIGNAL(changedStatusText(const QString &)),
            m_statusBarStatusLabel, SLOT(setText(const QString &)));
    view->addStatusBarItem(m_statusBarStatusLabel, 2);

    m_statusBarProfileLabel = new KSqueezedTextLabel();
    m_statusBarProfileLabel->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    m_statusBarProfileLabel->setContentsMargins(5, 5, 5, 5);
    view->addStatusBarItem(m_statusBarProfileLabel, 3);

    m_progress = new KisProgressWidget();
    view->addStatusBarItem(m_progress);

    m_imageSizeLabel = new QLabel(QString());
    m_imageSizeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_imageSizeLabel->setContentsMargins(5,5, 5, 5);
    m_imageSizeLabel->setMinimumWidth(100);
    view->addStatusBarItem(m_imageSizeLabel);

    m_pointerPositionLabel = new QLabel(QString());
    m_pointerPositionLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_pointerPositionLabel->setMinimumWidth(100);
    m_pointerPositionLabel->setContentsMargins(5,5, 5, 5);
    view->addStatusBarItem(m_pointerPositionLabel);
    m_pointerPositionLabel->setVisible(false);

}

KisStatusBar::~KisStatusBar()
{
    m_view->removeStatusBarItem(m_selectionStatus);
    m_view->removeStatusBarItem(m_statusBarStatusLabel);
    m_view->removeStatusBarItem(m_statusBarProfileLabel);
    m_view->removeStatusBarItem(m_imageSizeLabel);
    m_view->removeStatusBarItem(m_pointerPositionLabel);
    m_view->removeStatusBarItem(m_progress);
}

void KisStatusBar::setView(QPointer<KisView> imageView)
{
    if (m_imageView == imageView) {
        return;
    }
    if (m_imageView) {
        m_imageView->disconnect(this);
        m_view->removeStatusBarItem(m_imageView->zoomManager()->zoomActionWidget());
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
        imageSizeChanged();
        m_view->addStatusBarItem(m_imageView->zoomManager()->zoomActionWidget());
    }
}

void KisStatusBar::documentMousePositionChanged(const QPointF &pos)
{
    QPoint pixelPos = m_view->image()->documentToIntPixel(pos);

    pixelPos.setX(qBound(0, pixelPos.x(), m_view->image()->width() - 1));
    pixelPos.setY(qBound(0, pixelPos.y(), m_view->image()->height() - 1));
    m_pointerPositionLabel->setText(QString("%1, %2").arg(pixelPos.x()).arg(pixelPos.y()));
}

void KisStatusBar::imageSizeChanged()
{
    KisImageWSP image = m_view->image();
    qint32 w = image->width();
    qint32 h = image->height();

    m_imageSizeLabel->setText(QString("%1 x %2").arg(w).arg(h));
}

void KisStatusBar::updateSelectionIcon()
{
    KIcon icon;
    if (!m_view->selectionManager()->displaySelection()) {
        icon = themedIcon("selection-mode_invisible.png");
    } else if (m_view->selectionManager()->showSelectionAsMask()) {
        icon = themedIcon("selection-mode_mask.png");
    } else /* if (!m_view->selectionManager()->showSelectionAsMask()) */ {
        icon = themedIcon("selection-mode_ants.png");
    }
    m_selectionStatus->setIcon(icon);
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
    setProfile(m_view->image());
}


KisProgressWidget* KisStatusBar::progress()
{
    return m_progress;
}


#include "kis_statusbar.moc"
