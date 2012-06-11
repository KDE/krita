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

#include <KDebug>
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


class KoModeBox::Private
{
public:
    Private(KoCanvasController *c)
        : canvas(c->canvas())
        , canvasReset(false)
        , activeId(-1)
    {
    }

    KoCanvasBase *canvas;
    bool canvasReset;
    QList<KoToolButton> buttons; // buttons maintained by toolmanager
    QList<KoToolButton> addedButtons; //buttons in the order added to QToolBox
    QMap<int, QWidget *> addedWidgets;
    QSet<QWidget *> currentAuxWidgets;
    int activeId;
};

QString KoModeBox::applicationName;

static bool compareButton(const KoToolButton &b1, const KoToolButton &b2)
{
    if (b1.section == b2.section) {
        return b1.priority < b2.priority;
    } else {
        if (b1.section.contains(KoModeBox::applicationName)) {
            return true;
        } else if (b2.section.contains(KoModeBox::applicationName)) {
            return false;
        }

        if (b1.section == "main") {
            return true;
        } else if (b2.section == "main") {
            return false;
        }
        return b1.section < b2.section;
    }
}


KoModeBox::KoModeBox(KoCanvasControllerWidget *canvas, const QString &appName)
    : QToolBox()
    , d(new Private(canvas))
{
    applicationName = appName;

    foreach(const KoToolButton & button,
            KoToolManager::instance()->createToolList(canvas->canvas())) {
        addButton(button);
    }

    qSort(d->buttons.begin(), d->buttons.end(), compareButton);

    // Update visibility of buttons
    updateShownTools(canvas, QList<QString>());

    connect(this, SIGNAL(currentChanged(int)), this, SLOT(toolSelected(int)));

    connect(KoToolManager::instance(), SIGNAL(changedTool(KoCanvasController*, int)),
            this, SLOT(setActiveTool(KoCanvasController*, int)));
    connect(KoToolManager::instance(), SIGNAL(currentLayerChanged(const KoCanvasController*,const KoShapeLayer*)),
            this, SLOT(setCurrentLayer(const KoCanvasController*,const KoShapeLayer*)));
    connect(KoToolManager::instance(), SIGNAL(toolCodesSelected(const KoCanvasController*, QList<QString>)),
            this, SLOT(updateShownTools(const KoCanvasController*, QList<QString>)));
    connect(KoToolManager::instance(),
            SIGNAL(addedTool(const KoToolButton, KoCanvasController*)),
            this, SLOT(toolAdded(const KoToolButton, KoCanvasController*)));

    connect(canvas, SIGNAL(toolOptionWidgetsChanged(const QList<QWidget *> &)),
         this, SLOT(setOptionWidgets(const QList<QWidget *> &)));
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

void KoModeBox::setActiveTool(KoCanvasController *canvas, int id)
{
    if (canvas->canvas() == d->canvas) {
        d->activeId = id;
        blockSignals(true);
        int i = 0;
        foreach (const KoToolButton button, d->addedButtons) {
            if (button.buttonGroupId == d->activeId) {
                setCurrentIndex(i);
                break;
            }
            ++i;
        }
        blockSignals(false);
        return;
    }
}

void KoModeBox::addItem(const KoToolButton button)
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
    d->addedWidgets[button.buttonGroupId] = widget;

    QToolBox::addItem(widget, button.button->icon(), button.button->toolTip());
    d->addedButtons.append(button);
}

void KoModeBox::updateShownTools(const KoCanvasController *canvas, const QList<QString> &codes)
{
    if (canvas->canvas() != d->canvas) {
        return;
    }

    blockSignals(true);

    while (count()) {
        removeItem(0);
    }

    d->addedButtons.clear();

    int newIndex = -1;
    foreach (const KoToolButton button, d->buttons) {
        QString code = button.visibilityCode;
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

        if (code.startsWith(QLatin1String("flake/"))) {
            addItem(button);
            continue;
        }

        if (code.endsWith( QLatin1String( "/always"))) {
            addItem(button);
            continue;
        } else if (code.isEmpty() && codes.count() != 0) {
            addItem(button);
            continue;
        } else if (codes.contains(code)) {
            addItem(button);
            continue;
        }
    }
    if (newIndex != -1) {
        setCurrentIndex(newIndex);
    }
    blockSignals(false);
}

void KoModeBox::setOptionWidgets(const QList<QWidget *> &optionWidgetList)
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
    // need to unstretch row that have previously been stretched
    layout->setRowStretch(layout->rowCount()-1, 0);
    layout->setColumnMinimumWidth(0, 0); // used to be indent
    layout->setColumnStretch(1, 1);
    layout->setColumnStretch(2, 2);
    layout->setColumnStretch(3, 1);
    layout->setHorizontalSpacing(0);
    layout->setVerticalSpacing(2);
    int specialCount = 0;
    foreach(QWidget *widget, optionWidgetList) {
        if (widget->objectName().isEmpty()) {
            Q_ASSERT(!(widget->objectName().isEmpty()));
            continue; // skip this docker in release build when assert don't crash
        }
        if (!widget->windowTitle().isEmpty()) {
            QLabel *l;
            layout->addWidget(l = new QLabel(widget->windowTitle()), cnt++, 1, 1, 3, Qt::AlignHCenter);
            d->currentAuxWidgets.insert(l);
        }
        layout->addWidget(widget, cnt++, 1, 1, 3);
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
            layout->addWidget(s = new QFrame(), cnt++, 2, 1, 1);
            s->setFrameStyle(QFrame::HLine | QFrame::Sunken);
            d->currentAuxWidgets.insert(s);
        }
    }
    if (specialCount == optionWidgetList.count()) {
        layout->setRowStretch(cnt, 100);
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
        disconnect(ccwidget, SIGNAL(toolOptionWidgetsChanged(const QList<QWidget *> &)),
                    this, SLOT(setOptionWidgets(const QList<QWidget *> &)));
    }

    d->canvasReset = d->canvas != 0;

    d->canvas = canvas;

    ccwidget = dynamic_cast<KoCanvasControllerWidget *>(d->canvas->canvasController());
    connect(
        ccwidget, SIGNAL(toolOptionWidgetsChanged(const QList<QWidget *> &)),
         this, SLOT(setOptionWidgets(const QList<QWidget *> &)));
}

void KoModeBox::unsetCanvas()
{
    if (!d->canvasReset) {
        d->canvas = 0;
    }
}

void KoModeBox::toolAdded(const KoToolButton &button, KoCanvasController *canvas)
{
    if (canvas->canvas() == d->canvas) {
        addButton(button);

        qSort(d->buttons.begin(), d->buttons.end(), compareButton);

        updateShownTools(canvas, QList<QString>());
    }
}

void KoModeBox::toolSelected(int index)
{
    if (index != -1) {
        d->addedButtons[index].button->click();
    }
}
