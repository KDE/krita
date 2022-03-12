/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_color_label_selector_widget.h"

#include "kis_debug.h"
#include "kis_global.h"

#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QStyleOption>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QSpacerItem>

#include "kis_color_label_button.h"
#include "kis_node_view_color_scheme.h"

#include <kis_signals_blocker.h>
#include <KisWrappableHBoxLayout.h>

struct KisColorLabelSelectorWidget::Private
{
    QVector<QColor> colors;
    KisColorLabelFilterGroup *colorButtonGroup{nullptr};
    KisColorLabelMouseDragFilter *dragFilter{nullptr};
    int buttonSize{22};
};

KisColorLabelSelectorWidget::KisColorLabelSelectorWidget(QWidget *parent)
    : QWidget(parent)
    , m_d(new Private)
{
    KisNodeViewColorScheme scm;
    m_d->colors = scm.allColorLabels();

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setAlignment(Qt::AlignLeft);

    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(2);
    {
        m_d->colorButtonGroup = new KisColorLabelFilterGroup(this);
        m_d->colorButtonGroup->setExclusive(true);
        m_d->colorButtonGroup->setMinimumRequiredChecked(0);
        QSet<int> viableColors;

        for (int id = 0; id < m_d->colors.count(); id++) {
            KisColorLabelButton* btn = new KisColorLabelButton(m_d->colors[id], m_d->buttonSize, this);
            btn->setChecked(false);
            btn->setSelectionVisType(KisColorLabelButton::Outline);
            m_d->colorButtonGroup->addButton(btn, id);
            layout->addWidget(btn);
            viableColors << id;
        }

        m_d->colorButtonGroup->setViableLabels(viableColors);

        connect(m_d->colorButtonGroup, QOverload<QAbstractButton*, bool>::of(&QButtonGroup::buttonToggled),
            [this](QAbstractButton *button, bool state)
            {
                const int index = m_d->colorButtonGroup->id(button);
                emit buttonToggled(index, state);
                if (m_d->colorButtonGroup->exclusive()) {
                    if (!state) {
                        return;
                    }
                    emit currentIndexChanged(index);
                } else {
                    emit selectionChanged();
                }
            }
        );
    }
}

KisColorLabelSelectorWidget::~KisColorLabelSelectorWidget()
{}

bool KisColorLabelSelectorWidget::isButtonChecked(int index) const
{
    return m_d->colorButtonGroup->button(index)->isChecked();
}

void KisColorLabelSelectorWidget::setButtonChecked(int index, bool state)
{
    m_d->colorButtonGroup->button(index)->setChecked(state);
}

int KisColorLabelSelectorWidget::currentIndex() const
{
    if (!m_d->colorButtonGroup->exclusive()) {
        return -2;
    }
    return m_d->colorButtonGroup->checkedId();
}

void KisColorLabelSelectorWidget::setCurrentIndex(int index)
{
    if (!m_d->colorButtonGroup->exclusive()) {
        return;
    }
    if (index == -1) {
        QAbstractButton* btn = m_d->colorButtonGroup->checkedButton();
        if (btn) {
            btn->group()->setExclusive(false);
            btn->setChecked(false);
            btn->group()->setExclusive(true);
        }
    } else if (index != m_d->colorButtonGroup->checkedId()) {
        QAbstractButton* btn = m_d->colorButtonGroup->button(index);
        if (btn) {
            btn->setChecked(true);
        }
    }

    emit currentIndexChanged(index);
}

QList<int> KisColorLabelSelectorWidget::selection() const
{
    QList<int> indices;
    for (QAbstractButton *button : m_d->colorButtonGroup->buttons()) {
        if (button->isChecked()) {
            indices << m_d->colorButtonGroup->id(button);
        }
    }
    return indices;
}

void KisColorLabelSelectorWidget::setSelection(const QList<int> &indices)
{
    KisSignalsBlocker blocker(m_d->colorButtonGroup);
    for (QAbstractButton *button : m_d->colorButtonGroup->buttons()) {
        button->setChecked(false);
    }
    for (int index : indices) {
        m_d->colorButtonGroup->button(index)->setChecked(true);
    }
    emit selectionChanged();
}

bool KisColorLabelSelectorWidget::exclusive() const
{
    return m_d->colorButtonGroup->exclusive();
}

void KisColorLabelSelectorWidget::setExclusive(bool exclusive)
{
    m_d->colorButtonGroup->setExclusive(exclusive);
}

bool KisColorLabelSelectorWidget::buttonWrapEnabled() const
{
    return dynamic_cast<KisWrappableHBoxLayout*>(layout());
}

void KisColorLabelSelectorWidget::setButtonWrapEnabled(bool enabled)
{
    QLayout *newLayout;
    if (enabled) {
        if (dynamic_cast<KisWrappableHBoxLayout*>(layout())) {
            return;
        }
        newLayout = new KisWrappableHBoxLayout;
    } else {
        if (dynamic_cast<QHBoxLayout*>(layout())) {
            return;
        }
        newLayout = new QHBoxLayout;
    }
    newLayout->setContentsMargins(0,0,0,0);
    newLayout->setSpacing(2);
    for (QAbstractButton *button : m_d->colorButtonGroup->buttons()) {
        newLayout->addWidget(button);
    }
    delete layout();
    setLayout(newLayout);
}

