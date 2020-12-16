/*
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_layer_filter_widget.h"

#include <QDesktopWidget>
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
#include <QStylePainter>
#include <QGraphicsDropShadowEffect>

#include "kis_debug.h"
#include "kis_node.h"
#include "kis_global.h"
#include "kis_icon_utils.h"

#include "kis_color_filter_combo.h"
#include "kis_color_label_button.h"
#include "kis_color_label_selector_widget.h"
#include "kis_node_view_color_scheme.h"

#include "KisMouseClickEater.h"

KisLayerFilterWidget::KisLayerFilterWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    textFilter = new QLineEdit(this);
    textFilter->setPlaceholderText(i18n("Filter by name..."));
    textFilter->setMinimumWidth(255);
    textFilter->setMinimumHeight(28);
    textFilter->setClearButtonEnabled(true);

    connect(textFilter, SIGNAL(textChanged(QString)), this, SIGNAL(filteringOptionsChanged()));
    connect(textFilter, &QLineEdit::returnPressed, [this]() {
        QMenu* menu = dynamic_cast<QMenu*>(parentWidget());
        if (menu) {
            menu->close();
        }
    });

    KisNodeViewColorScheme colorScheme;

    QWidget* buttonContainer = new QWidget(this);
    MouseClickIgnore* mouseEater = new MouseClickIgnore(this);
    buttonContainer->setToolTip(i18n("Filter by color label..."));
    buttonContainer->installEventFilter(mouseEater);
    buttonEventFilter = new KisColorLabelMouseDragFilter(buttonContainer);
    {
        QHBoxLayout *subLayout = new QHBoxLayout(buttonContainer);
        subLayout->setContentsMargins(0,0,0,0);
        subLayout->setSpacing(0);
        subLayout->setAlignment(Qt::AlignLeft);
        buttonGroup = new KisColorLabelFilterGroup(buttonContainer);
        buttonGroup->setExclusive(false);
        QVector<QColor> colors = colorScheme.allColorLabels();

        for (int id = 0; id < colors.count(); id++) {
            KisColorLabelButton* btn = new KisColorLabelButton(colors[id], 28, buttonContainer);
            buttonGroup->addButton(btn, id);
            btn->installEventFilter(buttonEventFilter);
            subLayout->addWidget(btn);
        }

        connect(buttonGroup, SIGNAL(buttonToggled(int,bool)), this, SIGNAL(filteringOptionsChanged()));
    }

    resetButton = new QPushButton(i18n("Reset Filters"), this);
    resetButton->setMinimumHeight(28);
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
    const bool isFilteringText = hasTextFilter();
    const bool isFilteringColors = buttonGroup->getActiveLabels().count() > 0;

    return isFilteringText || isFilteringColors;
}

bool KisLayerFilterWidget::hasTextFilter() const
{
    return !textFilter->text().isEmpty();
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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
        QScreen *screen = QGuiApplication::screenAt(parentMenu->mapToGlobal(parentMenu->pos()));
        QRect screenGeometry = screen ? screen->geometry() : parentMenu->parentWidget()->window()->geometry();
#else
        QRect screenGeometry = QApplication::desktop()->screenGeometry(this);
#endif
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

KisLayerFilterWidgetToolButton::KisLayerFilterWidgetToolButton(QWidget *parent)
    : QToolButton(parent)
{
    m_textFilter = false;
    m_selectedColors = QList<int>();
}

KisLayerFilterWidgetToolButton::KisLayerFilterWidgetToolButton(const KisLayerFilterWidgetToolButton &rhs)
    : QToolButton(rhs.parentWidget())
    , m_textFilter(rhs.m_textFilter)
    , m_selectedColors(rhs.m_selectedColors)
{

}

void KisLayerFilterWidgetToolButton::setSelectedColors(QList<int> colors)
{
    m_selectedColors = colors;
}

void KisLayerFilterWidgetToolButton::setTextFilter(bool isTextFiltering)
{
    m_textFilter = isTextFiltering;
}

void KisLayerFilterWidgetToolButton::paintEvent(QPaintEvent *paintEvent)
{
    KisNodeViewColorScheme colorScheme;
    const bool validColorFilter = !(m_selectedColors.count() == 0 || m_selectedColors.count() == colorScheme.allColorLabels().count());

    if (m_textFilter == false && !validColorFilter)
    {
        QToolButton::paintEvent(paintEvent);
    }
    else
    {
        QStylePainter paint(this);
        QStyleOptionToolButton opt;
        initStyleOption(&opt);
        opt.icon = m_textFilter ? KisIconUtils::loadIcon("format-text-bold") : icon();
        paint.drawComplexControl(QStyle::CC_ToolButton, opt);
        const QSize halfIconSize = this->iconSize() / 2;
        const QSize halfButtonSize = this->size() / 2;
        const QRect editRect = kisGrowRect(QRect(QPoint(halfButtonSize.width() - halfIconSize.width(), halfButtonSize.height() - halfIconSize.height()),this->iconSize()), -1);
        const int size = qMin(editRect.width(), editRect.height());

        if( validColorFilter )
        {
            KisColorFilterCombo::paintColorPie(paint, opt.palette, m_selectedColors, editRect, size );
            if (m_textFilter) {
                if (!opt.icon.isNull()) {
                    QRadialGradient radGradient = QRadialGradient(editRect.center(), size);
                    QColor shadowTransparent = palette().shadow().color();
                    shadowTransparent.setAlpha(96);
                    radGradient.setColorAt(0.0f, shadowTransparent);
                    shadowTransparent.setAlpha(0);
                    radGradient.setColorAt(1.0f, shadowTransparent);
                    paint.setBrush(radGradient);
                    paint.setPen(Qt::NoPen);
                    paint.drawEllipse(editRect.center(), size, size);
                    opt.icon.paint(&paint, editRect);
                }
            }
        }
    }
}

MouseClickIgnore::MouseClickIgnore(QObject *parent)
    : QObject(parent)
{
}

bool MouseClickIgnore::eventFilter(QObject *obj, QEvent *event)
{
    if (obj &&
            (event->type() == QEvent::MouseButtonPress ||
             event->type() == QEvent::MouseButtonDblClick ||
             event->type() == QEvent::MouseButtonRelease)) {
        event->setAccepted(true);
        return true;
    } else {
        return false;
    }
}
