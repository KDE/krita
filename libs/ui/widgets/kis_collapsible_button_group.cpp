#include "kis_collapsible_button_group.h"

#include <QToolButton>
#include <QHBoxLayout>
#include <QAction>

#include "kis_debug.h"

struct KisCollapsibleButtonGroup::Private {
    Private()
        : collapsedButton()
        , allButtons()
        , autoCollapse(true)
        , collapsed(false) {}

    QScopedPointer<QToolButton> collapsedButton;
    QList<QToolButton*> allButtons;
    bool autoCollapse;
    bool collapsed;

    QHBoxLayout* layout;
};

KisCollapsibleButtonGroup::KisCollapsibleButtonGroup(QWidget *parent)
    : QWidget(parent)
    , m_d(new Private())
{
    m_d->layout = new QHBoxLayout;
    m_d->layout->setSpacing(0);
    m_d->layout->setMargin(0);
    setLayout(m_d->layout);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

    m_d->collapsedButton.reset(new QToolButton(this));
    m_d->layout->addWidget(m_d->collapsedButton.data());

    connect(m_d->collapsedButton.data(), &QToolButton::triggered, [this](QAction* triggered) {
        m_d->collapsedButton->setDefaultAction(triggered);
    });
}

void KisCollapsibleButtonGroup::setAutoRaise(bool autoRaise)
{
    m_d->collapsedButton->setAutoRaise(autoRaise);

    Q_FOREACH (QToolButton* button, m_d->allButtons) {
        button->setAutoRaise(autoRaise);
    }
}

bool KisCollapsibleButtonGroup::autoRaise() const
{
    return m_d->collapsedButton->autoRaise();
}

void KisCollapsibleButtonGroup::setIconSize(const QSize &size)
{
    m_d->collapsedButton->setIconSize(size);

    Q_FOREACH (QToolButton* button, m_d->allButtons) {
        button->setIconSize(size);
    }
}

QSize KisCollapsibleButtonGroup::iconSize() const
{
    return m_d->collapsedButton->iconSize();
}

void KisCollapsibleButtonGroup::setAutoCollapse(bool autoCollapse)
{
    m_d->autoCollapse = autoCollapse;
}

void KisCollapsibleButtonGroup::setCollapsed(bool collapse)
{
    m_d->collapsed = collapse;

    m_d->collapsedButton->setVisible(m_d->collapsed);

    Q_FOREACH (QToolButton* button, m_d->allButtons) {
        button->setVisible(!m_d->collapsed);
    }
}

bool KisCollapsibleButtonGroup::collapsed() const
{
    return m_d->collapsed;
}

QSize KisCollapsibleButtonGroup::sizeHint() const
{
    return m_d->collapsedButton->sizeHint() * m_d->allButtons.count();
}

QSize KisCollapsibleButtonGroup::minimumSizeHint() const
{
    return m_d->collapsedButton->size();
}

QToolButton* KisCollapsibleButtonGroup::addAction(QAction *action)
{
    QToolButton* button = new QToolButton(this);
    button->setDefaultAction(action);
    button->setIconSize(iconSize());

    m_d->allButtons.append(button);
    m_d->layout->insertWidget(1, button);
    m_d->collapsedButton->addAction(action);
    m_d->collapsedButton->setDefaultAction(action);

    button->setAutoRaise(autoRaise());
    button->setVisible(!m_d->collapsed);

    return button;
}

void KisCollapsibleButtonGroup::resizeEvent(QResizeEvent *event)
{
    if (m_d->autoCollapse) {
        if (size().width() < sizeHint().width()) {
            setCollapsed(true);
        } else {
            setCollapsed(false);
        }
    }
}
