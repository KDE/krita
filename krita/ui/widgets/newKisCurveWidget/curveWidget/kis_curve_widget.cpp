#include "kis_curve_widget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QButtonGroup>

#include "kis_cubic_curve_widget.h"
#include "kis_linear_curve_widget.h"
#include "kis_freehand_curve_widget.h"



KisCurveWidget::KisCurveWidget(QWidget *parent)
    : QWidget(parent),
      m_functionLikeWidget(new KisCubicCurveWidget),
      m_cubicWidget(new KisCubicCurveWidget),
      m_linearWidget(new KisLinearCurveWidget),
      m_freehandWidget(new KisFreehandCurveWidget)
{
    m_functionLikeWidget->hide();
    m_cubicWidget->hide();
    m_linearWidget->hide();
    m_freehandWidget->hide();

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(m_functionLikeWidget);
    layout->addWidget(m_cubicWidget);
    layout->addWidget(m_linearWidget);
    layout->addWidget(m_freehandWidget);

    QVBoxLayout* buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(0);
    buttonLayout->setMargin(0);

    QPushButton* functionlikeButton = new QPushButton("fun");
    QPushButton* cubicButton = new QPushButton("cub");
    QPushButton* linearButton = new QPushButton("lin");
    QPushButton* freehandButton = new QPushButton("fre");
    QPushButton* resetButton = new QPushButton("res");

    buttonLayout->addWidget(functionlikeButton);
    buttonLayout->addWidget(cubicButton);
    buttonLayout->addWidget(linearButton);
    buttonLayout->addWidget(freehandButton);

    QButtonGroup* buttonGroup = new QButtonGroup(this);

    for(int i=0; i<buttonLayout->count(); i++) {
        QPushButton* b = dynamic_cast<QPushButton*>(buttonLayout->itemAt(i)->widget());
        Q_ASSERT(b);

        b->setMaximumSize(30, 30);
        b->setMinimumSize(30, 30);
        b->setCheckable(true);
        buttonGroup->addButton(b);
    }

    buttonLayout->addStretch();
    buttonLayout->addWidget(resetButton);
    layout->addLayout(buttonLayout);

    resetButton->setMinimumSize(30, 30);
    resetButton->setMaximumSize(30, 30);

    //default is cubic curve
    m_currentCurve = m_cubicWidget;
    m_currentCurve->show();
    cubicButton->click();

    connect(functionlikeButton, SIGNAL(clicked()), SLOT(switchToFunction()));
    connect(cubicButton,        SIGNAL(clicked()), SLOT(switchToCubic()));
    connect(linearButton,       SIGNAL(clicked()), SLOT(switchToLinear()));
    connect(freehandButton,     SIGNAL(clicked()), SLOT(switchToFreehand()));
    connect(resetButton,        SIGNAL(clicked()), SLOT(reset()));
}

KisCurveWidget::~KisCurveWidget()
{}

void KisCurveWidget::reset()
{
    m_currentCurve->reset();
}

void KisCurveWidget::switchTo(KisCurveWidgetBase* newWidget)
{
    if(m_currentCurve == newWidget) return;

    newWidget->setControlPoints(m_currentCurve->controlPoints());
    m_currentCurve->hide();
    m_currentCurve = newWidget;
    m_currentCurve->show();

}

