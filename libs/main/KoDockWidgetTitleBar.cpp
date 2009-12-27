/* This file is part of the KDE project
   Copyright (c) 2007 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
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

#include <kdebug.h>
#include <kicon.h>

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


class KoDockWidgetTitleBarButton : public QAbstractButton
{
public:
    KoDockWidgetTitleBarButton(KoDockWidgetTitleBar *titleBar);

    QSize sizeHint() const;
    inline QSize minimumSizeHint() const {
        return sizeHint();
    }

    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual void paintEvent(QPaintEvent *event);

    QSize m_styleSize;
    int m_iconSize;
};

KoDockWidgetTitleBarButton::KoDockWidgetTitleBarButton(KoDockWidgetTitleBar *titleBar)
        : QAbstractButton(titleBar)
{
    setFocusPolicy(Qt::NoFocus);
    m_iconSize = 0;
    m_styleSize = QSize(0,0);
}

QSize KoDockWidgetTitleBarButton::sizeHint() const
{
    ensurePolished();

    const int margin = style()->pixelMetric(QStyle::PM_DockWidgetTitleBarButtonMargin, 0, this);
    if (icon().isNull())
        return QSize(margin, margin);
    
    int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize, 0, this);
    if (iconSize != m_iconSize) {
        const_cast<KoDockWidgetTitleBarButton*>(this)->m_iconSize = iconSize;
        const QPixmap pm = icon().pixmap(iconSize);
        const_cast<KoDockWidgetTitleBarButton*>(this)->m_styleSize = QSize(pm.width() + margin, pm.height() + margin);
    }
    return m_styleSize;
}

// redraw the button when the mouse enters or leaves it
void KoDockWidgetTitleBarButton::enterEvent(QEvent *event)
{
    if (isEnabled()) update();
    QAbstractButton::enterEvent(event);
}

void KoDockWidgetTitleBarButton::leaveEvent(QEvent *event)
{
    if (isEnabled()) update();
    QAbstractButton::leaveEvent(event);
}

void KoDockWidgetTitleBarButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    QRect r = rect();
    QStyleOptionToolButton opt;
    opt.init(this);
    opt.state |= QStyle::State_AutoRaise;

    if (isEnabled() && underMouse() && !isChecked() && !isDown())
        opt.state |= QStyle::State_Raised;
    if (isChecked())
        opt.state |= QStyle::State_On;
    if (isDown())
        opt.state |= QStyle::State_Sunken;
    style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, &p, this);

    opt.icon = icon();
    opt.subControls = 0;
    opt.activeSubControls = 0;
    opt.features = QStyleOptionToolButton::None;
    opt.arrowType = Qt::NoArrow;
    int size = style()->pixelMetric(QStyle::PM_SmallIconSize, 0, this);
    opt.iconSize = QSize(size, size);
    style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &p, this);
}


class KoDockWidgetTitleBar::Private
{
public:
    Private(KoDockWidgetTitleBar* thePublic) : thePublic(thePublic),
            openIcon(thePublic->style()->standardIcon(QStyle::SP_TitleBarShadeButton)), closeIcon(thePublic->style()->standardIcon(QStyle::SP_TitleBarUnshadeButton)) {if (openIcon.isNull()) openIcon= KIcon("arrow-down");if (closeIcon.isNull()) closeIcon= KIcon("arrow-right");}
    KoDockWidgetTitleBar* thePublic;
    KIcon openIcon, closeIcon;
    QAbstractButton* closeButton;
    QAbstractButton* floatButton;
    QAbstractButton* collapseButton;

    void toggleFloating();
    void toggleCollapsed();
    void featuresChanged(QDockWidget::DockWidgetFeatures features);
};

KoDockWidgetTitleBar::KoDockWidgetTitleBar(QDockWidget* dockWidget)
        : QWidget(dockWidget), d(new Private(this))
{
    QDockWidget *q = dockWidget;

    d->floatButton = new KoDockWidgetTitleBarButton(this);
    d->floatButton->setIcon(q->style()->standardIcon(QStyle::SP_TitleBarNormalButton, 0, q));
    connect(d->floatButton, SIGNAL(clicked()), SLOT(toggleFloating()));
    d->floatButton->setVisible(true);

    d->closeButton = new KoDockWidgetTitleBarButton(this);
    d->closeButton->setIcon(q->style()->standardIcon(QStyle::SP_TitleBarCloseButton, 0, q));
    connect(d->closeButton, SIGNAL(clicked()), q, SLOT(close()));
    d->closeButton->setVisible(true);

    d->collapseButton = new KoDockWidgetTitleBarButton(this);
    d->collapseButton->setIcon(d->openIcon);
    connect(d->collapseButton, SIGNAL(clicked()), SLOT(toggleCollapsed()));
    d->collapseButton->setVisible(true);

    connect(dockWidget, SIGNAL(featuresChanged(QDockWidget::DockWidgetFeatures)), SLOT(featuresChanged(QDockWidget::DockWidgetFeatures)));

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
    if (d->closeButton) {
        closeSize = d->closeButton->sizeHint();
    }
    QSize floatSize(0, 0);
    if (d->floatButton) {
        floatSize = d->floatButton->sizeHint();
    }
    QSize hideSize(0, 0);
    if (d->collapseButton) {
        hideSize = d->collapseButton->sizeHint();
    }

    int buttonHeight = qMax(qMax(closeSize.height(), floatSize.height()), hideSize.height()) + 2;
    int buttonWidth = closeSize.width() + floatSize.width() + hideSize.width();

    // get font size
    QFontMetrics titleFontMetrics = q->fontMetrics();
    int fontHeight = titleFontMetrics.lineSpacing() + 2 * mw;

    int height = qMax(buttonHeight, fontHeight);
    return QSize(buttonWidth + height + 4*mw + 2*fw, height);
}

void KoDockWidgetTitleBar::paintEvent(QPaintEvent*)
{
    QStylePainter p(this);

    QDockWidget *q = qobject_cast<QDockWidget*>(parentWidget());

    int fw = q->isFloating() ? q->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, q) : 0;
    int mw = q->style()->pixelMetric(QStyle::PM_DockWidgetTitleMargin, 0, q);

    QStyleOptionDockWidgetV2 titleOpt;
    titleOpt.initFrom(q);
    titleOpt.rect = QRect(QPoint(fw + mw + d->collapseButton->size().width(), fw),
                          QSize(geometry().width() - (fw * 2) -  mw - d->collapseButton->size().width(), geometry().height() - (fw * 2)));
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
    if (!floatRect.isNull()) top = floatRect.y();
    else if (!closeRect.isNull()) top = closeRect.y();

    QSize size = d->collapseButton->size();
    if (!closeRect.isNull()) {
        size = d->closeButton->size();
    } else if (!floatRect.isNull()) {
        size = d->floatButton->size();
    }
    QRect collapseRect = QRect(QPoint(fw, top), size);
    d->collapseButton->setGeometry(collapseRect);
}

void KoDockWidgetTitleBar::setCollapsed(bool collapsed)
{
    QDockWidget *q = qobject_cast<QDockWidget*>(parentWidget());
    if (q && q->widget() && q->widget()->isHidden() != collapsed)
        d->toggleCollapsed();
}

void KoDockWidgetTitleBar::Private::toggleFloating()
{
    QDockWidget *q = qobject_cast<QDockWidget*>(thePublic->parentWidget());

    q->setFloating(!q->isFloating());
}

void KoDockWidgetTitleBar::Private::toggleCollapsed()
{
    QDockWidget *q = qobject_cast<QDockWidget*>(thePublic->parentWidget());
    if (q == 0) // there does not *have* to be anything on the dockwidget.
        return;
    q->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX); // will be overwritten again next
    if (q->widget()) {
        q->widget()->setVisible(q->widget()->isHidden());
        collapseButton->setIcon(q->widget()->isHidden() ? closeIcon : openIcon);
    }
}

void KoDockWidgetTitleBar::Private::featuresChanged(QDockWidget::DockWidgetFeatures)
{
    QDockWidget *q = qobject_cast<QDockWidget*>(thePublic->parentWidget());
    closeButton->setVisible(hasFeature(q, QDockWidget::DockWidgetClosable));
    floatButton->setVisible(hasFeature(q, QDockWidget::DockWidgetFloatable));
    thePublic->resizeEvent(0);
}
#include <KoDockWidgetTitleBar.moc>
