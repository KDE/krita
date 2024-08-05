/*
 *  SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_selection_options.h"

#include <QCheckBox>
#include <QToolButton>

#include <kis_icon.h>
#include "kis_types.h"
#include "kis_layer.h"
#include "kis_image.h"
#include "kis_selection.h"
#include "kis_paint_device.h"
#include "KisViewManager.h"
#include "kis_shape_controller.h"

#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include <KisOptionButtonStrip.h>
#include <KoGroupButton.h>
#include <kis_color_label_selector_widget.h>
#include <kis_slider_spin_box.h>

class KisSelectionOptions::Private
{
public:
    KisSelectionOptions *q;
    KisOptionButtonStrip *optionButtonStripMode{nullptr};
    KisOptionButtonStrip *optionButtonStripAction{nullptr};
    QCheckBox *checkBoxAntiAliasSelection{nullptr};
    KisSliderSpinBox *sliderGrowSelection{nullptr};
    QToolButton *buttonStopGrowingAtDarkestPixel {nullptr};
    KisSliderSpinBox *sliderFeatherSelection{nullptr};
    KisOptionButtonStrip *optionButtonStripReference{nullptr};
    KisColorLabelSelectorWidget *widgetLabels{nullptr};

    int modeToButtonIndex(SelectionMode mode) const
    {
        return mode == PIXEL_SELECTION ? 0 : 1;
    }

    SelectionMode buttonIndexToMode(int index) const
    {
        return index == 0 ? PIXEL_SELECTION : SHAPE_PROTECTION;
    }

    int actionToButtonIndex(SelectionAction action) const
    {
        switch (action) {
        case SELECTION_REPLACE:
            return 0;
        case SELECTION_INTERSECT:
            return 1;
        case SELECTION_ADD:
            return 2;
        case SELECTION_SUBTRACT:
            return 3;
        case SELECTION_SYMMETRICDIFFERENCE:
            return 4;
        default:
            return 0;
        }
    }

    SelectionAction buttonIndexToAction(int index) const
    {
        switch (index) {
        case 0:
            return SELECTION_REPLACE;
        case 1:
            return SELECTION_INTERSECT;
        case 2:
            return SELECTION_ADD;
        case 3:
            return SELECTION_SUBTRACT;
        case 4:
            return SELECTION_SYMMETRICDIFFERENCE;
        default:
            return SELECTION_REPLACE;
        }
    }

    int referenceLayersToButtonIndex(ReferenceLayers action) const
    {
        switch (action) {
        case CurrentLayer:
            return 0;
        case AllLayers:
            return 1;
        case ColorLabeledLayers:
            return 2;
        default:
            return 0;
        }
    }

    ReferenceLayers buttonIndexToReferenceLayers(int index) const
    {
        switch (index) {
        case 0:
            return CurrentLayer;
        case 1:
            return AllLayers;
        case 2:
            return ColorLabeledLayers;
        default:
            return CurrentLayer;
        }
    }

    void on_optionButtonStripMode_buttonToggled(int index, bool checked)
    {
        if (!checked) {
            return;
        }
        const SelectionMode mode = buttonIndexToMode(index);
        q->setAdjustmentsSectionVisible(mode == PIXEL_SELECTION);
        Q_EMIT q->modeChanged(mode);
    }

    void on_optionButtonStripAction_buttonToggled(int index, bool checked)
    {
        if (!checked) {
            return;
        }
        Q_EMIT q->actionChanged(buttonIndexToAction(index));
    }

    void on_optionButtonStripReference_buttonToggled(int index, bool checked)
    {
        if (!checked) {
            return;
        }
        const ReferenceLayers referenceLayers =
            buttonIndexToReferenceLayers(index);
        KisOptionCollectionWidgetWithHeader *sectionReference =
            q->widgetAs<KisOptionCollectionWidgetWithHeader *>(
                "sectionReference");
        sectionReference->setWidgetVisible("widgetLabels",
                                           referenceLayers
                                               == ColorLabeledLayers);

        Q_EMIT q->referenceLayersChanged(referenceLayers);
    }
};

KisSelectionOptions::KisSelectionOptions(QWidget *parent)
    : KisOptionCollectionWidget(parent)
    , m_d(new Private)
{
    m_d->q = this;
    // Create widgets
    m_d->optionButtonStripMode = new KisOptionButtonStrip;
    m_d->optionButtonStripMode->addButton(
        KisIconUtils::loadIcon("select-pixel"));
    m_d->optionButtonStripMode->addButton(
        KisIconUtils::loadIcon("select-shape"));
    m_d->optionButtonStripMode->button(0)->setChecked(true);

    m_d->optionButtonStripAction = new KisOptionButtonStrip;
    m_d->optionButtonStripAction->addButton(
        KisIconUtils::loadIcon("selection_replace"));
    m_d->optionButtonStripAction->addButton(
        KisIconUtils::loadIcon("selection_intersect"));
    m_d->optionButtonStripAction->addButton(
        KisIconUtils::loadIcon("selection_add"));
    m_d->optionButtonStripAction->addButton(
        KisIconUtils::loadIcon("selection_subtract"));
    m_d->optionButtonStripAction->addButton(
        KisIconUtils::loadIcon("selection_symmetric_difference"));
    m_d->optionButtonStripAction->button(0)->setChecked(true);

    m_d->checkBoxAntiAliasSelection = new QCheckBox(
        i18nc("The anti-alias checkbox in fill tool options", "Anti-aliasing"));
    KisOptionCollectionWidget *containerGrowSelection = new KisOptionCollectionWidget;
    m_d->sliderGrowSelection = new KisSliderSpinBox;
    m_d->sliderGrowSelection->setPrefix(
        i18nc("The 'grow/shrink' spinbox prefix in selection tools options",
              "Grow: "));
    m_d->sliderGrowSelection->setRange(-400, 400);
    m_d->sliderGrowSelection->setSoftRange(-40, 40);
    m_d->sliderGrowSelection->setSuffix(i18n(" px"));
    m_d->buttonStopGrowingAtDarkestPixel = new QToolButton;
    m_d->buttonStopGrowingAtDarkestPixel->setAutoRaise(true);
    m_d->buttonStopGrowingAtDarkestPixel->setCheckable(true);
    m_d->buttonStopGrowingAtDarkestPixel->setIcon(KisIconUtils::loadIcon("stop-at-boundary"));
    containerGrowSelection->appendWidget("sliderGrowSelection", m_d->sliderGrowSelection);
    containerGrowSelection->appendWidget("buttonStopGrowingAtDarkestPixel", m_d->buttonStopGrowingAtDarkestPixel);
    containerGrowSelection->setOrientation(Qt::Horizontal);
    containerGrowSelection->setWidgetVisible("buttonStopGrowingAtDarkestPixel", false);
    m_d->sliderFeatherSelection = new KisSliderSpinBox;
    m_d->sliderFeatherSelection->setPrefix(
        i18nc("The 'feather' spinbox prefix in selection tools options",
              "Feather: "));
    m_d->sliderFeatherSelection->setRange(0, 400);
    m_d->sliderFeatherSelection->setSoftRange(0, 40);
    m_d->sliderFeatherSelection->setSuffix(i18n(" px"));

    m_d->optionButtonStripReference = new KisOptionButtonStrip;
    m_d->optionButtonStripReference->addButton(
        KisIconUtils::loadIcon("current-layer"));
    m_d->optionButtonStripReference->addButton(
        KisIconUtils::loadIcon("all-layers"));
    m_d->optionButtonStripReference->addButton(KisIconUtils::loadIcon("tag"));
    m_d->optionButtonStripReference->button(0)->setChecked(true);
    m_d->widgetLabels = new KisColorLabelSelectorWidget;
    m_d->widgetLabels->setExclusive(false);
    m_d->widgetLabels->setButtonSize(20);
    m_d->widgetLabels->setButtonWrapEnabled(true);
    m_d->widgetLabels->setMouseDragEnabled(true);

    // Set the tooltips
    m_d->optionButtonStripMode->button(0)->setToolTip(
        i18nc("@info:tooltip", "Pixel Selection"));
    m_d->optionButtonStripMode->button(1)->setToolTip(
        i18nc("@info:tooltip", "Vector Selection"));

    m_d->optionButtonStripAction->button(0)->setToolTip(
        i18nc("@info:tooltip", "Replace"));
    m_d->optionButtonStripAction->button(1)->setToolTip(
        i18nc("@info:tooltip", "Intersect"));
    m_d->optionButtonStripAction->button(2)->setToolTip(
        i18nc("@info:tooltip", "Add"));
    m_d->optionButtonStripAction->button(3)->setToolTip(
        i18nc("@info:tooltip", "Subtract"));
    m_d->optionButtonStripAction->button(4)->setToolTip(
        i18nc("@info:tooltip", "Symmetric Difference"));

    m_d->checkBoxAntiAliasSelection->setToolTip(
        i18n("Smooths the edges of the selection"));
    m_d->sliderGrowSelection->setToolTip(
        i18n("Grow or shrink the selection by the set amount"));
    m_d->buttonStopGrowingAtDarkestPixel->setToolTip(
        i18n("Stop growing at the darkest and/or most opaque pixels"));
    m_d->sliderFeatherSelection->setToolTip(
        i18n("Blur the selection by the set amount"));

    m_d->optionButtonStripReference->button(0)->setToolTip(
        i18n("Select regions found from the active layer"));
    m_d->optionButtonStripReference->button(1)->setToolTip(
        i18n("Select regions found from the merging of all layers"));
    m_d->optionButtonStripReference->button(2)->setToolTip(
        i18n("Select regions found from the merging of layers with specific color labels"));

    // Construct the option widget
    setSeparatorsVisible(true);

    KisOptionCollectionWidgetWithHeader *sectionMode =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'mode' section label in selection tools options",
                  "Mode"));
    sectionMode->setPrimaryWidget(m_d->optionButtonStripMode);
    appendWidget("sectionMode", sectionMode);

    KisOptionCollectionWidgetWithHeader *sectionAction =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'action' section label in selection tools options",
                  "Action"));
    sectionAction->setPrimaryWidget(m_d->optionButtonStripAction);
    appendWidget("sectionAction", sectionAction);

    KisOptionCollectionWidgetWithHeader *sectionReference =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'reference' section label in selection tools options",
                  "Reference"));
    sectionReference->setPrimaryWidget(m_d->optionButtonStripReference);
    sectionReference->appendWidget("widgetLabels", m_d->widgetLabels);
    sectionReference->setWidgetVisible("widgetLabels", false);
    appendWidget("sectionReference", sectionReference);

    KisOptionCollectionWidgetWithHeader *sectionAdjustments =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'adjustments' section label in selection tools options",
                  "Adjustments"));
    sectionAdjustments->appendWidget("checkBoxAntiAliasSelection",
                                     m_d->checkBoxAntiAliasSelection);
    sectionAdjustments->appendWidget("containerGrowSelection", containerGrowSelection);
    sectionAdjustments->appendWidget("sliderFeather",
                                     m_d->sliderFeatherSelection);
    appendWidget("sectionAdjustments", sectionAdjustments);

    // Make connections
    connect(m_d->optionButtonStripMode,
            QOverload<int, bool>::of(&KisOptionButtonStrip::buttonToggled),
            [this](int i, int c) {
                m_d->on_optionButtonStripMode_buttonToggled(i, c);
            });
    connect(m_d->optionButtonStripAction,
            QOverload<int, bool>::of(&KisOptionButtonStrip::buttonToggled),
            [this](int i, int c) {
                m_d->on_optionButtonStripAction_buttonToggled(i, c);
            });
    connect(m_d->checkBoxAntiAliasSelection,
            SIGNAL(toggled(bool)),
            SIGNAL(antiAliasSelectionChanged(bool)));
    connect(m_d->sliderGrowSelection,
            SIGNAL(valueChanged(int)),
            SIGNAL(growSelectionChanged(int)));
    connect(m_d->buttonStopGrowingAtDarkestPixel,
            SIGNAL(toggled(bool)),
            SIGNAL(stopGrowingAtDarkestPixelChanged(bool)));
    connect(m_d->sliderFeatherSelection,
            SIGNAL(valueChanged(int)),
            SIGNAL(featherSelectionChanged(int)));
    connect(m_d->optionButtonStripReference,
            QOverload<int, bool>::of(&KisOptionButtonStrip::buttonToggled),
            [this](int i, int c) {
                m_d->on_optionButtonStripReference_buttonToggled(i, c);
            });
    connect(m_d->widgetLabels,
            SIGNAL(selectionChanged()),
            SIGNAL(selectedColorLabelsChanged()));
}

KisSelectionOptions::~KisSelectionOptions()
{
}

SelectionMode KisSelectionOptions::mode() const
{
    return m_d->buttonIndexToMode(
        m_d->optionButtonStripMode->checkedButtonIndex());
}

SelectionAction KisSelectionOptions::action() const
{
    return m_d->buttonIndexToAction(
        m_d->optionButtonStripAction->checkedButtonIndex());
}

bool KisSelectionOptions::antiAliasSelection() const
{
    return m_d->checkBoxAntiAliasSelection->isChecked();
}

int KisSelectionOptions::growSelection() const
{
    return m_d->sliderGrowSelection->value();
}

bool KisSelectionOptions::stopGrowingAtDarkestPixel() const
{
    return m_d->buttonStopGrowingAtDarkestPixel->isChecked();
}

int KisSelectionOptions::featherSelection() const
{
    return m_d->sliderFeatherSelection->value();
}

KisSelectionOptions::ReferenceLayers
KisSelectionOptions::referenceLayers() const
{
    return m_d->buttonIndexToReferenceLayers(
        m_d->optionButtonStripReference->checkedButtonIndex());
}

QList<int> KisSelectionOptions::selectedColorLabels() const
{
    return m_d->widgetLabels->selection();
}

void KisSelectionOptions::setMode(SelectionMode newMode)
{
    KoGroupButton *button =
        m_d->optionButtonStripMode->button(m_d->modeToButtonIndex(newMode));
    KIS_SAFE_ASSERT_RECOVER_RETURN(button);

    button->setChecked(true);
}

void KisSelectionOptions::setAction(SelectionAction newAction)
{
    KoGroupButton *button = m_d->optionButtonStripAction->button(
        m_d->actionToButtonIndex(newAction));
    KIS_SAFE_ASSERT_RECOVER_RETURN(button);

    button->setChecked(true);
}

void KisSelectionOptions::setAntiAliasSelection(bool newAntiAliasSelection)
{
    m_d->checkBoxAntiAliasSelection->setChecked(newAntiAliasSelection);
}

void KisSelectionOptions::setGrowSelection(int newGrowSelection)
{
    m_d->sliderGrowSelection->setValue(newGrowSelection);
}

void KisSelectionOptions::setStopGrowingAtDarkestPixel(bool newStopGrowingAtDarkestPixel)
{
    m_d->buttonStopGrowingAtDarkestPixel->setChecked(newStopGrowingAtDarkestPixel);
}

void KisSelectionOptions::setFeatherSelection(int newFeatherSelection)
{
    m_d->sliderFeatherSelection->setValue(newFeatherSelection);
}

void KisSelectionOptions::setReferenceLayers(ReferenceLayers newReferenceLayers)
{
    KoGroupButton *button = m_d->optionButtonStripReference->button(
        m_d->referenceLayersToButtonIndex(newReferenceLayers));
    KIS_SAFE_ASSERT_RECOVER_RETURN(button);

    button->setChecked(true);
}

void KisSelectionOptions::setSelectedColorLabels(
    const QList<int> &newSelectedColorLabels)
{
    m_d->widgetLabels->setSelection(newSelectedColorLabels);
}

void KisSelectionOptions::setModeSectionVisible(bool visible)
{
    setWidgetVisible("sectionMode", visible);
}

void KisSelectionOptions::setActionSectionVisible(bool visible)
{
    setWidgetVisible("sectionAction", visible);
}

void KisSelectionOptions::setStopGrowingAtDarkestPixelButtonVisible(bool visible)
{
    widgetAs<KisOptionCollectionWidgetWithHeader*>("sectionAdjustments")
        ->widgetAs<KisOptionCollectionWidget*>("containerGrowSelection")
            ->setWidgetVisible("buttonStopGrowingAtDarkestPixel", visible);
}

void KisSelectionOptions::setAdjustmentsSectionVisible(bool visible)
{
    setWidgetVisible("sectionAdjustments", visible);
}

void KisSelectionOptions::setReferenceSectionVisible(bool visible)
{
    setWidgetVisible("sectionReference", visible);
}

void KisSelectionOptions::updateActionButtonToolTip(
    SelectionAction action,
    const QKeySequence &shortcut)
{
    const QString shortcutString = shortcut.toString(QKeySequence::NativeText);
    QString toolTipText;
    const int buttonIndex = m_d->actionToButtonIndex(action);

    switch (action) {
    case SELECTION_DEFAULT:
    case SELECTION_REPLACE:
        toolTipText = shortcutString.isEmpty()
            ? i18nc("@info:tooltip", "Replace")
            : i18nc("@info:tooltip", "Replace (%1)", shortcutString);
        break;
    case SELECTION_ADD:
        toolTipText = shortcutString.isEmpty()
            ? i18nc("@info:tooltip", "Add")
            : i18nc("@info:tooltip", "Add (%1)", shortcutString);
        break;
    case SELECTION_SUBTRACT:
        toolTipText = shortcutString.isEmpty()
            ? i18nc("@info:tooltip", "Subtract")
            : i18nc("@info:tooltip", "Subtract (%1)", shortcutString);
        break;
    case SELECTION_INTERSECT:
        toolTipText = shortcutString.isEmpty()
            ? i18nc("@info:tooltip", "Intersect")
            : i18nc("@info:tooltip", "Intersect (%1)", shortcutString);
        break;
    case SELECTION_SYMMETRICDIFFERENCE:
        toolTipText = shortcutString.isEmpty()
            ? i18nc("@info:tooltip", "Symmetric Difference")
            : i18nc("@info:tooltip",
                    "Symmetric Difference (%1)",
                    shortcutString);
        break;
    }

    m_d->optionButtonStripAction->button(buttonIndex)->setToolTip(toolTipText);
}
