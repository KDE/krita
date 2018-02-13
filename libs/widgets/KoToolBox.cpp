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
#include "KoToolBoxButton_p.h"

#include <QButtonGroup>
#include <QToolButton>
#include <QStyleOptionFrameV3>
#include <QPainter>
#include <QHash>
#include <QApplication>
#include <QStyle>
#include <QTimer>
#include <QMenu>
#include <QAction>

#include <klocalizedstring.h>
#include <WidgetsDebug.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <KoCanvasController.h>
#include <KoShapeLayer.h>

#define BUTTON_MARGIN 10

static int buttonSize(int screen)
{
    QRect rc = qApp->desktop()->screenGeometry(screen);
    if (rc.width() <= 1024) {
        return 12;
    }
    else if (rc.width() <= 1377) {
        return 14;
    }
    else  if (rc.width() <= 1920 ) {
        return 16;
    }
    else {
        return 22;
    }
}

class KoToolBox::Private
{
public:
    Private()
        : layout(0)
        , buttonGroup(0)
        , floating(false)
        , contextSize(0)
    {
    }

    void addSection(Section *section, const QString &name);

    QList<QToolButton*> buttons;
    QMap<QString, Section*> sections;
    KoToolBoxLayout *layout;
    QButtonGroup *buttonGroup;
    QHash<QToolButton*, QString> visibilityCodes;
    bool floating;
    QMap<QAction*,int> contextIconSizes;
    QMenu* contextSize;
    Qt::Orientation orientation;
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
    Q_FOREACH (KoToolAction *toolAction, KoToolManager::instance()->toolActionList()) {
        addButton(toolAction);
    }

    // Update visibility of buttons
    setButtonsVisible(QList<QString>());

    connect(KoToolManager::instance(), SIGNAL(changedTool(KoCanvasController*, int)),
            this, SLOT(setActiveTool(KoCanvasController*, int)));
    connect(KoToolManager::instance(), SIGNAL(currentLayerChanged(const KoCanvasController*,const KoShapeLayer*)),
            this, SLOT(setCurrentLayer(const KoCanvasController*,const KoShapeLayer*)));
    connect(KoToolManager::instance(), SIGNAL(toolCodesSelected(QList<QString>)), this, SLOT(setButtonsVisible(QList<QString>)));
    connect(KoToolManager::instance(),
            SIGNAL(addedTool(KoToolAction*,KoCanvasController*)),
            this, SLOT(toolAdded(KoToolAction*,KoCanvasController*)));

}

KoToolBox::~KoToolBox()
{
    delete d;
}

void KoToolBox::addButton(KoToolAction *toolAction)
{
    KoToolBoxButton *button = new KoToolBoxButton(toolAction, this);

    d->buttons << button;

    int toolbuttonSize = buttonSize(qApp->desktop()->screenNumber(this));
    KConfigGroup cfg =  KSharedConfig::openConfig()->group("KoToolBox");
    int iconSize = cfg.readEntry("iconSize", toolbuttonSize);

   button->setIconSize(QSize(iconSize, iconSize));
    foreach (Section *section, d->sections.values())  {
        section->setButtonSize(QSize(iconSize + BUTTON_MARGIN, iconSize + BUTTON_MARGIN));
    }

    QString sectionToBeAddedTo;
    const QString section = toolAction->section();
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
    sectionWidget->addButton(button, toolAction->priority());

    d->buttonGroup->addButton(button, toolAction->buttonGroupId());

    d->visibilityCodes.insert(button, toolAction->visibilityCode());
}

void KoToolBox::setActiveTool(KoCanvasController *canvas, int id)
{
    Q_UNUSED(canvas);

    QAbstractButton *button = d->buttonGroup->button(id);
    if (button) {
        button->setChecked(true);
        (qobject_cast<KoToolBoxButton*>(button))->setHighlightColor();
    }
    else {
        warnWidgets << "KoToolBox::setActiveTool(" << id << "): no such button found";
    }
}

