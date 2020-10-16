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
#include <kis_icon_utils.h>

#include <WidgetsDebug.h>
#include <klocalizedstring.h>

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

    d->floatIcon = kisIcon("docker_float");
    d->floatButton = new KoDockWidgetTitleBarButton(this);
    d->floatButton->setIcon(d->floatIcon);
    connect(d->floatButton, SIGNAL(clicked()), SLOT(toggleFloating()));
    d->floatButton->setVisible(true);
    d->floatButton->setToolTip(i18nc("@info:tooltip", "Float Docker"));
    d->floatButton->setStyleSheet("border: 0");

    d->removeIcon = kisIcon("docker_close");
    d->closeButton = new KoDockWidgetTitleBarButton(this);
    d->closeButton->setIcon(d->removeIcon);
    connect(d->closeButton, SIGNAL(clicked()), q, SLOT(close()));
    d->closeButton->setVisible(true);
    d->closeButton->setToolTip(i18nc("@info:tooltip", "Close Docker"));   
    d->closeButton->setStyleSheet("border: 0"); // border makes the header busy looking (appears on some OSs)

    d->openIcon = kisIcon("docker_collapse_a");
    d->closeIcon = kisIcon("docker_collapse_b");
    d->collapseButton = new KoDockWidgetTitleBarButton(this);
    d->collapseButton->setIcon(d->openIcon);
    connect(d->collapseButton, SIGNAL(clicked()), SLOT(toggleCollapsed()));
    d->collapseButton->setVisible(true);
    d->collapsable = true;
    d->collapseButton->setToolTip(i18nc("@info:tooltip", "Collapse Docker"));
    d->collapseButton->setStyleSheet("border: 0");

    d->lockIcon = kisIcon("docker_lock_a");
    d->lockButton = new KoDockWidgetTitleBarButton(this);
    d->lockButton->setCheckable(true);
    d->lockButton->setIcon(d->lockIcon);
    connect(d->lockButton, SIGNAL(toggled(bool)), SLOT(setLocked(bool)));
    d->lockButton->setVisible(true);
    d->lockable = true;
    d->lockButton->setToolTip(i18nc("@info:tooltip", "Lock Docker"));
    d->lockButton->setStyleSheet("border: 0");

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
    if (isHidden()) {
        return QSize(0, 0);
    }

    QDockWidget *q = qobject_cast<QDockWidget*>(parentWidget());

    int mw = q->style()->pixelMetric(QStyle::PM_DockWidgetTitleMargin, 0, q);
    int fw = q->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, q);

    // get size of buttons...
    QSize closeSize(0, 0);
    if (d->closeButton && hasFeature(q, QDockWidget::DockWidgetClosable)) {
        closeSize = d->closeButton->sizeHint();
    }

    QSize floatSize(0, 0);
    if (d->floatButton && hasFeature(q, QDockWidget::DockWidgetFloatable)) {
        floatSize = d->floatButton->sizeHint();
    }

    QSize hideSize(0, 0);
    if (d->collapseButton && d->collapsable) {
        hideSize = d->collapseButton->sizeHint();
    }

    QSize lockSize(0, 0);
    if (d->lockButton && d->lockable) {
        lockSize = d->lockButton->sizeHint();
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
        if (d->textVisibilityMode == FullTextAlwaysVisible) {
            return QSize(buttonWidth /*+ height*/ + 2*mw + 2*fw + titleWidth, height);
        }
        else {
            if (q->widget()) {
                return QSize(qMin(q->widget()->minimumSizeHint().width(), buttonWidth), height);
            }
            else {
                return QSize(buttonWidth, height);
            }
        }
    }
}

