/* This file is part of the KDE project
   Copyright (c) 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KoDockWidgetTitleBar.h"
#include "KoDockWidgetTitleBar_p.h"
#include "KoDockWidgetTitleBarButton.h"

#include <KoIcon.h>

#include <kdebug.h>
#include <klocale.h>

#include <QAbstractButton>
#include <QAction>
#include <QLabel>
#include <QLayout>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionFrame>

static inline bool hasFeature(const QDockWidget *dockwidget, QDockWidget::DockWidgetFeature feature)
{
    return (dockwidget->features() & feature) == feature;
}

KoDockWidgetTitleBar::KoDockWidgetTitleBar(QDockWidget* dockWidget)
        : QWidget(dockWidget), d(new Private(this))
{
    QDockWidget *q = dockWidget;

    d->floatButton = new KoDockWidgetTitleBarButton(this);
    d->floatButton->setIcon(q->style()->standardIcon(QStyle::SP_TitleBarNormalButton, 0, q));
    connect(d->floatButton, SIGNAL(clicked()), SLOT(toggleFloating()));
    d->floatButton->setVisible(true);
    d->floatButton->setToolTip(i18nc("@info:tooltip", "Float Docker"));

    d->closeButton = new KoDockWidgetTitleBarButton(this);
    d->closeButton->setIcon(q->style()->standardIcon(QStyle::SP_TitleBarCloseButton, 0, q));
    connect(d->closeButton, SIGNAL(clicked()), q, SLOT(close()));
    d->closeButton->setVisible(true);
    d->closeButton->setToolTip(i18nc("@info:tooltip", "Close Docker"));

    d->collapseButton = new KoDockWidgetTitleBarButton(this);
    d->collapseButton->setIcon(d->openIcon);
    connect(d->collapseButton, SIGNAL(clicked()), SLOT(toggleCollapsed()));
    d->collapseButton->setVisible(false);
    d->collapseButton->setToolTip(i18nc("@info:tooltip", "Collapse Docker"));

    d->lockIcon = themedIcon("docker_lock_b");
    d->lockButton = new KoDockWidgetTitleBarButton(this);
    d->lockButton->setIcon(d->lockIcon);
    connect(d->lockButton, SIGNAL(clicked()), SLOT(toggleLocked()));
    d->lockButton->setVisible(true);
    d->lockButton->setToolTip(i18nc("@info:tooltip", "Lock Docker"));

    connect(dockWidget, SIGNAL(featuresChanged(QDockWidget::DockWidgetFeatures)), SLOT(featuresChanged(QDockWidget::DockWidgetFeatures)));
    connect(dockWidget, SIGNAL(topLevelChanged(bool)), SLOT(topLevelChanged(bool)));
    d->featuresChanged(0);
}

KoDockWidgetTitleBar::~KoDockWidgetTitleBar()
{
    delete d;
}

QSize KoDockWidgetTitleBar::minimumSizeHint() const
{
    return sizeHint();
}

QSize KoDockWidgetTitleBar::sizeHint() const
{
    QDockWidget *q = qobject_cast<QDockWidget*>(parentWidget());

    int mw = q->style()->pixelMetric(QStyle::PM_DockWidgetTitleMargin, 0, q);
    int fw = q->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, q);

    // get size of buttons...
    QSize closeSize(0, 0);
    if (d->closeButton && d->closeButton->isVisible()) {
        closeSize = d->closeButton->sizeHint();
    }
    QSize floatSize(0, 0);
    if (d->floatButton && d->floatButton->isVisible()) {
        floatSize = d->floatButton->sizeHint();
    }
    QSize hideSize(0, 0);
    if (d->collapseButton && d->collapseButton->isVisible()) {
        hideSize = d->collapseButton->sizeHint();
    }

    d->lockButton->setIcon(d->lockIcon);

    QSize lockSize(0, 0);
    if (d->lockButton && d->lockButton->isVisible()) {
        hideSize = d->lockButton->sizeHint();
    }

    int buttonHeight = qMax(qMax(qMax(closeSize.height(), floatSize.height()), hideSize.height()), lockSize.height()) + 2;
    int buttonWidth = closeSize.width() + floatSize.width() + hideSize.width() + lockSize.width();

    int height = buttonHeight;
    if (d->textVisibilityMode == FullTextAlwaysVisible) {
        // get font size
        QFontMetrics titleFontMetrics = q->fontMetrics();
        int fontHeight = titleFontMetrics.lineSpacing() + 2 * mw;

        height = qMax(height, fontHeight);
    }

    /*
     * Calculate the width of title and add to the total width of the docker window when collapsed.
     */
    const int titleWidth =
        (d->textVisibilityMode == FullTextAlwaysVisible) ? (q->fontMetrics().width(q->windowTitle()) + 2*mw) :
                                                           0;

    if (d->preCollapsedWidth > 0) {
        return QSize(d->preCollapsedWidth, height);
    }
    else {
        return QSize(buttonWidth /*+ height*/ + 2*mw + 2*fw + titleWidth, height);
    }
}

