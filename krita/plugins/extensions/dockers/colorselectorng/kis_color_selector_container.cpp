#include "kis_color_selector_container.h"

#include "kis_color_selector.h"
#include "kis_my_paint_shade_selector.h"
#include "kis_minimal_shade_selector.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <KIcon>

#include <KDebug>

KisColorSelectorContainer::KisColorSelectorContainer(QWidget *parent) :
    QWidget(parent),
    m_colorSelector(new KisColorSelector(this)),
    m_myPaintShadeSelector(new KisMyPaintShadeSelector(this)),
    m_minimalShadeSelector(new KisMinimalShadeSelector(this)),
    m_shadeSelector(m_myPaintShadeSelector),
    m_shadeSelectorHideable(false),
    m_allowHorizontalLayout(true)
{
    QPushButton* pipetteButton = new QPushButton(this);
    QPushButton* settingsButton = new QPushButton(this);

    pipetteButton->setIcon(KIcon("krita_tool_color_picker"));
    pipetteButton->setFlat(true);
    settingsButton->setIcon(KIcon("configure"));
    settingsButton->setFlat(true);
    pipetteButton->setMaximumWidth(24);
    settingsButton->setMaximumWidth(24);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(pipetteButton);
    buttonLayout->addWidget(settingsButton);
    buttonLayout->setMargin(0);
    buttonLayout->setSpacing(0);
    m_buttonLayout = buttonLayout;

    connect(settingsButton, SIGNAL(clicked()), this, SIGNAL(openSettings()));

    setNewLayout(Vertical);

    m_myPaintShadeSelector->hide();
    m_minimalShadeSelector->hide();
    setShadeSelectorType(MyPaintSelector);
}

void KisColorSelectorContainer::setShadeSelectorType(int type)
{
    if(m_shadeSelector!=0)
        m_shadeSelector->hide();

    switch(type) {
    case MyPaintSelector:
        m_shadeSelector = m_myPaintShadeSelector;
        break;
    case MinimalSelector:
        m_shadeSelector = m_minimalShadeSelector;
        break;
    default:
        m_shadeSelector = 0;
        break;
    }

    if(m_shadeSelector!=0) {
        m_shadeSelector->show();
//        setMinimumHeight(m_colorSelector->minimumHeight()+m_shadeSelector->minimumHeight()+30); //+30 for the buttons (temporarily)
    }
    else {
//        setMinimumHeight(m_colorSelector->minimumHeight()+30);
    }
}

void KisColorSelectorContainer::setShadeSelectorHideable(bool hideable)
{
    m_shadeSelectorHideable = hideable;
}

void KisColorSelectorContainer::setAllowHorizontalLayout(bool allow)
{
    m_allowHorizontalLayout = allow;
}

void KisColorSelectorContainer::resizeEvent(QResizeEvent * e)
{
    if(m_shadeSelector!=0) {
        int minimumHeightForBothWidgets = m_colorSelector->minimumHeight()+m_shadeSelector->minimumHeight()+30; //+30 for the buttons (temporarily)

        if(height()<minimumHeightForBothWidgets) {
            if(m_shadeSelectorHideable) {
                m_shadeSelector->hide();
            }
            else {
                m_shadeSelector->show();
            }

            if(m_allowHorizontalLayout) {
                setNewLayout(Horizontal);
            }

        }
        else {
            setNewLayout(Vertical);
        }
    }

    QWidget::resizeEvent(e);
}

void KisColorSelectorContainer::setNewLayout(Direction direction)
{
    if(layout()) {
        layout()->removeItem(m_buttonLayout);
        m_buttonLayout->setParent(0);
        delete layout();
    }

    QBoxLayout* l;
    if(direction==Horizontal)
        l = new QHBoxLayout(this);
    else
        l = new QVBoxLayout(this);

    l->setSpacing(0);
    l->setMargin(0);

    m_buttonLayout->setParent(0);
    l->addLayout(m_buttonLayout);
    l->addWidget(m_colorSelector);
    l->addWidget(m_myPaintShadeSelector);
    l->addWidget(m_minimalShadeSelector);
}
