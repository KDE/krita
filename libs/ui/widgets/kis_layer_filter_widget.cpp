/*
 *  Copyright (c) 2020 Eoin O'Neill <eoinoneill1991@gmail.com>
 *  Copyright (c) 2020 Emmet O'Neill <emmetoneill.pdx@gmail.com>
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
#include "kis_layer_filter_widget.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QCompleter>
#include <QEvent>
#include <QMouseEvent>
#include <QButtonGroup>
#include <QPushButton>
#include <QMenu>
#include <QScreen>

#include "kis_debug.h"
#include "kis_node.h"
#include "kis_global.h"
#include "kis_color_label_button.h"
#include "kis_color_label_selector_widget.h"
#include "kis_node_view_color_scheme.h"

KisLayerFilterWidget::KisLayerFilterWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    textFilter = new QLineEdit(this);
    textFilter->setPlaceholderText(i18n("Filter by name..."));
    textFilter->setMinimumWidth(192);
    textFilter->setMinimumHeight(32);
    textFilter->setClearButtonEnabled(true);

    connect(textFilter, SIGNAL(textChanged(QString)), this, SIGNAL(filteringOptionsChanged()));

    KisNodeViewColorScheme colorScheme;

    QWidget* buttonContainer = new QWidget(this);
    buttonContainer->setToolTip(i18n("Filter by color label..."));
    buttonEventFilter = new EventFilter(buttonContainer);
    {
        QHBoxLayout *subLayout = new QHBoxLayout(buttonContainer);
        buttonContainer->setLayout(subLayout);
        subLayout->setContentsMargins(0,0,0,0);
        subLayout->setAlignment(Qt::AlignLeft);
        buttonGroup = new KisColorLabelFilterGroup(buttonContainer);
        buttonGroup->setExclusive(false);
        QVector<QColor> colors = colorScheme.allColorLabels();

        for (int id = 0; id < colors.count(); id++) {
            KisColorLabelButton* btn = new KisColorLabelButton(colors[id], 32, buttonContainer);
            buttonGroup->addButton(btn, id);
            btn->installEventFilter(buttonEventFilter);
            subLayout->addWidget(btn);
        }

        connect(buttonGroup, SIGNAL(buttonToggled(int,bool)), this, SIGNAL(filteringOptionsChanged()));
    }

    resetButton = new QPushButton(i18n("Reset Filters"), this);
    resetButton->setMinimumHeight(32);
    connect(resetButton, &QPushButton::clicked, [this](){
       this->reset();
    });


    layout->addWidget(textFilter);
    layout->addWidget(buttonContainer);
    layout->addWidget(resetButton);
}

void KisLayerFilterWidget::scanUsedColorLabels(KisNodeSP node, QSet<int> &colorLabels)
{
    if (node->parent()) {
        colorLabels.insert(node->colorLabelIndex());
    }

    KisNodeSP child = node->firstChild();
    while(child) {
        scanUsedColorLabels(child, colorLabels);
        child = child->nextSibling();
    }
}

void KisLayerFilterWidget::updateColorLabels(KisNodeSP root)
{
    QSet<int> colorLabels;

    scanUsedColorLabels(root, colorLabels);
    buttonGroup->setViableLabels(colorLabels);
}

bool KisLayerFilterWidget::isCurrentlyFiltering() const
{
    const bool isFilteringText = !textFilter->text().isEmpty();
    const bool isFilteringColors = buttonGroup->getActiveLabels().count() > 0;

    return isFilteringText || isFilteringColors;
}

QSet<int> KisLayerFilterWidget::getActiveColors() const
{
    QSet<int> activeColors = buttonGroup->getActiveLabels();

    return activeColors;
}

QString KisLayerFilterWidget::getTextFilter() const
{
    return textFilter->text();
}

int KisLayerFilterWidget::getDesiredMinimumWidth() const {
    return qMax(textFilter->minimumWidth(), buttonGroup->countViableButtons() * 32);
}

int KisLayerFilterWidget::getDesiredMinimumHeight() const {
    QList<QAbstractButton*> viableButtons = buttonGroup->viableButtons();
    if (viableButtons.count() > 1) {
        return viableButtons[0]->sizeHint().height() + textFilter->minimumHeight() + resetButton->minimumHeight();
    } else {
        return textFilter->minimumHeight() + resetButton->minimumHeight();
    }
}

void KisLayerFilterWidget::reset()
{
    textFilter->clear();
    buttonGroup->reset();
    filteringOptionsChanged();
}

QSize KisLayerFilterWidget::sizeHint() const
{
    return QSize(getDesiredMinimumWidth(), getDesiredMinimumHeight());
}

void KisLayerFilterWidget::showEvent(QShowEvent *show)
{
    QMenu *parentMenu = dynamic_cast<QMenu*>(parentWidget());

    if (parentMenu) {
        const int widthBefore = parentMenu->width();
        const int rightEdgeThreshold = 5;

        //Fake resize event needs to be made to register change in widget menu size.
        //Not doing this will cause QMenu to not resize properly!
        resize(sizeHint());
        adjustSize();
        QResizeEvent event = QResizeEvent(sizeHint(), parentMenu->size());

        parentMenu->resize(sizeHint());
        parentMenu->adjustSize();
        qApp->sendEvent(parentMenu, &event);

        QScreen *screen = QGuiApplication::screenAt(parentMenu->mapToGlobal(parentMenu->pos()));
        QRect screenGeometry = screen ? screen->geometry() : parentMenu->parentWidget()->window()->geometry();

        const bool onRightEdge = (parentMenu->pos().x() + widthBefore + rightEdgeThreshold) >  screenGeometry.width();
        const int widthAfter = parentMenu->width();

        if (onRightEdge) {
            if (widthAfter > widthBefore) {
                const QRect newGeo = kisEnsureInRect( parentMenu->geometry(), screenGeometry );
                const int xShift = newGeo.x() - parentMenu->pos().x();
                parentMenu->move(parentMenu->pos().x() + xShift, parentMenu->pos().y() + 0);
            } else {
                const int xShift = widthBefore - widthAfter;
                parentMenu->move(parentMenu->pos().x() + xShift, parentMenu->pos().y() + 0);
            }
        }
    }
    QWidget::showEvent(show);
}

KisLayerFilterWidget::EventFilter::EventFilter(QObject* parent) : QObject(parent)
{
    lastKnownMousePosition = QPoint(0,0);
    currentState = Idle;
}

bool KisLayerFilterWidget::EventFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

        currentState = WaitingForDragLeave;
        lastKnownMousePosition = mouseEvent->globalPos();

        return true;

    } else if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        QAbstractButton* startingButton = static_cast<QAbstractButton*>(obj);

        //If we never left, toggle the original button.
        if( currentState == WaitingForDragLeave ) {
            if ( startingButton->group() && (mouseEvent->modifiers() & Qt::SHIFT)) {
                KisColorLabelFilterGroup* const group = static_cast<KisColorLabelFilterGroup*>(startingButton->group());
                const QList<QAbstractButton*> viableCheckedButtons = group->checkedViableButtons();

                const int buttonsEnabled = viableCheckedButtons.count();
                const bool shouldChangeIsolation = (buttonsEnabled == 1) && (viableCheckedButtons.first() == startingButton);
                const bool shouldIsolate = (buttonsEnabled != 1) || !shouldChangeIsolation;

                Q_FOREACH(QAbstractButton* otherBtn, group->viableButtons()) {
                    if (otherBtn == startingButton){
                        startingButton->setChecked(true);
                    } else {
                        otherBtn->setChecked(!shouldIsolate);
                    }
                }

            } else {
                startingButton->click();
            }
        }

        currentState = Idle;
        lastKnownMousePosition = mouseEvent->globalPos();

        return true;

    } else if (event->type() == QEvent::MouseMove) {

        if (currentState == WaitingForDragLeave) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QWidget* firstClicked = static_cast<QWidget*>(obj);
            const QPointF localPosition = mouseEvent->localPos();

            if (!firstClicked->rect().contains(localPosition.x(), localPosition.y())) {
                QAbstractButton* btn = static_cast<QAbstractButton*>(obj);
                btn->click();

                checkSlideOverNeighborButtons(mouseEvent, btn);

                currentState = WaitingForDragEnter;
            }

            lastKnownMousePosition = mouseEvent->globalPos();

            return true;

        } else if (currentState == WaitingForDragEnter) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QAbstractButton* startingButton = static_cast<QAbstractButton*>(obj);
            const QPoint currentPosition = mouseEvent->globalPos();

            checkSlideOverNeighborButtons(mouseEvent, startingButton);

            lastKnownMousePosition = currentPosition;

            return true;
        }

    }

    return false;
}

void KisLayerFilterWidget::EventFilter::checkSlideOverNeighborButtons(QMouseEvent* mouseEvent, QAbstractButton* startingButton)
{
    const QPoint currentPosition = mouseEvent->globalPos();

    if (startingButton->group()) {
        QList<QAbstractButton*> allButtons = startingButton->group()->buttons();

        Q_FOREACH(QAbstractButton* button, allButtons) {
            const QRect bounds = QRect(button->mapToGlobal(QPoint(0,0)), button->size());
            const QPoint upperLeft = QPoint(qMin(lastKnownMousePosition.x(), currentPosition.x()), qMin(lastKnownMousePosition.y(), currentPosition.y()));
            const QPoint lowerRight = QPoint(qMax(lastKnownMousePosition.x(), currentPosition.x()), qMax(lastKnownMousePosition.y(), currentPosition.y()));
            const QRect mouseMovement = QRect(upperLeft, lowerRight);
            if( bounds.intersects(mouseMovement) && !bounds.contains(lastKnownMousePosition)) {
                button->click();
            }
        }
    }
}