void KoDockWidgetTitleBar::paintEvent(QPaintEvent*)
{
    QStylePainter p(this);

    QDockWidget *q = qobject_cast<QDockWidget*>(parentWidget());

    int fw = q->isFloating() ? q->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, q) : 0;
    int mw = q->style()->pixelMetric(QStyle::PM_DockWidgetTitleMargin, 0, q);

    QStyleOptionDockWidgetV2 titleOpt;
    titleOpt.initFrom(q);
    QSize collapseButtonSize(0,0);
    if (d->collapseButton->isVisible()) {
        collapseButtonSize = d->collapseButton->size();
    }
    QSize lockButtonSize(0,0);
    if (d->lockButton->isVisible()) {
        lockButtonSize = d->lockButton->size();
    }

    titleOpt.rect = QRect(QPoint(fw + mw + collapseButtonSize.width() + lockButtonSize.width(), 0),
                          QSize(geometry().width() - (fw * 2) -  mw - collapseButtonSize.width() - lockButtonSize.width(), geometry().height()));
    titleOpt.title = q->windowTitle();
    titleOpt.closable = hasFeature(q, QDockWidget::DockWidgetClosable);
    titleOpt.floatable = hasFeature(q, QDockWidget::DockWidgetFloatable);
    p.drawControl(QStyle::CE_DockWidgetTitle, titleOpt);
}

void KoDockWidgetTitleBar::resizeEvent(QResizeEvent*)
{
    QDockWidget *q = qobject_cast<QDockWidget*>(parentWidget());
    int fw = q->isFloating() ? q->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, q) : 0;
    QStyleOptionDockWidgetV2 opt;
    opt.initFrom(q);
    opt.rect = QRect(QPoint(fw, fw), QSize(geometry().width() - (fw * 2), geometry().height() - (fw * 2)));
    opt.title = q->windowTitle();
    opt.closable = hasFeature(q, QDockWidget::DockWidgetClosable);
    opt.floatable = hasFeature(q, QDockWidget::DockWidgetFloatable);

    QRect floatRect = q->style()->subElementRect(QStyle::SE_DockWidgetFloatButton, &opt, q);
    if (!floatRect.isNull())
        d->floatButton->setGeometry(floatRect);

    QRect closeRect = q->style()->subElementRect(QStyle::SE_DockWidgetCloseButton, &opt, q);
    if (!closeRect.isNull())
        d->closeButton->setGeometry(closeRect);

    int top = fw;
    if (!floatRect.isNull())
        top = floatRect.y();
    else if (!closeRect.isNull())
        top = closeRect.y();

    QSize size = d->collapseButton->size();
    if (!closeRect.isNull()) {
        size = d->closeButton->size();
    } else if (!floatRect.isNull()) {
        size = d->floatButton->size();
    }
    QRect collapseRect = QRect(QPoint(fw, top), size);
    d->collapseButton->setGeometry(collapseRect);

    size = d->lockButton->size();
    if (!closeRect.isNull()) {
        size = d->closeButton->size();
    } else if (!floatRect.isNull()) {
        size = d->floatButton->size();
    }
    int offset = 0;
    if (d->collapseButton->isVisible()) {
        offset = collapseRect.width();
    }
    QRect lockRect = QRect(QPoint(fw + 2 + offset, top), size);
    d->lockButton->setGeometry(lockRect);


}

