/*
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_color_label_button.h"

#include <QStylePainter>
#include <QStyleOption>
#include <QMimeData>
#include <QMouseEvent>


#include "kis_global.h"
#include "kis_debug.h"
#include "krita_container_utils.h"

struct KisColorLabelButton::Private
{
    const QColor m_color;
    const uint m_sizeSquared;
    KisColorLabelButton::SelectionIndicationType selectionVis;

    Private(QColor color, uint sizeSquared)
        : m_color(color)
        , m_sizeSquared(sizeSquared)
        , selectionVis(KisColorLabelButton::FillIn)
    {

    }

    Private(const Private& rhs)
        : m_color(rhs.m_color)
        , m_sizeSquared(rhs.m_sizeSquared)
        , selectionVis(rhs.selectionVis)
    {

    }
};

KisColorLabelButton::KisColorLabelButton(QColor color, uint sizeSquared, QWidget *parent) : QAbstractButton(parent), m_d(new Private(color, sizeSquared))
{
    setCheckable(true);
    setChecked(true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

KisColorLabelButton::~KisColorLabelButton() {}

void KisColorLabelButton::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QStylePainter painter(this);
    QStyleOptionButton styleOption;
    styleOption.initFrom(this);

    const bool darkTheme = styleOption.palette.window().color().value() < 128;

    if (isDown() || isChecked()){
        styleOption.state |= QStyle::State_On;
    }

    QRect fillRect = kisGrowRect(rect(), -2);
    QRect outlineRect = kisGrowRect(fillRect, -1);

    const QColor shadowColor = styleOption.palette.window().color().darker(darkTheme ? 128 : 200);
    const QBrush shadowBrush = QBrush(shadowColor);
    const QBrush bgBrush = QBrush(styleOption.palette.window().color());

    if (!isEnabled()) {
        fillRect -= QMargins(sizeHint().width() / 4, sizeHint().height() / 4, sizeHint().width() / 4, sizeHint().height() / 4);
    } else {
        fillRect = kisGrowRect(fillRect, -3);
    }

    if (m_d->m_color.alpha() > 0) {
        QColor fillColor = m_d->m_color;

        if ((!isChecked() || !isEnabled()) && (m_d->selectionVis == FillIn)) {
            fillColor.setAlpha(32);
        } else if ((!isChecked() || !isEnabled()) && (m_d->selectionVis == Outline)) {
            if ((styleOption.state & QStyle::State_MouseOver) == 0) {
                fillColor.setAlpha(192);
            }
        }

        if ((isEnabled() && isChecked() && m_d->selectionVis == FillIn) ||
                m_d->selectionVis == Outline) {
            painter.fillRect(kisGrowRect(fillRect, 1), shadowBrush);
            painter.fillRect(fillRect, bgBrush);
        }

        QBrush brush = QBrush(fillColor);
        painter.fillRect(fillRect, brush);

        if ((isEnabled() && (m_d->selectionVis == FillIn)) ||
            (isChecked() && (m_d->selectionVis == Outline))) {
            const QRect& shadowRect = outlineRect;
            painter.setPen(QPen(shadowColor, 4));
            painter.drawRect(shadowRect);

            painter.setPen(QPen(bgBrush.color(), 2));
            painter.drawRect(outlineRect);

            painter.setPen(QPen(m_d->m_color, 2));
            painter.drawRect(outlineRect);
        }

    } else {
        QColor white = QColor(255,255,255);
        QColor grey = QColor(200,200,200);
        QColor xOverlayColor = QColor(100,100,100);
        QColor outlineColor = grey;

        if ((!isChecked() || !isEnabled()) && (m_d->selectionVis == FillIn)) {
            white.setAlpha(32);
            grey.setAlpha(32);

            if (darkTheme) {
                xOverlayColor = styleOption.palette.window().color().lighter(130);
            } else {
                xOverlayColor = xOverlayColor.lighter(190);
            }

        } else if ((!isChecked() || !isEnabled()) && (m_d->selectionVis == Outline)) {
            if ((styleOption.state & QStyle::State_MouseOver) == 0) {
                white.setAlpha(192);
                xOverlayColor.setAlpha(192);
                if (darkTheme) {
                    grey = grey.darker(110);
                } else {
                    grey = grey.lighter(110);
                }
                grey.setAlpha(192);
            }
        }

        QBrush whiteBrush = QBrush(white);
        QBrush greyBrush = QBrush(grey);

        QRect upperLeftGrey = fillRect - QMargins(0, 0, fillRect.size().width() / 2, fillRect.size().height() /2);
        QRect lowerRightGrey = fillRect - QMargins(fillRect.size().width() / 2, fillRect.size().height() / 2, 0, 0);
        QRect xOverlay = kisGrowRect(fillRect, (m_d->m_sizeSquared / 8) * -1);

        if (isEnabled() && ((isChecked() && m_d->selectionVis == FillIn) ||
                m_d->selectionVis == Outline)) {
            painter.fillRect(kisGrowRect(fillRect, 1), shadowBrush);
            painter.fillRect(fillRect, bgBrush);
        }

        painter.fillRect(fillRect, whiteBrush);

        painter.fillRect(upperLeftGrey, greyBrush);
        painter.fillRect(lowerRightGrey, greyBrush);

        painter.setPen(QPen(xOverlayColor, 2));
        painter.drawLine(xOverlay.topLeft(), xOverlay.bottomRight());
        painter.drawLine(xOverlay.bottomLeft(), xOverlay.topRight());

        if ((isEnabled() && (m_d->selectionVis == FillIn)) ||
            (isChecked() && (m_d->selectionVis == Outline))) {
            const QRect& shadowRect = outlineRect;
            painter.setPen(QPen(shadowColor, 4));
            painter.drawRect(shadowRect);

            painter.setPen(QPen(bgBrush.color(), 2));
            painter.drawRect(outlineRect);

            painter.setPen(QPen(outlineColor, 2));
            painter.drawRect(outlineRect);
        }

    }
}

void KisColorLabelButton::enterEvent(QEvent *event) {
    Q_UNUSED(event);
    update();
}

void KisColorLabelButton::leaveEvent(QEvent *event) {
    Q_UNUSED(event);
    update();
}

QSize KisColorLabelButton::sizeHint() const
{
    return QSize(m_d->m_sizeSquared,m_d->m_sizeSquared);
}

void KisColorLabelButton::setSelectionVisType(KisColorLabelButton::SelectionIndicationType type)
{
    m_d->selectionVis = type;
}

void KisColorLabelButton::nextCheckState()
{
    KisColorLabelFilterGroup* colorLabelFilterGroup = dynamic_cast<KisColorLabelFilterGroup*>(group());

    if (!colorLabelFilterGroup || (colorLabelFilterGroup->countCheckedViableButtons() > colorLabelFilterGroup->minimumRequiredChecked() || !isChecked())) {
        setChecked(!isChecked());
    } else {
        setChecked(isChecked());
    }
}

KisColorLabelFilterGroup::KisColorLabelFilterGroup(QObject *parent)
    : QButtonGroup(parent)
    , minimumCheckedButtons(1)
{
}

KisColorLabelFilterGroup::~KisColorLabelFilterGroup()
{
}

QList<QAbstractButton *> KisColorLabelFilterGroup::viableButtons() const {
    QList<QAbstractButton*> viableButtons;

    Q_FOREACH( int index, viableColorLabels ) {
        viableButtons.append(button(index));
    }

    return viableButtons;
}

void KisColorLabelFilterGroup::setViableLabels(const QSet<int> &labels) {
    setAllVisibility(false);
    disableAll();
    QSet<int> removed = viableColorLabels.subtract(labels);

    viableColorLabels = labels;

    if (viableColorLabels.count() > 1) {
        setAllVisibility(true);
        Q_FOREACH( int index, viableColorLabels) {
            if (button(index)) {
                button(index)->setEnabled(true);
            }
        }
    }

    Q_FOREACH( int index, removed ) {
        button(index)->setChecked(true);
    }
}

void KisColorLabelFilterGroup::setViableLabels(const QList<int> &viableLabels)
{
    QSet<int> uniqueViableLabels;
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
    uniqueViableLabels = QSet<int>(viableLabels.cbegin(), viableLabels.cend());
#else
    uniqueViableLabels = QSet<int>::fromList(viableLabels);
#endif
    setViableLabels(uniqueViableLabels);
}

QSet<int> KisColorLabelFilterGroup::getActiveLabels() const {
    QSet<int> checkedLabels = QSet<int>();

    Q_FOREACH( int index, viableColorLabels ) {
        if (button(index)->isChecked()) {
            checkedLabels.insert(index);
        }
    }

    return checkedLabels.count() == viableColorLabels.count() && minimumRequiredChecked() > 0 ? QSet<int>() : checkedLabels;
}

QList<QAbstractButton *> KisColorLabelFilterGroup::checkedViableButtons() const {
    QList<QAbstractButton*> checkedButtons = viableButtons();

    KritaUtils::filterContainer(checkedButtons, [](QAbstractButton* btn){
       return (btn->isChecked());
    });

    return checkedButtons;
}

int KisColorLabelFilterGroup::countCheckedViableButtons() const {
    return checkedViableButtons().count();
}

int KisColorLabelFilterGroup::countViableButtons() const {
    return viableColorLabels.count();
}

void KisColorLabelFilterGroup::setMinimumRequiredChecked(int checkedBtns)
{
    minimumCheckedButtons = checkedBtns;
}

int KisColorLabelFilterGroup::minimumRequiredChecked() const
{
    return minimumCheckedButtons;
}

void KisColorLabelFilterGroup::reset() {
    Q_FOREACH( QAbstractButton* btn, viableButtons() ) {
        btn->setChecked(true);
    }
}

void KisColorLabelFilterGroup::disableAll() {
    Q_FOREACH( QAbstractButton* btn, buttons() ) {
        btn->setDisabled(true);
    }
}

void KisColorLabelFilterGroup::setAllVisibility(const bool vis)
{
    Q_FOREACH( QAbstractButton* btn, buttons() ) {
        btn->setVisible(vis);
    }
}

KisColorLabelMouseDragFilter::KisColorLabelMouseDragFilter(QObject* parent) : QObject(parent)
{
    lastKnownMousePosition = QPoint(0,0);
    currentState = Idle;
}

bool KisColorLabelMouseDragFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick) {
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

void KisColorLabelMouseDragFilter::checkSlideOverNeighborButtons(QMouseEvent* mouseEvent, QAbstractButton* startingButton)
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