void KoDockWidgetTitleBar::paintEvent(QPaintEvent*)
{
    QStylePainter p(this);

    QDockWidget *q = qobject_cast<QDockWidget*>(parentWidget());

    int fw = q->isFloating() ? q->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, q) : 0;
    int mw = q->style()->pixelMetric(QStyle::PM_DockWidgetTitleMargin, 0, q);

    QStyleOptionDockWidget titleOpt;
    titleOpt.initFrom(q);

    QSize collapseButtonSize(0,0);
    if (d->collapsable && d->collapseButton->isVisible()) {
        collapseButtonSize = d->collapseButton->size();
    }

    QSize lockButtonSize(0,0);
    if (d->lockable && d->lockButton->isVisible()) {
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

    QStyleOptionDockWidget opt;
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

    const QSize lockRectSize = size;

    if (q->isFloating() || (width() < (closeRect.width() + lockRectSize.width()) + 50)) {
        d->collapsable = false;
        d->collapseButton->setVisible(false);
        d->lockButton->setVisible(false);
        d->lockable = false;
    } else {
        d->collapsable = d->collapsableSet;
        d->collapseButton->setVisible(d->collapsableSet);
        d->lockButton->setVisible(true);
        d->lockable = true;
    }

    int offset = 0;

    if (d->collapsable) {
        offset = collapseRect.width();
    }
    QRect lockRect = QRect(QPoint(fw + 2 + offset, top), lockRectSize);
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

    d->locked = locked;
    d->lockButton->blockSignals(true);
    d->lockButton->setChecked(locked);
    d->lockButton->blockSignals(false);

    //qDebug() << "setlocked" << q << d->locked << locked;

    if (locked) {
        d->features = q->features();
        q->setFeatures(QDockWidget::NoDockWidgetFeatures);
    }
    else {
        q->setFeatures(d->features);
    }

    q->toggleViewAction()->setEnabled(!locked);
    d->closeButton->setEnabled(!locked);
    d->floatButton->setEnabled(!locked);
    d->collapseButton->setEnabled(!locked);

    d->updateIcons();
    q->setProperty("Locked", locked);
    resizeEvent(0);
}


void KoDockWidgetTitleBar::setCollapsable(bool collapsable)
{
    d->collapsableSet = collapsable;
    d->collapsable = collapsable;
    d->collapseButton->setVisible(collapsable);
}

void KoDockWidgetTitleBar::setTextVisibilityMode(TextVisibilityMode textVisibilityMode)
{
    d->textVisibilityMode = textVisibilityMode;
}

void KoDockWidgetTitleBar::updateIcons()
{
    d->updateIcons();
}

void KoDockWidgetTitleBar::Private::toggleFloating()
{
    QDockWidget *q = qobject_cast<QDockWidget*>(thePublic->parentWidget());

    q->setFloating(!q->isFloating());
    updateIcons();
}

void KoDockWidgetTitleBar::Private::topLevelChanged(bool topLevel)
{
    lockButton->setEnabled(!topLevel);
    updateIcons();
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
        collapseButton->setIcon(q->widget()->isHidden() ? kisIcon("docker_collapse_b") : kisIcon("docker_collapse_a"));
    }
}

void KoDockWidgetTitleBar::Private::featuresChanged(QDockWidget::DockWidgetFeatures)
{
    QDockWidget *q = qobject_cast<QDockWidget*>(thePublic->parentWidget());

    closeButton->setVisible(hasFeature(q, QDockWidget::DockWidgetClosable));
    floatButton->setVisible(hasFeature(q, QDockWidget::DockWidgetFloatable));

    thePublic->resizeEvent(0);
}


void KoDockWidgetTitleBar::Private::updateIcons()
{    
    QDockWidget *q = qobject_cast<QDockWidget*>(thePublic->parentWidget());

    lockIcon = (!locked) ? kisIcon("docker_lock_a") : kisIcon("docker_lock_b");
    lockButton->setIcon(lockIcon);

    // this method gets called when switching themes, so update all of the themed icons now
   floatButton->setIcon(kisIcon("docker_float"));
   closeButton->setIcon(kisIcon("docker_close"));

    if (q->widget()) {
        collapseButton->setIcon(q->widget()->isHidden() ? kisIcon("docker_collapse_b") : kisIcon("docker_collapse_a"));
    }
    thePublic->resizeEvent(0);

}

//have to include this because of Q_PRIVATE_SLOT
#include "moc_KoDockWidgetTitleBar.cpp"
