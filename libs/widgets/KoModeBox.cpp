/*
 * Copyright (c) 2005-2009 Thomas Zander <zander@kde.org>
 * Copyright (c) 2009 Peter Simonsson <peter.simonsson@gmail.com>
 * Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2011 C. Boemann <cbo@boemann.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoModeBox_p.h"

#include <KoCanvasControllerWidget.h>
#include <KoCanvasBase.h>
#include <KoToolManager.h>
#include <KoShapeLayer.h>
#include <KoInteractionTool.h>

#include <kdebug.h>
#include <kglobalsettings.h>
#include <kconfiggroup.h>
#include <klocale.h>
#include <kselectaction.h>

#include <QMap>
#include <QList>
#include <QToolButton>
#include <QHash>
#include <QSet>
#include <QRect>
#include <QLabel>
#include <QFrame>
#include <QGridLayout>
#include <QApplication>
#include <QTabBar>
#include <QStackedWidget>
#include <QPainter>
#include <QTextLayout>
#include <QMenu>
#include <QScrollBar>

class KoModeBox::Private
{
public:
    Private(KoCanvasController *c)
        : canvas(c->canvas())
        , activeId(-1)
        , iconTextFitted(true)
        , fittingIterations(0)
        , iconMode(IconAndText)
        , verticalTabsSide(TopSide)
        , horizontalTabsSide(LeftSide)
        , horizontalMode(false)
    {
    }

    KoCanvasBase *canvas;
    QGridLayout *layout;
    QList<KoToolButton> buttons; // buttons maintained by toolmanager
    QList<KoToolButton> addedButtons; //buttons in the order added to QToolBox
    QMap<int, QWidget *> addedWidgets;
    QSet<QWidget *> currentAuxWidgets;
    int activeId;
    QTabBar *tabBar;
    QStackedWidget *stack;
    bool iconTextFitted;
    int fittingIterations;
    IconMode iconMode;
    VerticalTabsSide verticalTabsSide;
    HorizontalTabsSide horizontalTabsSide;
    bool horizontalMode;
};

QString KoModeBox::applicationName;

static bool compareButton(const KoToolButton &b1, const KoToolButton &b2)
{
    int b1Level;
    int b2Level;
    if (b1.section.contains(KoModeBox::applicationName)) {
        b1Level = 0;
    } else if (b1.section.contains("main")) {
        b1Level = 1;
    } else {
        b1Level = 2;
    }

    if (b2.section.contains(KoModeBox::applicationName)) {
        b2Level = 0;
    } else if (b2.section.contains("main")) {
        b2Level = 1;
    } else {
        b2Level = 2;
    }

    if (b1Level == b2Level) {
        return b1.priority < b2.priority;
    } else {
        return b1Level < b2Level;
    }
}


KoModeBox::KoModeBox(KoCanvasControllerWidget *canvas, const QString &appName)
    : QWidget()
    , d(new Private(canvas))
{
    applicationName = appName;

    KConfigGroup cfg = KGlobal::config()->group("calligra");
    d->iconMode = (IconMode)cfg.readEntry("ModeBoxIconMode", (int)IconAndText);
    d->verticalTabsSide = (VerticalTabsSide)cfg.readEntry("ModeBoxVerticalTabsSide", (int)TopSide);
    d->horizontalTabsSide = (HorizontalTabsSide)cfg.readEntry("ModeBoxHorizontalTabsSide", (int)LeftSide);

    d->layout = new QGridLayout();
    d->stack = new QStackedWidget();

    d->tabBar = new QTabBar();
    d->tabBar->setExpanding(false);
    if (d->iconMode == IconAndText) {
        if (d->horizontalMode) {
            d->tabBar->setIconSize(QSize(38,32));
        } else {
            d->tabBar->setIconSize(QSize(32,64));
        }
    } else {
        d->tabBar->setIconSize(QSize(22,22));
    }
    d->tabBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    if (d->horizontalMode) {
        switchTabsSide(d->verticalTabsSide);
    } else {
        switchTabsSide(d->horizontalTabsSide);
    }
    d->layout->addWidget(d->stack, 0, 1);

    d->layout->setContentsMargins(0,0,0,0);
    setLayout(d->layout);

    foreach(const KoToolButton &button, KoToolManager::instance()->createToolList()) {
        addButton(button);
    }

    qSort(d->buttons.begin(), d->buttons.end(), compareButton);

    // Update visibility of buttons
    updateShownTools(QList<QString>());

    d->tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(d->tabBar, SIGNAL(currentChanged(int)), this, SLOT(toolSelected(int)));
    connect(d->tabBar, SIGNAL(customContextMenuRequested(QPoint)), SLOT(slotContextMenuRequested(QPoint)));

    connect(KoToolManager::instance(), SIGNAL(changedTool(KoCanvasController *, int)),
            this, SLOT(setActiveTool(KoCanvasController *, int)));
    connect(KoToolManager::instance(), SIGNAL(currentLayerChanged(const KoCanvasController *,const KoShapeLayer*)),
            this, SLOT(setCurrentLayer(const KoCanvasController *,const KoShapeLayer *)));
    connect(KoToolManager::instance(), SIGNAL(toolCodesSelected(QList<QString>)), this, SLOT(updateShownTools(QList<QString>)));
    connect(KoToolManager::instance(),
            SIGNAL(addedTool(KoToolButton,KoCanvasController*)),
            this, SLOT(toolAdded(KoToolButton,KoCanvasController*)));

    connect(canvas, SIGNAL(toolOptionWidgetsChanged(const QList<QPointer<QWidget> > &)),
         this, SLOT(setOptionWidgets(const QList<QPointer<QWidget> > &)));
}

KoModeBox::~KoModeBox()
{
    delete d;
}

void KoModeBox::addButton(const KoToolButton &button)
{
    d->buttons.append(button);
    button.button->setVisible(false);
}

void KoModeBox::locationChanged(Qt::DockWidgetArea area)
{
    resize(0,0);
    switch(area) {
        case Qt::TopDockWidgetArea:
        case Qt::BottomDockWidgetArea:
            d->horizontalMode = true;
            d->layout->removeWidget(d->stack);
            d->layout->addWidget(d->stack, 1, 0);
            d->layout->setColumnStretch(1, 0);
            d->layout->setRowStretch(1, 100);
            break;
        case Qt::LeftDockWidgetArea:
        case Qt::RightDockWidgetArea:
            d->horizontalMode = false;
            d->layout->removeWidget(d->stack);
            d->layout->addWidget(d->stack, 0, 1);
            d->layout->setColumnStretch(1, 100);
            d->layout->setRowStretch(1, 0);
            break;
        default:
            break;
    }
    d->layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    d->layout->invalidate();

    if (d->iconMode == IconAndText) {
        if (d->horizontalMode) {
            d->tabBar->setIconSize(QSize(42,32));
        } else {
            d->tabBar->setIconSize(QSize(32,64));
        }
    } else {
        d->tabBar->setIconSize(QSize(22,22));
    }

    if (d->horizontalMode) {
        switchTabsSide(d->verticalTabsSide);
    } else {
        switchTabsSide(d->horizontalTabsSide);
    }
}

void KoModeBox::setActiveTool(KoCanvasController *canvas, int id)
{
    if (canvas->canvas() == d->canvas) {
        // Clear the minimumSize instigated by the previous tool
        // The new minimumSize will be set in updateShownTools()
        if (d->addedWidgets.contains(d->activeId)) {
            ScrollArea *sa = qobject_cast<ScrollArea *>(d->addedWidgets[d->activeId]->parentWidget()->parentWidget());
            sa->setMinimumWidth(0);
            sa->setMinimumHeight(0);
        }

        d->activeId = id;
        d->tabBar->blockSignals(true);
        int i = 0;
        foreach (const KoToolButton &button, d->addedButtons) {
            if (button.buttonGroupId == d->activeId) {
                d->tabBar->setCurrentIndex(i);
                d->stack->setCurrentIndex(i);
                break;
            }
            ++i;
        }
        d->tabBar->blockSignals(false);
        return;
    }
}

QIcon KoModeBox::createTextIcon(const KoToolButton &button) const
{
    QSize iconSize = d->tabBar->iconSize();
    QFont smallFont  = KGlobalSettings::generalFont();
    qreal pointSize = KGlobalSettings::smallestReadableFont().pointSizeF();
    smallFont.setPointSizeF(pointSize);
    // This must be a QImage, as drawing to a QPixmap outside the
    // UI thread will cause sporadic crashes.
    QImage pm(iconSize, QImage::Format_ARGB32_Premultiplied);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    if (!d->horizontalMode) {
        if (d->horizontalTabsSide == LeftSide ) {
            p.rotate(90);
            p.translate(0,-iconSize.width());
        } else {
            p.rotate(-90);
            p.translate(-iconSize.height(),0);
        }
    }

    button.button->icon().paint(&p, 0, 0, iconSize.height(), 22);

    QTextLayout textLayout(button.button->toolTip(), smallFont, p.device());
    QTextOption option;
    if (d->horizontalMode) {
        option = QTextOption(Qt::AlignVCenter | Qt::AlignHCenter);
    } else {
        option = QTextOption(Qt::AlignTop | Qt::AlignHCenter);
    }
    option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    textLayout.setTextOption(option);
    textLayout.beginLayout();
    qreal height = 0;
    while (1) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid())
            break;

        line.setLineWidth(iconSize.height());
        line.setPosition(QPointF(0, height));
        height += line.height();
    }
    textLayout.endLayout();

    if (textLayout.lineCount() > 2) {
        iconSize.setHeight(iconSize.height() + 8);
        d->tabBar->setIconSize(iconSize);
        d->iconTextFitted = false;
    } else if (height > iconSize.width() - 22) {
        iconSize.setWidth(22 + height);
        d->tabBar->setIconSize(iconSize);
        d->iconTextFitted = false;
    }

    p.setFont(smallFont);
    p.setPen(palette().text().color());
    textLayout.draw(&p, QPoint(0, 22));
    p.end();

    return QIcon(QPixmap::fromImage(pm));
}

QIcon KoModeBox::createSimpleIcon(const KoToolButton &button) const
{
    QSize iconSize = d->tabBar->iconSize();

    // This must be a QImage, as drawing to a QPixmap outside the
    // UI thread will cause sporadic crashes.
    QImage pm(iconSize, QImage::Format_ARGB32_Premultiplied);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    if (!d->horizontalMode) {
        if (d->horizontalTabsSide == LeftSide ) {
            p.rotate(90);
            p.translate(0,-iconSize.width());
        } else {
            p.rotate(-90);
            p.translate(-iconSize.height(),0);
        }
    }

    button.button->icon().paint(&p, 0, 0, iconSize.height(), iconSize.width());

    return QIcon(QPixmap::fromImage(pm));
}

void KoModeBox::addItem(const KoToolButton &button)
{
    QWidget *oldwidget = d->addedWidgets[button.buttonGroupId];
    QWidget *widget;

    // We need to create a new widget in all cases as QToolBox seeems to crash if we reuse
    // a widget (even though the item had been removed)
    QLayout *layout;
    if (!oldwidget) {
        layout = new QGridLayout();
    } else {
        layout = oldwidget->layout();
    }
    widget = new QWidget();
    widget->setLayout(layout);
    layout->setContentsMargins(0,0,0,0);
    layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    d->addedWidgets[button.buttonGroupId] = widget;

    // Create a rotated icon with text
    if (d->iconMode == IconAndText) {
        d->tabBar->addTab(createTextIcon(button), QString());
    } else {
        int index = d->tabBar->addTab(createSimpleIcon(button), QString());
        d->tabBar->setTabToolTip(index, button.button->toolTip());
    }
    d->tabBar->blockSignals(false);
    ScrollArea *sa = new ScrollArea();
    if (d->horizontalMode) {
        sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        sa->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else {
        sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        sa->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }
    sa->setWidgetResizable(true);
    sa->setContentsMargins(0,0,0,0);
    sa->setWidget(widget);
    sa->setFrameShape(QFrame::NoFrame);
    sa->setFocusPolicy(Qt::NoFocus);
    d->stack->addWidget(sa);
    d->addedButtons.append(button);
}

void KoModeBox::updateShownTools(const QList<QString> &codes)
{
    if (d->iconTextFitted) {
        d->fittingIterations = 0;
    }
    d->iconTextFitted = true;

    d->tabBar->blockSignals(true);

    while (d->tabBar->count()) {
        d->tabBar->removeTab(0);
        d->stack->removeWidget(d->stack->widget(0));
    }

    d->addedButtons.clear();

    int newIndex = -1;
    foreach (const KoToolButton &button, d->buttons) {
        QString toolCodes = button.visibilityCode;
        if (button.buttonGroupId == d->activeId) {
            newIndex = d->addedButtons.length();
        }
        if (button.section.contains(applicationName)) {
            addItem(button);
            continue;
        } else if (!button.section.contains("dynamic")
            && !button.section.contains("main")) {
            continue;
        }
        if (toolCodes.startsWith(QLatin1String("flake/"))) {
            addItem(button);
            continue;
        }

        if (toolCodes.endsWith( QLatin1String( "/always"))) {
            addItem(button);
            continue;
        } else if (toolCodes.isEmpty() && codes.count() != 0) {
            addItem(button);
            continue;
        } else {
           foreach (const QString &shapeCode, codes) {
                if (toolCodes.contains(shapeCode)) {
                    addItem(button);
                    break;
                }
           }
        }
    }
    if (newIndex != -1) {
        d->tabBar->setCurrentIndex(newIndex);
        d->stack->setCurrentIndex(newIndex);
    }
    d->tabBar->blockSignals(false);

    if (!d->iconTextFitted &&  d->fittingIterations++ < 8) {
        updateShownTools(codes);
    }
    d->iconTextFitted = true;
}

void KoModeBox::setOptionWidgets(const QList<QPointer<QWidget> > &optionWidgetList)
{
    if (! d->addedWidgets.contains(d->activeId)) return;

    // For some reason we need to set some attr on our placeholder widget here
    // eventhough these settings should be default
    // Otherwise Sheets' celltool's optionwidget looks ugly
    d->addedWidgets[d->activeId]->setAutoFillBackground(false);
    d->addedWidgets[d->activeId]->setBackgroundRole(QPalette::NoRole);

    qDeleteAll(d->currentAuxWidgets);
    d->currentAuxWidgets.clear();

    int cnt = 0;
    QGridLayout *layout = (QGridLayout *)d->addedWidgets[d->activeId]->layout();
    // need to unstretch row/column that have previously been stretched
    layout->setRowStretch(layout->rowCount()-1, 0);
    layout->setRowStretch(layout->columnCount()-1, 0);
    layout->setColumnStretch(0, 0);
    layout->setColumnStretch(1, 0);
    layout->setColumnStretch(2, 0);
    layout->setRowStretch(0, 0);
    layout->setRowStretch(1, 0);
    layout->setRowStretch(2, 0);

    if (d->horizontalMode) {
        layout->setRowStretch(0, 1);
        layout->setRowStretch(1, 2);
        layout->setRowStretch(2, 1);
        layout->setHorizontalSpacing(2);
        layout->setVerticalSpacing(0);
        foreach(QWidget *widget, optionWidgetList) {
            if (!widget->windowTitle().isEmpty()) {
                QLabel *l;
                layout->addWidget(l = new QLabel(widget->windowTitle()), 0, cnt, 1, 1, Qt::AlignLeft);
                d->currentAuxWidgets.insert(l);
            }
            layout->addWidget(widget, 1, cnt++, 2, 1);
            widget->show();
            if (widget != optionWidgetList.last()) {
                QFrame *s;
                layout->addWidget(s = new QFrame(), 1, cnt, 1, 1, Qt::AlignHCenter);
                layout->setColumnMinimumWidth(cnt++, 16);
                s->setFrameStyle(QFrame::VLine | QFrame::Sunken);
                d->currentAuxWidgets.insert(s);
                ++cnt;
            }
            layout->setColumnStretch(cnt, 100);
        }
    } else {
        layout->setColumnStretch(0, 1);
        layout->setColumnStretch(1, 2);
        layout->setColumnStretch(2, 1);
        layout->setHorizontalSpacing(0);
        layout->setVerticalSpacing(2);
        int specialCount = 0;
        foreach(QWidget *widget, optionWidgetList) {
            if (!widget->windowTitle().isEmpty()) {
                QLabel *l;
                layout->addWidget(l = new QLabel(widget->windowTitle()), cnt++, 0, 1, 3, Qt::AlignHCenter);
                d->currentAuxWidgets.insert(l);
            }
            layout->addWidget(widget, cnt++, 0, 1, 3);
            QLayout *subLayout = widget->layout();
            if (subLayout) {
                for (int i = 0; i < subLayout->count(); ++i) {
                    QWidget *spacerWidget = subLayout->itemAt(i)->widget();
                    if (spacerWidget && spacerWidget->objectName().contains("SpecialSpacer")) {
                        specialCount++;
                    }
                }
            }
            widget->show();
            if (widget != optionWidgetList.last()) {
                QFrame *s;
                layout->addWidget(s = new QFrame(), cnt, 1, 1, 1);
                layout->setRowMinimumHeight(cnt++, 16);
                s->setFrameStyle(QFrame::HLine | QFrame::Sunken);
                d->currentAuxWidgets.insert(s);
            }
        }
        if (specialCount == optionWidgetList.count()) {
            layout->setRowStretch(cnt, 100);
        }
    }
}

void ScrollArea::showEvent(QShowEvent *e)
{
    QScrollArea::showEvent(e);
    if (horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff) {
        setMinimumWidth(widget()->minimumSizeHint().width() + (verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0));
    } else {
        setMinimumHeight(widget()->minimumSizeHint().height() + (horizontalScrollBar()->isVisible() ? horizontalScrollBar()->height() : 0));
    }
}

void KoModeBox::setCurrentLayer(const KoCanvasController *canvas, const KoShapeLayer *layer)
{
    Q_UNUSED(canvas);
    Q_UNUSED(layer);
    //Since tageted application don't use this we won't bother implemeting
}

void KoModeBox::setCanvas(KoCanvasBase *canvas)
{
    KoCanvasControllerWidget *ccwidget;

    if (d->canvas) {
        ccwidget = dynamic_cast<KoCanvasControllerWidget *>(d->canvas->canvasController());
        disconnect(ccwidget, SIGNAL(toolOptionWidgetsChanged(const QList<QPointer<QWidget> > &)),
                    this, SLOT(setOptionWidgets(const QList<QPointer<QWidget> > &)));
    }

    d->canvas = canvas;

    ccwidget = dynamic_cast<KoCanvasControllerWidget *>(d->canvas->canvasController());
    connect(
        ccwidget, SIGNAL(toolOptionWidgetsChanged(const QList<QPointer<QWidget> > &)),
         this, SLOT(setOptionWidgets(const QList<QPointer<QWidget> > &)));
}

void KoModeBox::unsetCanvas()
{
    d->canvas = 0;
}

void KoModeBox::toolAdded(const KoToolButton &button, KoCanvasController *canvas)
{
    if (canvas->canvas() == d->canvas) {
        addButton(button);

        qStableSort(d->buttons.begin(), d->buttons.end(), compareButton);

        updateShownTools(QList<QString>());
    }
}

void KoModeBox::toolSelected(int index)
{
    if (index != -1) {
        d->addedButtons[index].button->click();
    }
}

void KoModeBox::slotContextMenuRequested(const QPoint &pos)
{
    QMenu menu;
    KSelectAction* textAction = new KSelectAction(i18n("Text"), &menu);
    connect(textAction, SIGNAL(triggered(int)), SLOT(switchIconMode(int)));
    menu.addAction(textAction);
    textAction->addAction(i18n("Icon and Text"));
    textAction->addAction(i18n("Icon only"));
    textAction->setCurrentItem(d->iconMode);

    KSelectAction* buttonPositionAction = new KSelectAction(i18n("Tabs side"), &menu);
    connect(buttonPositionAction, SIGNAL(triggered(int)), SLOT(switchTabsSide(int)));
    menu.addAction(buttonPositionAction);
    if (d->horizontalMode) {
        buttonPositionAction->addAction(i18n("Top side"));
        buttonPositionAction->addAction(i18n("Bottom side"));
        buttonPositionAction->setCurrentItem(d->verticalTabsSide);
    } else {
        buttonPositionAction->addAction(i18n("Left side"));
        buttonPositionAction->addAction(i18n("Right side"));
        buttonPositionAction->setCurrentItem(d->horizontalTabsSide);
    }

    menu.exec(d->tabBar->mapToGlobal(pos));
}

void KoModeBox::switchIconMode(int mode)
{
    d->iconMode = static_cast<IconMode>(mode);
    if (d->iconMode == IconAndText) {
        if (d->horizontalMode) {
            d->tabBar->setIconSize(QSize(38,32));
        } else {
            d->tabBar->setIconSize(QSize(32,64));
        }
    } else {
        d->tabBar->setIconSize(QSize(22,22));
    }
    updateShownTools(QList<QString>());

    KConfigGroup cfg = KGlobal::config()->group("calligra");
    cfg.writeEntry("ModeBoxIconMode", (int)d->iconMode);

}

void KoModeBox::switchTabsSide(int side)
{
    if (d->horizontalMode) {
        d->verticalTabsSide = static_cast<VerticalTabsSide>(side);
        if (d->verticalTabsSide == TopSide) {
            d->layout->removeWidget(d->tabBar);
            d->tabBar->setShape(QTabBar::RoundedNorth);
            d->layout->addWidget(d->tabBar, 0, 0);
        } else {
            d->layout->removeWidget(d->tabBar);
            d->tabBar->setShape(QTabBar::RoundedSouth);
            d->layout->addWidget(d->tabBar, 2, 0);
        }

        KConfigGroup cfg = KGlobal::config()->group("calligra");
        cfg.writeEntry("ModeBoxVerticalTabsSide", (int)d->verticalTabsSide);
    } else {
        d->horizontalTabsSide = static_cast<HorizontalTabsSide>(side);
        if (d->horizontalTabsSide == LeftSide) {
            d->layout->removeWidget(d->tabBar);
            d->tabBar->setShape(QTabBar::RoundedWest);
            d->layout->addWidget(d->tabBar, 0, 0);
        } else {
            d->layout->removeWidget(d->tabBar);
            d->tabBar->setShape(QTabBar::RoundedEast);
            d->layout->addWidget(d->tabBar, 0, 2);
        }

        KConfigGroup cfg = KGlobal::config()->group("calligra");
        cfg.writeEntry("ModeBoxHorizontalTabsSide", (int)d->horizontalTabsSide);
    }
    updateShownTools(QList<QString>());
}
