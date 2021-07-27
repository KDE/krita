/*
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2016 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QPainter>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPoint>
#include <QMenu>
#include <QAction>
#include <QDialog>

#include <KoColorSpace.h>
#include <resources/KoStopGradient.h>

#include "kis_debug.h"

#include <kis_icon_utils.h>

#include <KoCanvasResourcesIds.h>
#include <KoCanvasResourcesInterface.h>

#include "KisStopGradientEditor.h"

KisStopGradientEditor::KisStopGradientEditor(QWidget *parent)
    : QWidget(parent),
      m_gradient(0)
{
    setupUi(this);

    QAction *selectPreviousStopAction = new QAction(KisIconUtils::loadIcon("arrow-left"), i18nc("Button to select previous stop in the stop gradient editor", "Select previous stop"), this);
    selectPreviousStopAction->setToolTip(selectPreviousStopAction->text());
    connect(selectPreviousStopAction, SIGNAL(triggered()), gradientSlider, SLOT(selectPreviousStop()));

    QAction *selectNextStopAction = new QAction(KisIconUtils::loadIcon("arrow-right"), i18nc("Button to select next stop in the stop gradient editor", "Select next stop"), this);
    selectNextStopAction->setToolTip(selectNextStopAction->text());
    connect(selectNextStopAction, SIGNAL(triggered()), gradientSlider, SLOT(selectNextStop()));

    m_editStopAction = new QAction(KisIconUtils::loadIcon("document-edit"), i18nc("Button to edit the selected stop color in the stop gradient editor", "Edit stop"), this);
    m_editStopAction->setToolTip(m_editStopAction->text());
    connect(m_editStopAction, SIGNAL(triggered()), this, SLOT(editSelectedStop()));

    m_deleteStopAction = new QAction(KisIconUtils::loadIcon("edit-delete"), i18nc("Button to delete the selected stop in the stop gradient editor", "Delete stop"), this);
    m_deleteStopAction->setToolTip(m_deleteStopAction->text());
    connect(m_deleteStopAction, SIGNAL(triggered()), gradientSlider, SLOT(deleteSelectedStop()));

    QAction *flipStopsAction = new QAction(KisIconUtils::loadIcon("transform_icons_mirror_x"), i18nc("Button to flip the stops in the stop gradient editor", "Flip gradient"), this);
    flipStopsAction->setToolTip(flipStopsAction->text());
    connect(flipStopsAction, SIGNAL(triggered()), this, SLOT(reverse()));

    QAction *sortByValueAction = new QAction(KisIconUtils::loadIcon("sort-by-value"), i18nc("Button to sort the stops by value in the stop gradient editor", "Sort stops by value"), this);
    sortByValueAction->setToolTip(sortByValueAction->text());
    connect(sortByValueAction, &QAction::triggered, this, [this]{ this->sortByValue(SORT_ASCENDING); } );
    
    QAction *sortByHueAction = new QAction(KisIconUtils::loadIcon("sort-by-hue"), i18nc("Button to sort the stops by hue in the stop gradient editor", "Sort stops by hue"), this);
    sortByHueAction->setToolTip(sortByHueAction->text());
    connect(sortByHueAction, &QAction::triggered, this, [this]{ this->sortByHue(SORT_ASCENDING); } );

    QAction *distributeEvenlyAction = new QAction(KisIconUtils::loadIcon("distribute-horizontal"), i18nc("Button to evenly distribute the stops in the stop gradient editor", "Distribute stops evenly"), this);
    distributeEvenlyAction->setToolTip(distributeEvenlyAction->text());
    connect(distributeEvenlyAction, SIGNAL(triggered()), this, SLOT(distributeStopsEvenly()));

    selectPreviousStopButton->setAutoRaise(true);
    selectPreviousStopButton->setDefaultAction(selectPreviousStopAction);
    
    selectNextStopButton->setAutoRaise(true);
    selectNextStopButton->setDefaultAction(selectNextStopAction);

    deleteStopButton->setAutoRaise(true);
    deleteStopButton->setDefaultAction(m_deleteStopAction);

    flipStopsButton->setAutoRaise(true);
    flipStopsButton->setDefaultAction(flipStopsAction);

    sortByValueButton->setAutoRaise(true);
    sortByValueButton->setDefaultAction(sortByValueAction);

    sortByHueButton->setAutoRaise(true);
    sortByHueButton->setDefaultAction(sortByHueAction);

    distributeEvenlyButton->setAutoRaise(true);
    distributeEvenlyButton->setDefaultAction(distributeEvenlyAction);

    compactModeSelectPreviousStopButton->setAutoRaise(true);
    compactModeSelectPreviousStopButton->setDefaultAction(selectPreviousStopAction);
    
    compactModeSelectNextStopButton->setAutoRaise(true);
    compactModeSelectNextStopButton->setDefaultAction(selectNextStopAction);
    
    compactModeMiscOptionsButton->setPopupMode(QToolButton::InstantPopup);
    compactModeMiscOptionsButton->setAutoRaise(true);
    compactModeMiscOptionsButton->setIcon(KisIconUtils::loadIcon("view-choose"));
    compactModeMiscOptionsButton->setStyleSheet("QToolButton::menu-indicator { image: none; }");
    QAction *separator = new QAction;
    separator->setSeparator(true);
    compactModeMiscOptionsButton->addAction(m_editStopAction);
    compactModeMiscOptionsButton->addAction(m_deleteStopAction);
    compactModeMiscOptionsButton->addAction(separator);
    compactModeMiscOptionsButton->addAction(flipStopsAction);
    compactModeMiscOptionsButton->addAction(sortByValueAction);
    compactModeMiscOptionsButton->addAction(sortByHueAction);
    compactModeMiscOptionsButton->addAction(distributeEvenlyAction);

    stopEditor->setUseTransParentCheckBox(false);

    connect(gradientSlider, SIGNAL(sigSelectedStop(int)), this, SLOT(stopChanged(int)));
    connect(nameedit, SIGNAL(editingFinished()), this, SLOT(nameChanged()));
    connect(stopEditor, SIGNAL(colorChanged(KoColor)), SLOT(colorChanged(KoColor)));
    connect(stopEditor, SIGNAL(colorTypeChanged(KisGradientWidgetsUtils::ColorType)), this, SLOT(stopTypeChanged(KisGradientWidgetsUtils::ColorType)));
    connect(stopEditor, SIGNAL(opacityChanged(qreal)), this, SLOT(opacityChanged(qreal)));
    connect(stopEditor, SIGNAL(positionChanged(qreal)), this, SLOT(positionChanged(qreal)));

    setCompactMode(false);

    setGradient(0);
    stopChanged(-1);
}

KisStopGradientEditor::KisStopGradientEditor(KoStopGradientSP gradient, QWidget *parent, const char* name, const QString& caption,
      KoCanvasResourcesInterfaceSP canvasResourcesInterface)
    : KisStopGradientEditor(parent)
{
    m_canvasResourcesInterface = canvasResourcesInterface;
    setObjectName(name);
    setWindowTitle(caption);
    setGradient(gradient);
}

void KisStopGradientEditor::setCompactMode(bool value)
{
    lblName->setVisible(!value);
    nameedit->setVisible(!value);
    buttonsContainer->setVisible(!value);
    stopEditorContainer->setVisible(!value);
    compactModeButtonsContainer->setVisible(value);
}

void KisStopGradientEditor::setGradient(KoStopGradientSP gradient)
{
    m_gradient = gradient;
    setEnabled(m_gradient);

    if (m_gradient) {
        nameedit->setText(gradient->name());
        gradientSlider->setGradientResource(m_gradient);
        // stopChanged(gradientSlider->selectedStop());
    }

    emit sigGradientChanged();
}

void KisStopGradientEditor::setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    m_canvasResourcesInterface = canvasResourcesInterface;
}

KoCanvasResourcesInterfaceSP KisStopGradientEditor::canvasResourcesInterface() const
{
    return m_canvasResourcesInterface;
}

void KisStopGradientEditor::notifyGlobalColorChanged(const KoColor &color)
{
    if (stopEditor->colorType() == KisGradientWidgetsUtils::Custom) {

        stopEditor->setColor(color);
    }
}

boost::optional<KoColor> KisStopGradientEditor::currentActiveStopColor() const
{
    if (stopEditor->colorType() != KisGradientWidgetsUtils::Custom) return boost::none;
    return stopEditor->color();
}

void KisStopGradientEditor::stopChanged(int stop)
{
    if (!m_gradient) return;

    const bool hasStopSelected = stop >= 0;

    m_editStopAction->setEnabled(hasStopSelected);
    m_deleteStopAction->setEnabled(hasStopSelected && m_gradient->stops().size() > 2);
    stopEditorContainer->setCurrentIndex(hasStopSelected ? 0 : 1);

    if (hasStopSelected) {
        selectedStopLabel->setText(i18nc("Text that indicates the selected stop in the stop gradient editor", "Stop #%1", stop + 1));

        KoGradientStop gradientStop = m_gradient->stops()[stop];
        stopEditor->setPosition(gradientStop.position * 100.0);

        KoColor color;
        qreal opacity;
        KoGradientStopType type = gradientStop.type;

        if (type == FOREGROUNDSTOP) {
            stopEditor->setColorType(KisGradientWidgetsUtils::Foreground);
            if (m_canvasResourcesInterface) {
                color = m_canvasResourcesInterface->resource(KoCanvasResource::ForegroundColor).value<KoColor>();
            } else {
                color = gradientStop.color;
            }
            opacity = 100.0;
        }
        else if (type == BACKGROUNDSTOP) {
            stopEditor->setColorType(KisGradientWidgetsUtils::Background);
            if (m_canvasResourcesInterface) {
                color = m_canvasResourcesInterface->resource(KoCanvasResource::BackgroundColor).value<KoColor>();
            } else {
                color = gradientStop.color;
            }
            opacity = 100.0;
        }
        else {
            stopEditor->setColorType(KisGradientWidgetsUtils::Custom);
            color = gradientStop.color;
            opacity = color.opacityF() * 100.0;
        }

        stopEditor->setColor(color);
        stopEditor->setOpacity(opacity);

    } else {
        selectedStopLabel->setText(i18nc("Text that indicates no stop is selected in the stop gradient editor", "No stop selected"));
    }

    emit sigGradientChanged();
}

void KisStopGradientEditor::stopTypeChanged(KisGradientWidgetsUtils::ColorType type) {
    QList<KoGradientStop> stops = m_gradient->stops();
    int currentStop = gradientSlider->selectedStop();
    KoGradientStop stop = stops[currentStop];
 
    if (type == KisGradientWidgetsUtils::Foreground) {
        stop.type = FOREGROUNDSTOP;
        if (m_canvasResourcesInterface) {
            stop.color = m_canvasResourcesInterface->resource(KoCanvasResource::ForegroundColor).value<KoColor>();
        }
    } else if (type == KisGradientWidgetsUtils::Background) {
        stop.type = BACKGROUNDSTOP;
        if (m_canvasResourcesInterface) {
            stop.color = m_canvasResourcesInterface->resource(KoCanvasResource::BackgroundColor).value<KoColor>();
        }
    } else {
        stop.type = COLORSTOP;
    }

    stop.color.setOpacity(1.0);

    stops.removeAt(currentStop);
    stops.insert(currentStop, stop);
    m_gradient->setStops(stops);
    stopEditor->setColor(stop.color);
    stopEditor->setOpacity(100.0);
    emit gradientSlider->updateRequested(); //setSelectedStopType(type);
    emit sigGradientChanged();
}

void KisStopGradientEditor::colorChanged(const KoColor& color)
{
    if (!m_gradient) return;

    QList<KoGradientStop> stops = m_gradient->stops();
    int currentStop = gradientSlider->selectedStop();
    KoGradientStop stop = stops[currentStop];
        
    KoColor c(color);
    c.setOpacity(stop.color.opacityU8());
    stop.color = c;
    
    stops.removeAt(currentStop);
    stops.insert(currentStop, stop);
    m_gradient->setStops(stops);

    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
}

void KisStopGradientEditor::opacityChanged(qreal value)
{
    if (!m_gradient) return;

    QList<KoGradientStop> stops = m_gradient->stops();
    int currentStop = gradientSlider->selectedStop();
    KoGradientStop stop = stops[currentStop];
    
    stop.color.setOpacity(value / 100.0);

    stops.removeAt(currentStop);
    stops.insert(currentStop, stop);
    m_gradient->setStops(stops);

    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
}

void KisStopGradientEditor::positionChanged(qreal value)
{
    if (!m_gradient) return;

    QList<KoGradientStop> stops = m_gradient->stops();
    int currentStop = gradientSlider->selectedStop();
    KoGradientStop stop = stops[currentStop];
    stop.position = value / 100.0;
    
    stops.removeAt(currentStop);
    {
        currentStop = 0;
        for (int i = 0; i < stops.size(); i++) {
            if (stop.position <= stops[i].position) break;

            currentStop = i + 1;
        }
    }
    stops.insert(currentStop, stop);
    m_gradient->setStops(stops);
    gradientSlider->setSelectedStop(currentStop);

    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
}

void KisStopGradientEditor::nameChanged()
{
    if (!m_gradient) return;

    m_gradient->setName(nameedit->text());
    m_gradient->setFilename(nameedit->text() + m_gradient->defaultFileExtension());
    emit sigGradientChanged();
}

void KisStopGradientEditor::reverse()
{
    if (!m_gradient) return;

    QList<KoGradientStop> stops = m_gradient->stops();
    QList<KoGradientStop> reversedStops;
    for(const KoGradientStop& stop : stops) {
        reversedStops.push_front(KoGradientStop(1 - stop.position, stop.color, stop.type));
    }
    m_gradient->setStops(reversedStops);
    if (gradientSlider->selectedStop() >= 0) {
        gradientSlider->setSelectedStop(stops.size() - 1 - gradientSlider->selectedStop());
    } else {
        emit gradientSlider->updateRequested();
    }

    emit sigGradientChanged();
}

void KisStopGradientEditor::distributeStopsEvenly()
{
    if (!m_gradient) return;

    QList<KoGradientStop> stops = m_gradient->stops();
    qreal spacing = 1.0 / static_cast<qreal>(stops.size() - 1);
    for (int i = 0; i < stops.size(); ++i) {
        stops[i].position = qBound(0.0, static_cast<qreal>(i) * spacing, 1.0);
    }
    m_gradient->setStops(stops);
    if (gradientSlider->selectedStop() >= 0) {
        stopEditor->setPosition(stops[gradientSlider->selectedStop()].position * 100.0);
    }
    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
}

void KisStopGradientEditor::sortByValue( SortFlags flags = SORT_ASCENDING )
{
    if (!m_gradient) return;

    bool ascending = (flags & SORT_ASCENDING) > 0;
    bool evenDistribution = (flags & EVEN_DISTRIBUTION) > 0;

    QList<KoGradientStop> stops = m_gradient->stops();
    const int stopCount = stops.size();

    QList<KoGradientStop> sortedStops;
    std::sort(stops.begin(), stops.end(), KoGradientStopValueSort());

    int stopIndex = 0;
    for (const KoGradientStop& stop : stops) {
        const float value = evenDistribution ? (float)stopIndex / (float)(stopCount - 1) : stop.color.toQColor().valueF();
        const float position = ascending ? value : 1.f - value;

        if (ascending) {
            sortedStops.push_back(KoGradientStop(position, stop.color, stop.type));
        } else {
            sortedStops.push_front(KoGradientStop(position, stop.color, stop.type));
        }

        stopIndex++;
    }

    m_gradient->setStops(sortedStops);
    gradientSlider->setSelectedStop(stopCount - 1);

    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
}

void KisStopGradientEditor::sortByHue( SortFlags flags = SORT_ASCENDING )
{
    if (!m_gradient) return;

    bool ascending = (flags & SORT_ASCENDING) > 0;
    bool evenDistribution = (flags & EVEN_DISTRIBUTION) > 0;

    QList<KoGradientStop> stops = m_gradient->stops();
    const int stopCount = stops.size();

    QList<KoGradientStop> sortedStops;
    std::sort(stops.begin(), stops.end(), KoGradientStopHueSort());

    int stopIndex = 0;
    for (const KoGradientStop& stop : stops) {
        const float value = evenDistribution ? (float)stopIndex / (float)(stopCount - 1) : qMax(0.0, stop.color.toQColor().hueF());
        const float position = ascending ? value : 1.f - value;

        if (ascending) {
            sortedStops.push_back(KoGradientStop(position, stop.color, stop.type));
        } else {
            sortedStops.push_front(KoGradientStop(position, stop.color, stop.type));
        }

        stopIndex++;
    }

    m_gradient->setStops(sortedStops);
    gradientSlider->setSelectedStop(stopCount - 1);

    emit gradientSlider->updateRequested();
    emit sigGradientChanged();
}

void KisStopGradientEditor::editSelectedStop()
{
    if (gradientSlider->selectedStop() < 0) {
        return;
    }

    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle(i18nc("Title for the gradient stop editor", "Edit Stop"));
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    QWidget *editor = stopEditorContainer->currentWidget();
    int index = stopEditorContainer->indexOf(editor);
    stopEditorContainer->removeWidget(editor);

    QVBoxLayout *dialogLayout = new QVBoxLayout;
    dialogLayout->setMargin(10);
    dialogLayout->addWidget(editor);

    dialog->setLayout(dialogLayout);
    editor->show();
    dialog->resize(0, 0);

    connect(dialog, &QDialog::finished, [this, editor, index](int)
                                        {
                                            stopEditorContainer->insertWidget(index, editor);
                                            stopEditorContainer->setCurrentIndex(index);
                                        });

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}