bool KisColorLabelSelectorWidget::mouseDragEnabled() const
{
    return m_d->dragFilter;
}

void KisColorLabelSelectorWidget::setMouseDragEnabled(bool enabled)
{
    if (enabled) {
        if (m_d->dragFilter) {
            return;
        }
        m_d->dragFilter = new KisColorLabelMouseDragFilter(this);
        for (QAbstractButton *button : m_d->colorButtonGroup->buttons()) {
            button->installEventFilter(m_d->dragFilter);
        }
    } else {
        if (!m_d->dragFilter) {
            return;
        }
        for (QAbstractButton *button : m_d->colorButtonGroup->buttons()) {
            button->removeEventFilter(m_d->dragFilter);
        }
        delete m_d->dragFilter;
        m_d->dragFilter = nullptr;
    }
}

KisColorLabelSelectorWidget::SelectionIndicationType KisColorLabelSelectorWidget::selectionIndicationType() const
{
    return 
        static_cast<SelectionIndicationType>(
            qobject_cast<KisColorLabelButton*>(
                m_d->colorButtonGroup->button(0)
            )->selectionVisType()
        );
}

void KisColorLabelSelectorWidget::setSelectionIndicationType(SelectionIndicationType type)
{
    for (QAbstractButton *button : m_d->colorButtonGroup->buttons()) {
        qobject_cast<KisColorLabelButton*>(button)->setSelectionVisType(
            static_cast<KisColorLabelButton::SelectionIndicationType>(type)
        );
    }
}

int KisColorLabelSelectorWidget::buttonSize() const
{
    return m_d->buttonSize;
}

void KisColorLabelSelectorWidget::setButtonSize(int size)
{
    if (size == m_d->buttonSize) {
        return;
    }
    m_d->buttonSize = size;
    for (QAbstractButton *button : m_d->colorButtonGroup->buttons()) {
        qobject_cast<KisColorLabelButton*>(button)->setSize(static_cast<uint>(size));
    }
    update();
}


struct KisColorLabelSelectorWidgetMenuWrapper::Private
{
    KisColorLabelSelectorWidget *colorLabelSelector{nullptr};
    QSpacerItem* menuAlignmentOffset{nullptr};
};

KisColorLabelSelectorWidgetMenuWrapper::KisColorLabelSelectorWidgetMenuWrapper(QWidget *parent)
    : QWidget(parent)
    , m_d(new Private)
{
    m_d->colorLabelSelector = new KisColorLabelSelectorWidget(this);
    m_d->menuAlignmentOffset = new QSpacerItem(0, 0);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 1, 0, 1);
    layout->setSpacing(0);
    layout->addItem(m_d->menuAlignmentOffset);
    layout->addWidget(m_d->colorLabelSelector);
}

KisColorLabelSelectorWidgetMenuWrapper::~KisColorLabelSelectorWidgetMenuWrapper()
{}

KisColorLabelSelectorWidget* KisColorLabelSelectorWidgetMenuWrapper::colorLabelSelector() const
{
    return m_d->colorLabelSelector;
}

QSize KisColorLabelSelectorWidgetMenuWrapper::sizeHint() const
{
    return m_d->colorLabelSelector->sizeHint() + QSize(calculateMenuOffset(), 0);
}

void KisColorLabelSelectorWidgetMenuWrapper::resizeEvent(QResizeEvent *e)
{
    QMenu *menu = qobject_cast<QMenu*>(parent());

    if(menu) {
        int menuOffset = calculateMenuOffset();
        m_d->menuAlignmentOffset->changeSize(menuOffset, height());
        layout()->invalidate();
        menu->resize(menu->width() + menuOffset, menu->height());
    }

    QWidget::resizeEvent(e);
}

int KisColorLabelSelectorWidgetMenuWrapper::calculateMenuOffset() const
{
    bool hasWideItems = false;
    QMenu *menu = qobject_cast<QMenu*>(parent());
    bool hasCheckable = false;
    bool hasIcon = false;

    int menuOffset = 0;

    if (menu) {
        Q_FOREACH(QAction *action, menu->actions()) {
            hasCheckable |= action->isCheckable();
            hasIcon |= action->icon().isNull();
            hasWideItems |= (hasCheckable || hasIcon);

            if (hasWideItems) {
                break;
            }
        }
    }

    if (hasWideItems) {
        QStyleOption opt;
        opt.init(this);
        // some copy-pasted code from QFusionStyle style
        const int hMargin = style()->pixelMetric(QStyle::PM_MenuHMargin, &opt, this);
        const int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize, &opt, this);
        menuOffset = (hMargin + iconSize + 6);
    }

    return menuOffset;
}