void KoToolBox::setButtonsVisible(const QList<QString> &codes)
{
    Q_FOREACH (QToolButton *button, d->visibilityCodes.keys()) {
        QString code = d->visibilityCodes.value(button);

        if (code.startsWith(QLatin1String("flake/"))) {
            continue;
        }

        if (code.endsWith( QLatin1String( "/always"))) {
            button->setVisible(true);
            button->setEnabled(true);
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
    const bool enabled = layer == 0 || (layer->isShapeEditable() && layer->isVisible());
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

    const QList<Section*> sections = d->sections.values();
    QList<Section*>::const_iterator iterator = sections.begin();
    int halfSpacing = layout()->spacing();
    if (halfSpacing > 0) {
        halfSpacing /= 2;
    }
    while(iterator != sections.end()) {
        Section *section = *iterator;
        QStyleOption styleoption;
        styleoption.palette = palette();

        if (section->separators() & Section::SeparatorTop) {
            int y = section->y() - halfSpacing;
            styleoption.state = QStyle::State_None;
            styleoption.rect = QRect(section->x(), y-1, section->width(), 2);

            style()->drawPrimitive(QStyle::PE_IndicatorToolBarSeparator, &styleoption, &painter);
        }

        if (section->separators() & Section::SeparatorLeft) {
            int x = section->x() - halfSpacing;
            styleoption.state = QStyle::State_Horizontal;
            styleoption.rect = QRect(x-1, section->y(), 2, section->height());

            style()->drawPrimitive(QStyle::PE_IndicatorToolBarSeparator, &styleoption, &painter);
        }

        ++iterator;
    }

    painter.end();
}

void KoToolBox::setOrientation(Qt::Orientation orientation)
{
    d->orientation = orientation;
    d->layout->setOrientation(orientation);
    QTimer::singleShot(0, this, SLOT(update()));
    Q_FOREACH (Section* section, d->sections) {
        section->setOrientation(orientation);
    }
}

void KoToolBox::setFloating(bool v)
{
    d->floating = v;
}

void KoToolBox::toolAdded(KoToolAction *toolAction, KoCanvasController *canvas)
{
    Q_UNUSED(canvas);
    addButton(toolAction);
    setButtonsVisible(QList<QString>());

}

void KoToolBox::slotContextIconSize()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action && d->contextIconSizes.contains(action)) {
        const int iconSize = d->contextIconSizes.value(action);

        KConfigGroup cfg =  KSharedConfig::openConfig()->group("KoToolBox");
        cfg.writeEntry("iconSize", iconSize);

        Q_FOREACH (QToolButton *button, d->buttons) {
            button->setIconSize(QSize(iconSize, iconSize));
        }

        Q_FOREACH (Section *section, d->sections.values())  {
            section->setButtonSize(QSize(iconSize + BUTTON_MARGIN, iconSize + BUTTON_MARGIN));
        }

    }
}

void KoToolBox::contextMenuEvent(QContextMenuEvent *event)
{

    int toolbuttonSize = buttonSize(qApp->desktop()->screenNumber(this));

    if (!d->contextSize) {

        d->contextSize = new QMenu(i18n("Icon Size"), this);
        d->contextIconSizes.insert(d->contextSize->addAction(i18nc("@item:inmenu Icon size", "Default"),
                                                          this, SLOT(slotContextIconSize())),
                                   toolbuttonSize);

        QList<int> sizes;
        sizes << 12 << 14 << 16 << 22 << 32 << 48 << 64; //<< 96 << 128 << 192 << 256;
        Q_FOREACH (int i, sizes) {
            d->contextIconSizes.insert(d->contextSize->addAction(i18n("%1x%2", i, i), this, SLOT(slotContextIconSize())), i);
        }

        QActionGroup *sizeGroup = new QActionGroup(d->contextSize);
        foreach (QAction *action, d->contextSize->actions()) {
            action->setActionGroup(sizeGroup);
            action->setCheckable(true);
        }
    }
    KConfigGroup cfg =  KSharedConfig::openConfig()->group("KoToolBox");
    toolbuttonSize = cfg.readEntry("iconSize", toolbuttonSize);

    QMapIterator< QAction*, int > it = d->contextIconSizes;
    while (it.hasNext()) {
        it.next();
        if (it.value() == toolbuttonSize) {
            it.key()->setChecked(true);
            break;
        }
    }

    d->contextSize->exec(event->globalPos());
}
KoToolBoxLayout *KoToolBox::toolBoxLayout() const
{
    return d->layout;
}

#include "moc_KoToolBoxScrollArea_p.cpp"
