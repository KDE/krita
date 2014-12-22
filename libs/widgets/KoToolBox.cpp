/*
 * Copyright (c) 2005-2009 Thomas Zander <zander@kde.org>
 * Copyright (c) 2009 Peter Simonsson <peter.simonsson@gmail.com>
 * Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "KoToolBox_p.h"
#include "KoToolBoxLayout_p.h"

#include <KoCanvasController.h>
#include <KoShapeLayer.h>
#include <QButtonGroup>
#include <QToolButton>
#include <QStyleOptionFrameV3>
#include <QPainter>
#include <QHash>
#include <QApplication>
#include <QTimer>

class KoToolBox::Private
{
public:
    Private()
        : layout(0)
        , buttonGroup(0)
        , floating(false)
    {
    }

    void addSection(Section *section, const QString &name);

    QMap<QString, Section*> sections;
    KoToolBoxLayout *layout;
    QButtonGroup *buttonGroup;
    QHash<QToolButton*, QString> visibilityCodes;
    bool floating;
};

void KoToolBox::Private::addSection(Section *section, const QString &name)
{
    section->setName(name);
    layout->addSection(section);
    sections.insert(name, section);
}

KoToolBox::KoToolBox()
    : d(new Private)
{
    d->layout = new KoToolBoxLayout(this);
    // add defaults
    d->addSection(new Section(this), "main");
    d->addSection(new Section(this), "dynamic");

    d->buttonGroup = new QButtonGroup(this);
    setLayout(d->layout);
    foreach(const KoToolButton & button, KoToolManager::instance()->createToolList()) {
        addButton(button.button, button.section, button.priority, button.buttonGroupId);
        d->visibilityCodes.insert(button.button, button.visibilityCode);
    }

    // Update visibility of buttons
    setButtonsVisible(QList<QString>());

    connect(KoToolManager::instance(), SIGNAL(changedTool(KoCanvasController*, int)),
            this, SLOT(setActiveTool(KoCanvasController*, int)));
    connect(KoToolManager::instance(), SIGNAL(currentLayerChanged(const KoCanvasController*,const KoShapeLayer*)),
            this, SLOT(setCurrentLayer(const KoCanvasController*,const KoShapeLayer*)));
    connect(KoToolManager::instance(), SIGNAL(toolCodesSelected(QList<QString>)), this, SLOT(setButtonsVisible(QList<QString>)));
    connect(KoToolManager::instance(),
            SIGNAL(addedTool(const KoToolButton, KoCanvasController*)),
            this, SLOT(toolAdded(const KoToolButton, KoCanvasController*)));

    QTimer::singleShot(0, this, SLOT(adjustToFit()));
}

KoToolBox::~KoToolBox()
{
    delete d;
}

void KoToolBox::addButton(QToolButton *button, const QString &section, int priority, int buttonGroupId)
{
    // ensure same L&F
    button->setCheckable(true);
    button->setAutoRaise(true);

    QString sectionToBeAddedTo;
    if (section.contains(qApp->applicationName())) {
        sectionToBeAddedTo = "main";
    } else if (section.contains("main")) {
        sectionToBeAddedTo = "main";
    }  else if (section.contains("dynamic")) {
        sectionToBeAddedTo = "dynamic";
    } else {
        sectionToBeAddedTo = section;
    }

    Section *sectionWidget = d->sections.value(sectionToBeAddedTo);
    if (sectionWidget == 0) {
        sectionWidget = new Section(this);
        d->addSection(sectionWidget, sectionToBeAddedTo);
    }
    sectionWidget->addButton(button, priority);

    if (buttonGroupId < 0)
        d->buttonGroup->addButton(button);
    else
        d->buttonGroup->addButton(button, buttonGroupId);
}

void KoToolBox::setActiveTool(KoCanvasController *canvas, int id)
{
    Q_UNUSED(canvas);
    QAbstractButton *button = d->buttonGroup->button(id);
    if (button) {
        button->setChecked(true);
    }
    else {
        kWarning(30004) << "KoToolBox::setActiveTool(" << id << "): no such button found";
    }
}

void KoToolBox::setButtonsVisible(const QList<QString> &codes)
{
    foreach(QToolButton *button, d->visibilityCodes.keys()) {
        QString code = d->visibilityCodes.value(button);

        if (code.startsWith(QLatin1String("flake/"))) {
            continue;
        }

        if (code.endsWith( QLatin1String( "/always"))) {
            button->setVisible(true);
            button->setEnabled( true );
        }
        else if (code.isEmpty()) {
            button->setVisible(true);
            button->setEnabled( codes.count() != 0 );
        }
        else {
            button->setVisible( codes.contains(code) );
        }
    }
    layout()->invalidate();
    update();
}

void KoToolBox::setCurrentLayer(const KoCanvasController *canvas, const KoShapeLayer *layer)
{
    Q_UNUSED(canvas);
    const bool enabled = layer == 0 || (layer->isEditable() && layer->isVisible());
    foreach (QToolButton *button, d->visibilityCodes.keys()) {
        if (d->visibilityCodes[button].endsWith( QLatin1String( "/always") ) ) {
            continue;
        }
        button->setEnabled(enabled);
    }
}

void KoToolBox::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.setBrush(palette().shadow());

    const QList<Section*> sections = d->sections.values();
    QList<Section*>::const_iterator iterator = sections.begin();
    int halfSpacing = layout()->spacing();
    if (halfSpacing > 0) {
        halfSpacing /= 2;
    }
    while(iterator != sections.end()) {
        Section *section = *iterator;
        QStyleOptionFrameV3 frameoption;
        frameoption.lineWidth = 1;
        frameoption.midLineWidth = 0;

        if (section->separators() & Section::SeparatorTop) {
            int y = section->y() - halfSpacing;
            frameoption.frameShape = QFrame::HLine;
            frameoption.rect = QRect(section->x(), y, section->width(), 2);
            style()->drawControl(QStyle::CE_ShapedFrame, &frameoption, &painter);
        }

        if (section->separators() & Section::SeparatorLeft) {
            int x = section->x() - halfSpacing;
            frameoption.frameShape = QFrame::VLine;
            frameoption.rect = QRect(x, section->y(), 2, section->height());
            style()->drawControl(QStyle::CE_ShapedFrame, &frameoption, &painter);
        }

        ++iterator;
    }

    painter.end();
}

void KoToolBox::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (!d->floating) {
        setMinimumSize(layout()->minimumSize()); // This enforces the minimum size on the widget
    }
}

void KoToolBox::setOrientation(Qt::Orientation orientation)
{
    d->layout->setOrientation(orientation);
    QTimer::singleShot(0, this, SLOT(update()));
    foreach(Section* section, d->sections) {
        section->setOrientation(orientation);
    }
}

void KoToolBox::setFloating(bool v)
{
    setMinimumSize(QSize(1,1));
    d->floating = v;
}

void KoToolBox::toolAdded(const KoToolButton &button, KoCanvasController *canvas)
{
    Q_UNUSED(canvas);
    addButton(button.button, button.section, button.priority, button.buttonGroupId);
    d->visibilityCodes.insert(button.button, button.visibilityCode);
    setButtonsVisible(QList<QString>());

}

void KoToolBox::adjustToFit()
{
    int newWidth = width() - (width() % layout()->minimumSize().width());
    if(newWidth != width() && newWidth >= layout()->minimumSize().width()) {
        setMaximumWidth(newWidth);
        QTimer::singleShot(0, this, SLOT(resizeUnlock()));
    }
}

void KoToolBox::resizeUnlock()
{
    setMaximumWidth(QWIDGETSIZE_MAX);
}