void KoDockWidgetTitleBar::setCollapsed(bool collapsed)
{
    QDockWidget *q = qobject_cast<QDockWidget*>(parentWidget());
    if (q && q->widget() && q->widget()->isHidden() != collapsed)
        d->toggleCollapsed();
}

void KoDockWidgetTitleBar::setLocked(bool locked)
{
    QDockWidget *q = qobject_cast<QDockWidget*>(parentWidget());

    if (q && q->widget() && d->locked != locked)
        d->toggleLocked();
}


void KoDockWidgetTitleBar::setCollapsable(bool collapsable)
{
    d->collapseButton->setVisible(collapsable);
}

void KoDockWidgetTitleBar::setTextVisibilityMode(TextVisibilityMode textVisibilityMode)
{
    d->textVisibilityMode = textVisibilityMode;
}

void KoDockWidgetTitleBar::Private::toggleFloating()
{
    QDockWidget *q = qobject_cast<QDockWidget*>(thePublic->parentWidget());

    q->setFloating(!q->isFloating());
}

void KoDockWidgetTitleBar::Private::topLevelChanged(bool topLevel)
{
    lockButton->setEnabled(!topLevel);
}

void KoDockWidgetTitleBar::Private::toggleCollapsed()
{
    QDockWidget *q = qobject_cast<QDockWidget*>(thePublic->parentWidget());
    if (q == 0) // there does not *have* to be anything on the dockwidget.
        return;

    preCollapsedWidth = q->widget()->isHidden() ? -1 : thePublic->width();
    q->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX); // will be overwritten again next
    if (q->widget()) {
        q->widget()->setVisible(q->widget()->isHidden());
        collapseButton->setIcon(q->widget()->isHidden() ? closeIcon : openIcon);
    }
}

void KoDockWidgetTitleBar::Private::toggleLocked()
{
    QDockWidget *q = qobject_cast<QDockWidget*>(thePublic->parentWidget());
    if (q == 0) // there does not *have* to be anything on the dockwidget.
        return;

    if (!locked) {
        locked = true;
        lockIcon = themedIcon("docker_lock_a");
        lockButton->setIcon(themedIcon("docker_lock_a"));
        features = q->features();
        q->setFeatures(QDockWidget::NoDockWidgetFeatures);
        closeButton->setEnabled(false);
        floatButton->setEnabled(false);
        collapseButton->setEnabled(false);
    }
    else {
        locked = false;
        lockIcon = themedIcon("docker_lock_b");
        lockButton->setIcon(themedIcon("docker_lock_b"));
        q->setFeatures(features);
        closeButton->setEnabled(true);
        floatButton->setEnabled(true);
        collapseButton->setEnabled(true);
    }
    q->setProperty("Locked", locked);

}


void KoDockWidgetTitleBar::Private::featuresChanged(QDockWidget::DockWidgetFeatures)
{
    QDockWidget *q = qobject_cast<QDockWidget*>(thePublic->parentWidget());
    closeButton->setVisible(hasFeature(q, QDockWidget::DockWidgetClosable));
    floatButton->setVisible(hasFeature(q, QDockWidget::DockWidgetFloatable));

    thePublic->resizeEvent(0);
}

#include <KoDockWidgetTitleBar.moc>
