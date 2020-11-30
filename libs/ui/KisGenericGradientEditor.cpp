/*
 * KDE. Krita Project.
 *
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QToolButton>
#include <QWidgetAction>
#include <QSlider>
#include <QHeaderView>

#include <KoStopGradient.h>
#include <KoSegmentGradient.h>
#include <kis_stopgradient_editor.h>
#include <kis_autogradient.h>
#include <KoResourceItemChooser.h>
#include <kis_icon_utils.h>
#include <KisGradientConversion.h>
#include <KoResourceItemView.h>
#include <kis_signals_blocker.h>
#include <KoResourceServerProvider.h>
#include <kis_canvas_resource_provider.h>
#include <KoResourceServerProvider.h>
#include <KoResourceServerAdapter.h>

#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include "KisGenericGradientEditor.h"

class Q_DECL_HIDDEN KisGenericGradientEditor::Private
{
public:
    KoAbstractGradientSP gradient;
    KoColor foregroundColor, backgroundColor;

    QPushButton *buttonConvertGradient;
    QPushButton *buttonUpdateGradient;
    QPushButton *buttonAddGradient;
    QLabel *labelConvertGradientWarning;
    KoResourceItemChooser *widgetGradientPresetChooser;
    QToolButton *toolButtonGradientPresetChooser;
    QWidget *widgetToolButtonGradientPresetChooser;
    QToolButton *toolButtonGradientPresetChooserOptions;
    QAction *actionUseGradientPresetChooserPopUp;
    QAction *actionCompactGradientPresetChooserMode;
    QAction *actionGradientPresetChooserViewModeIcon;
    QAction *actionGradientPresetChooserViewModeList;
    QAction *actionGradientPresetChooserItemSizeSmall;
    QAction *actionGradientPresetChooserItemSizeMedium;
    QAction *actionGradientPresetChooserItemSizeLarge;
    QAction *actionGradientPresetChooserItemSizeCustom;
    QSlider *sliderGradientPresetChooserItemSizeCustom;
    QWidget *widgetSliderGradientPresetChooserItemSizeCustom;
    QWidget *widgetGradientEditor;

    bool compactMode;
    bool isConvertGradientButtonVisible;
    bool isUpdateGradientButtonVisible;
    bool isAddGradientButtonVisible;
    bool isGradientPresetChooserVisible;
    bool isGradientPresetChooserOptionsButtonVisible;
    bool useGradientPresetChooserPopUp;
    bool compactGradientPresetChooserMode;
    GradientPresetChooserViewMode gradientPresetChooserViewMode;
    GradientPresetChooserItemSize gradientPresetChooserItemSize;
    int gradientPresetChooserItemSizeSmall;
    int gradientPresetChooserItemSizeMedium;
    int gradientPresetChooserItemSizeLarge;
    int gradientPresetChooserItemSizeCustom;
    qreal gradientPresetChooserItemSizeWidthFactor;
    bool compactGradientEditorMode;
};

KisGenericGradientEditor::KisGenericGradientEditor(QWidget* parent)
    : QWidget(parent)
    , m_d(new Private)
{
    m_d->gradient = nullptr;
    m_d->foregroundColor = KoColor();
    m_d->backgroundColor = KoColor();
    m_d->widgetGradientEditor = nullptr;

    QVBoxLayout *layoutMain = new QVBoxLayout;
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->setSpacing(10);

    QHBoxLayout *layoutButtons = new QHBoxLayout;
    layoutButtons->setContentsMargins(0, 0, 0, 0);
    layoutButtons->setSpacing(5);

    m_d->buttonConvertGradient = new QPushButton(this);
    m_d->labelConvertGradientWarning = new QLabel(this);
    m_d->labelConvertGradientWarning->setPixmap(KisIconUtils::loadIcon("warning").pixmap(16, 16));
    m_d->labelConvertGradientWarning->setToolTip(
        i18nc(
            "Warning text shown when converting from a segment gradient to a stop gradient",
            "Converting a segment gradient to a stop gradient may cause lose of information"
        )
    );

    KoResourceServer<KoAbstractGradient> *rserver = KoResourceServerProvider::instance()->gradientServer();
    QSharedPointer<KoAbstractResourceServerAdapter> adapter (new KoResourceServerAdapter<KoAbstractGradient>(rserver));
    m_d->widgetGradientPresetChooser = new KoResourceItemChooser(adapter, this);
    m_d->widgetGradientPresetChooser->itemView()->keepAspectRatio(false);
    m_d->widgetGradientPresetChooser->itemView()->setShowGrid(false);
    m_d->widgetGradientPresetChooser->itemView()->horizontalHeader()->setMinimumSectionSize(0);
    m_d->widgetGradientPresetChooser->itemView()->verticalHeader()->setMinimumSectionSize(0);

    m_d->toolButtonGradientPresetChooser = new QToolButton(this);
    m_d->toolButtonGradientPresetChooser->setText(
        i18nc("Choose a preset gradient from the button popup", "Choose Gradient Preset")
    );
    m_d->toolButtonGradientPresetChooser->setPopupMode(QToolButton::InstantPopup);
    m_d->widgetToolButtonGradientPresetChooser = new QWidget(this);
    QVBoxLayout *layoutWidgetToolButtonGradientPresetChooser = new QVBoxLayout;
    layoutWidgetToolButtonGradientPresetChooser->setContentsMargins(0, 0, 0, 0);
    layoutWidgetToolButtonGradientPresetChooser->setSpacing(0);
    m_d->widgetToolButtonGradientPresetChooser->setLayout(layoutWidgetToolButtonGradientPresetChooser);
    QWidgetAction *widgetActionToolButtonGradientPresetChooser = new QWidgetAction(this);
    widgetActionToolButtonGradientPresetChooser->setDefaultWidget(m_d->widgetToolButtonGradientPresetChooser);
    m_d->toolButtonGradientPresetChooser->addAction(widgetActionToolButtonGradientPresetChooser);
    
    m_d->buttonUpdateGradient = new QPushButton(this);
    m_d->buttonUpdateGradient->setIcon(KisIconUtils::loadIcon("document-save"));
    m_d->buttonUpdateGradient->setToolTip(
        i18nc(
            "Update the current gradient in the presets with the one in the generic gradient editor",
            "Update the selected gradient preset with the current gradient"
        )
    );
    
    m_d->buttonAddGradient = new QPushButton(this);
    m_d->buttonAddGradient->setIcon(KisIconUtils::loadIcon("list-add"));
    m_d->buttonAddGradient->setToolTip(
        i18nc(
            "Add the current gradient in the generic gradient editor to the presets",
            "Add the current gradient to the presets"
        )
    );

    m_d->toolButtonGradientPresetChooserOptions = new QToolButton(this);
    m_d->toolButtonGradientPresetChooserOptions->setPopupMode(QToolButton::InstantPopup);
    m_d->toolButtonGradientPresetChooserOptions->setAutoRaise(true);
    m_d->toolButtonGradientPresetChooserOptions->setIcon(KisIconUtils::loadIcon("hamburger_menu_dots"));
    m_d->toolButtonGradientPresetChooserOptions->setStyleSheet("QToolButton::menu-indicator { image: none; }");
    m_d->actionUseGradientPresetChooserPopUp = new QAction(this);
    m_d->actionUseGradientPresetChooserPopUp->setCheckable(true);
    m_d->actionUseGradientPresetChooserPopUp->setText(
        i18nc(
            "Show the gradient preset chooser as a pop-up in a button or inline",
            "Use a pop-up gradient preset chooser"
        )
    );
    m_d->actionCompactGradientPresetChooserMode = new QAction(this);
    m_d->actionCompactGradientPresetChooserMode->setCheckable(true);
    m_d->actionCompactGradientPresetChooserMode->setText(
        i18nc(
            "Hide/show option widgets around the gradient preset chooser",
            "Show compact gradient preset chooser"
        )
    );
    m_d->toolButtonGradientPresetChooserOptions->addAction(m_d->actionUseGradientPresetChooserPopUp);
    m_d->toolButtonGradientPresetChooserOptions->addAction(m_d->actionCompactGradientPresetChooserMode);

    QActionGroup *actionGroupGradientPresetChooserViewMode = new QActionGroup(this);
    m_d->actionGradientPresetChooserViewModeIcon = new QAction(this);
    m_d->actionGradientPresetChooserViewModeIcon->setCheckable(true);
    m_d->actionGradientPresetChooserViewModeIcon->setActionGroup(actionGroupGradientPresetChooserViewMode);
    m_d->actionGradientPresetChooserViewModeIcon->setText(
        i18nc(
            "Set the gradient preset chooser to show icons instead of a list",
            "Icon view"
        )
    );
    m_d->actionGradientPresetChooserViewModeList = new QAction(this);
    m_d->actionGradientPresetChooserViewModeList->setCheckable(true);
    m_d->actionGradientPresetChooserViewModeList->setActionGroup(actionGroupGradientPresetChooserViewMode);
    m_d->actionGradientPresetChooserViewModeList->setText(
        i18nc(
            "Set the gradient preset chooser to show a list instead of icons",
            "List view"
        )
    );
    QAction *separatorGradientPresetChooserViewMode1 = new QAction(this);
    separatorGradientPresetChooserViewMode1->setSeparator(true);
    QActionGroup *actionGroupGradientPresetChooserItemSize = new QActionGroup(this);
    m_d->actionGradientPresetChooserItemSizeSmall = new QAction(this);
    m_d->actionGradientPresetChooserItemSizeSmall->setCheckable(true);
    m_d->actionGradientPresetChooserItemSizeSmall->setActionGroup(actionGroupGradientPresetChooserItemSize);
    m_d->actionGradientPresetChooserItemSizeSmall->setText(
        i18nc(
            "Set the gradient preset chooser to show small items",
            "Small items"
        )
    );
    m_d->actionGradientPresetChooserItemSizeMedium = new QAction(this);
    m_d->actionGradientPresetChooserItemSizeMedium->setCheckable(true);
    m_d->actionGradientPresetChooserItemSizeMedium->setActionGroup(actionGroupGradientPresetChooserItemSize);
    m_d->actionGradientPresetChooserItemSizeMedium->setText(
        i18nc(
            "Set the gradient preset chooser to show medium size items",
            "Medium size items"
        )
    );
    m_d->actionGradientPresetChooserItemSizeLarge = new QAction(this);
    m_d->actionGradientPresetChooserItemSizeLarge->setCheckable(true);
    m_d->actionGradientPresetChooserItemSizeLarge->setActionGroup(actionGroupGradientPresetChooserItemSize);
    m_d->actionGradientPresetChooserItemSizeLarge->setText(
        i18nc(
            "Set the gradient preset chooser to show large items",
            "Large items"
        )
    );
    m_d->actionGradientPresetChooserItemSizeCustom = new QAction(this);
    m_d->actionGradientPresetChooserItemSizeCustom->setCheckable(true);
    m_d->actionGradientPresetChooserItemSizeCustom->setActionGroup(actionGroupGradientPresetChooserItemSize);
    m_d->actionGradientPresetChooserItemSizeCustom->setText(
        i18nc(
            "Set the gradient preset chooser to show custom size items",
            "Custom size items"
        )
    );
    m_d->sliderGradientPresetChooserItemSizeCustom = new QSlider(this);
    m_d->sliderGradientPresetChooserItemSizeCustom->setRange(16, 128);
    m_d->sliderGradientPresetChooserItemSizeCustom->setOrientation(Qt::Horizontal);
    m_d->widgetSliderGradientPresetChooserItemSizeCustom = new QWidget(this);
    QVBoxLayout *layoutWidgetSliderGradientPresetChooserItemSizeCustom = new QVBoxLayout;
    layoutWidgetSliderGradientPresetChooserItemSizeCustom->addWidget(m_d->sliderGradientPresetChooserItemSizeCustom);
    m_d->widgetSliderGradientPresetChooserItemSizeCustom->setLayout(
        layoutWidgetSliderGradientPresetChooserItemSizeCustom
    );
    QWidgetAction *widgetActionSliderGradientPresetChooserItemSizeCustom = new QWidgetAction(this);
    widgetActionSliderGradientPresetChooserItemSizeCustom->setDefaultWidget(
        m_d->widgetSliderGradientPresetChooserItemSizeCustom
    );
    QToolButton *toolButtonWidgetGradientPresetChooserViewMode =
        m_d->widgetGradientPresetChooser->viewModeButton();
    toolButtonWidgetGradientPresetChooserViewMode->addActions(actionGroupGradientPresetChooserViewMode->actions());
    toolButtonWidgetGradientPresetChooserViewMode->addAction(separatorGradientPresetChooserViewMode1);
    toolButtonWidgetGradientPresetChooserViewMode->addActions(actionGroupGradientPresetChooserItemSize->actions());
    toolButtonWidgetGradientPresetChooserViewMode->addAction(widgetActionSliderGradientPresetChooserItemSizeCustom);

    layoutButtons->addWidget(m_d->buttonAddGradient, 0);
    layoutButtons->addWidget(m_d->buttonUpdateGradient, 0);
    layoutButtons->addWidget(m_d->buttonConvertGradient, 0);
    layoutButtons->addWidget(m_d->labelConvertGradientWarning, 0);
    layoutButtons->addStretch(1);
    layoutButtons->addWidget(m_d->toolButtonGradientPresetChooser, 0);
    layoutButtons->addWidget(m_d->toolButtonGradientPresetChooserOptions, 0);

    layoutMain->addWidget(m_d->widgetGradientPresetChooser, 1);
    layoutMain->addLayout(layoutButtons, 0);
    layoutMain->addStretch();

    setLayout(layoutMain);

    m_d->compactMode = false;
    m_d->isConvertGradientButtonVisible = true;
    m_d->isUpdateGradientButtonVisible = true;
    m_d->isAddGradientButtonVisible = true;
    m_d->isGradientPresetChooserVisible = true;
    m_d->isGradientPresetChooserOptionsButtonVisible = true;
    m_d->useGradientPresetChooserPopUp = true;
    m_d->compactGradientPresetChooserMode = false;
    m_d->gradientPresetChooserViewMode = GradientPresetChooserViewMode_Icon;
    m_d->gradientPresetChooserItemSize = GradientPresetChooserItemSize_Medium;
    m_d->gradientPresetChooserItemSizeSmall = 32;
    m_d->gradientPresetChooserItemSizeMedium = 48;
    m_d->gradientPresetChooserItemSizeLarge = 64;
    m_d->gradientPresetChooserItemSizeCustom = 32;
    m_d->gradientPresetChooserItemSizeWidthFactor = 2.0;
    m_d->compactGradientEditorMode = false;

    updateConvertGradientButton();
    updateUpdateGradientButton();
    updateAddGradientButton();
    updateGradientPresetChooser();
    updateGradientEditor();
    updateWidgetSliderGradientPresetChooserItemSizeCustom();

    connect(
        m_d->buttonConvertGradient,
        SIGNAL(clicked()),
        this,
        SLOT(on_buttonConvertGradient_clicked())
    );

    connect(
        m_d->buttonUpdateGradient,
        SIGNAL(clicked()),
        this,
        SLOT(on_buttonUpdateGradient_clicked())
    );

    connect(
        m_d->buttonAddGradient,
        SIGNAL(clicked()),
        this,
        SLOT(on_buttonAddGradient_clicked())
    );

    connect(
        m_d->widgetGradientPresetChooser,
        SIGNAL(resourceSelected(KoResource*)),
        this,
        SLOT(on_widgetGradientPresetChooser_resourceClicked(KoResource*))
    );

    connect(
        m_d->widgetGradientPresetChooser,
        SIGNAL(resourceClicked(KoResource*)),
        this,
        SLOT(on_widgetGradientPresetChooser_resourceClicked(KoResource*))
    );

    connect(
        m_d->widgetGradientPresetChooser->itemView(),
        SIGNAL(sigSizeChanged()),
        this,
        SLOT(on_widgetGradientPresetChooserItemView_sigSizeChanged())
    );

    connect(
        m_d->actionUseGradientPresetChooserPopUp,
        SIGNAL(toggled(bool)),
        this,
        SLOT(setUseGradientPresetChooserPopUp(bool))
    );

    connect(
        m_d->actionCompactGradientPresetChooserMode,
        SIGNAL(toggled(bool)),
        this,
        SLOT(setCompactGradientPresetChooserMode(bool))
    );

    connect(
        actionGroupGradientPresetChooserViewMode,
        SIGNAL(triggered(QAction*)),
        this,
        SLOT(on_actionGroupGradientPresetChooserViewMode_triggered(QAction*))
    );

    connect(
        actionGroupGradientPresetChooserItemSize,
        SIGNAL(triggered(QAction*)),
        this,
        SLOT(on_actionGroupGradientPresetChooserItemSize_triggered(QAction*))
    );

    connect(
        m_d->sliderGradientPresetChooserItemSizeCustom,
        SIGNAL(valueChanged(int)),
        this,
        SLOT(on_sliderGradientPresetChooserItemSizeCustom_valueChanged(int))
    );
}

KisGenericGradientEditor::~KisGenericGradientEditor()
{}

void KisGenericGradientEditor::loadUISettings(const QString &prefix)
{
    KConfigGroup configGroup(KSharedConfig::openConfig(), "GenericGradientEditor");
    const QString pfx = prefix.isEmpty() ? QString("global/") : prefix + "/";

    m_d->useGradientPresetChooserPopUp = configGroup.readEntry(pfx + "useGradientPresetChooserPopUp", m_d->useGradientPresetChooserPopUp);
    m_d->compactGradientPresetChooserMode = configGroup.readEntry(pfx + "compactGradientPresetChooserMode", m_d->compactGradientPresetChooserMode);
    QString strGradientPresetChooserViewMode = configGroup.readEntry(pfx + "gradientPresetChooserViewMode", QString());
    if (strGradientPresetChooserViewMode == "icon") {
        m_d->gradientPresetChooserViewMode = GradientPresetChooserViewMode_Icon;
    } else if (strGradientPresetChooserViewMode == "list") {
        m_d->gradientPresetChooserViewMode = GradientPresetChooserViewMode_List;
    }
    QString strGradientPresetChooserItemSize = configGroup.readEntry(pfx + "gradientPresetChooserItemSize", QString());
    if (strGradientPresetChooserItemSize == "small") {
        m_d->gradientPresetChooserItemSize = GradientPresetChooserItemSize_Small;
    } else if (strGradientPresetChooserItemSize == "medium") {
        m_d->gradientPresetChooserItemSize = GradientPresetChooserItemSize_Medium;
    } else if (strGradientPresetChooserItemSize == "large") {
        m_d->gradientPresetChooserItemSize = GradientPresetChooserItemSize_Large;
    } else if (strGradientPresetChooserItemSize == "custom") {
        m_d->gradientPresetChooserItemSize = GradientPresetChooserItemSize_Custom;
    }
    m_d->gradientPresetChooserItemSizeCustom = configGroup.readEntry(pfx + "gradientPresetChooserItemSizeCustom", m_d->gradientPresetChooserItemSizeCustom);

    updateGradientPresetChooser();
    updateWidgetSliderGradientPresetChooserItemSizeCustom();
}

void KisGenericGradientEditor::saveUISettings(const QString &prefix)
{
    KConfigGroup configGroup(KSharedConfig::openConfig(), "GenericGradientEditor");
    const QString pfx = prefix.isEmpty() ? QString("global/") : prefix + "/";

    configGroup.writeEntry(pfx + "useGradientPresetChooserPopUp", m_d->useGradientPresetChooserPopUp);
    configGroup.writeEntry(pfx + "compactGradientPresetChooserMode", m_d->compactGradientPresetChooserMode);
    if (m_d->gradientPresetChooserViewMode == GradientPresetChooserViewMode_Icon) {
        configGroup.writeEntry(pfx + "gradientPresetChooserViewMode", "icon");
    } else {
        configGroup.writeEntry(pfx + "gradientPresetChooserViewMode", "list");
    }
    if (m_d->gradientPresetChooserItemSize == GradientPresetChooserItemSize_Small) {
        configGroup.writeEntry(pfx + "gradientPresetChooserItemSize", "small");
    } else if (m_d->gradientPresetChooserItemSize == GradientPresetChooserItemSize_Medium) {
        configGroup.writeEntry(pfx + "gradientPresetChooserItemSize", "medium");
    } else if (m_d->gradientPresetChooserItemSize == GradientPresetChooserItemSize_Large) {
        configGroup.writeEntry(pfx + "gradientPresetChooserItemSize", "large");
    } else {
        configGroup.writeEntry(pfx + "gradientPresetChooserItemSize", "custom");
    }
    configGroup.writeEntry(pfx + "gradientPresetChooserItemSizeCustom", m_d->gradientPresetChooserItemSizeCustom);
}

KoAbstractGradientSP KisGenericGradientEditor::gradient() const
{
    if (m_d->gradient) {
        return KoAbstractGradientSP(m_d->gradient->clone());
    }
    return nullptr;
}

const KoColor& KisGenericGradientEditor::foregroundColor() const
{
    return m_d->foregroundColor;
}

const KoColor& KisGenericGradientEditor::backgroundColor() const
{
    return m_d->backgroundColor;
}

bool KisGenericGradientEditor::compactMode() const
{
    return m_d->compactMode;
}

bool KisGenericGradientEditor::isConvertGradientButtonVisible() const
{
    return m_d->isConvertGradientButtonVisible;
}

bool KisGenericGradientEditor::isUpdateGradientButtonVisible() const
{
    return m_d->isUpdateGradientButtonVisible;
}

bool KisGenericGradientEditor::isAddGradientButtonVisible() const
{
    return m_d->isAddGradientButtonVisible;
}

bool KisGenericGradientEditor::isGradientPresetChooserVisible() const
{
    return m_d->isGradientPresetChooserVisible;
}

bool KisGenericGradientEditor::isGradientPresetChooserOptionsButtonVisible() const
{
    return m_d->isGradientPresetChooserOptionsButtonVisible;
}

bool KisGenericGradientEditor::useGradientPresetChooserPopUp() const
{
    return m_d->useGradientPresetChooserPopUp;
}

bool KisGenericGradientEditor::compactGradientPresetChooserMode() const
{
    return m_d->compactGradientPresetChooserMode;
}

KisGenericGradientEditor::GradientPresetChooserViewMode KisGenericGradientEditor::gradientPresetChooserViewMode() const
{
    return m_d->gradientPresetChooserViewMode;
}

KisGenericGradientEditor::GradientPresetChooserItemSize KisGenericGradientEditor::gradientPresetChooserItemSize() const
{
    return m_d->gradientPresetChooserItemSize;
}

int KisGenericGradientEditor::gradientPresetChooserItemSizeSmall() const
{
    return m_d->gradientPresetChooserItemSizeSmall;
}

int KisGenericGradientEditor::gradientPresetChooserItemSizeMedium() const
{
    return m_d->gradientPresetChooserItemSizeMedium;
}

int KisGenericGradientEditor::gradientPresetChooserItemSizeLarge() const
{
    return m_d->gradientPresetChooserItemSizeLarge;
}

int KisGenericGradientEditor::gradientPresetChooserItemSizeCustom() const
{
    return m_d->gradientPresetChooserItemSizeCustom;
}

qreal KisGenericGradientEditor::gradientPresetChooserItemSizeWidthFactor() const
{
    return m_d->gradientPresetChooserItemSizeWidthFactor;
}

bool KisGenericGradientEditor::compactGradientEditorMode() const
{
    return m_d->compactGradientEditorMode;
}

void KisGenericGradientEditor::setGradient(KoAbstractGradientSP newGradient)
{
    if (newGradient == m_d->gradient) {
        return;
    }

    if (!newGradient || !newGradient->valid()) {
        if (m_d->widgetGradientEditor) {
            layout()->removeWidget(m_d->widgetGradientEditor);
            delete m_d->widgetGradientEditor;
            m_d->widgetGradientEditor = nullptr;
        }
        m_d->gradient = nullptr;
        updateConvertGradientButton();
        updateUpdateGradientButton();
        updateAddGradientButton();
        updateGradientEditor();
        return;
    }

    m_d->gradient = KoAbstractGradientSP(newGradient->clone());
    m_d->gradient->setVariableColors(m_d->foregroundColor, m_d->backgroundColor);
    
    QWidget *newGradientEditorWidget = nullptr;
    if (dynamic_cast<KoStopGradient*>(m_d->gradient.data())) {
        if (dynamic_cast<KisStopGradientEditor*>(m_d->widgetGradientEditor)) {
            dynamic_cast<KisStopGradientEditor*>(m_d->widgetGradientEditor)->setGradient(dynamic_cast<KoStopGradient*>(m_d->gradient.data()));
        } else {
            newGradientEditorWidget =
                new KisStopGradientEditor(
                    dynamic_cast<KoStopGradient*>(m_d->gradient.data()), nullptr, "", "",
                    m_d->foregroundColor, m_d->backgroundColor
                );
        }
    } else if (dynamic_cast<KoSegmentGradient*>(m_d->gradient.data())) {
        if (dynamic_cast<KisAutogradientEditor*>(m_d->widgetGradientEditor)) {
            dynamic_cast<KisAutogradientEditor*>(m_d->widgetGradientEditor)->setGradient(dynamic_cast<KoSegmentGradient*>(m_d->gradient.data()));
        } else {
            newGradientEditorWidget =
                new KisAutogradientEditor(
                    dynamic_cast<KoSegmentGradient*>(m_d->gradient.data()), nullptr, "", "",
                    m_d->foregroundColor, m_d->backgroundColor
                );
        }
    }

    if (newGradientEditorWidget) {
        QWidget *oldGradientEditorWidget = m_d->widgetGradientEditor;
        m_d->widgetGradientEditor = newGradientEditorWidget;
        m_d->widgetGradientEditor->layout()->setContentsMargins(0, 0, 0, 0);
        m_d->widgetGradientEditor->setMinimumSize(0, 0);
        m_d->widgetGradientEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
        if (dynamic_cast<KisStopGradientEditor*>(m_d->widgetGradientEditor)) {
            dynamic_cast<KisStopGradientEditor*>(m_d->widgetGradientEditor)->setCompactMode(m_d->compactGradientPresetChooserMode);
        } else {
            dynamic_cast<KisAutogradientEditor*>(m_d->widgetGradientEditor)->setCompactMode(m_d->compactGradientPresetChooserMode);
        }
        if (oldGradientEditorWidget) {
            setUpdatesEnabled(false);
            layout()->replaceWidget(oldGradientEditorWidget, m_d->widgetGradientEditor);
            dynamic_cast<QVBoxLayout*>(layout())->setStretchFactor(m_d->widgetGradientEditor, 0);
            layout()->activate();
            delete oldGradientEditorWidget;
            setUpdatesEnabled(true);
        } else {
            dynamic_cast<QVBoxLayout*>(layout())->insertWidget(
                m_d->useGradientPresetChooserPopUp ? 1 : 2,
                m_d->widgetGradientEditor,
                0
            );
        }
        connect(m_d->widgetGradientEditor, SIGNAL(sigGradientChanged()), this, SLOT(on_widgetGradientEditor_sigGradientChanged()));
        updateConvertGradientButton();
        updateUpdateGradientButton();
        updateAddGradientButton();
        updateGradientEditor();
    }

    emit sigGradientChanged();
}

void KisGenericGradientEditor::setForegroundColor(const KoColor &newForegroundColor)
{
    m_d->foregroundColor = newForegroundColor;
    if (m_d->gradient) {
        m_d->gradient->setVariableColors(m_d->foregroundColor, m_d->backgroundColor);
    }
}

void KisGenericGradientEditor::setBackgroundColor(const KoColor &newBackgroundColor)
{
    m_d->backgroundColor = newBackgroundColor;
    if (m_d->gradient) {
        m_d->gradient->setVariableColors(m_d->foregroundColor, m_d->backgroundColor);
    }
}

void KisGenericGradientEditor::setVariableColors(const KoColor &newForegroundColor,
                                                 const KoColor &newBackgroundColor)
{
    m_d->foregroundColor = newForegroundColor;
    m_d->backgroundColor = newBackgroundColor;
    if (m_d->gradient) {
        m_d->gradient->setVariableColors(m_d->foregroundColor, m_d->backgroundColor);
    }
}

void KisGenericGradientEditor::setCompactMode(bool compact)
{
    m_d->compactMode = compact;
    
    updateConvertGradientButton();
    updateUpdateGradientButton();
    updateAddGradientButton();
    updateGradientPresetChooser();
}

void KisGenericGradientEditor::setConvertGradientButtonVisible(bool visible)
{
    m_d->isConvertGradientButtonVisible = visible;
    updateConvertGradientButton();
}

void KisGenericGradientEditor::setUpdateGradientButtonVisible(bool visible)
{
    m_d->isUpdateGradientButtonVisible = visible;
    updateUpdateGradientButton();
}

void KisGenericGradientEditor::setAddGradientButtonVisible(bool visible)
{
    m_d->isAddGradientButtonVisible = visible;
    updateAddGradientButton();
}

void KisGenericGradientEditor::setGradientPresetChooserVisible(bool visible)
{
    m_d->isGradientPresetChooserVisible = visible;
    updateGradientPresetChooser();
}

void KisGenericGradientEditor::setGradientPresetChooserOptionsButtonVisible(bool visible)
{
    m_d->isGradientPresetChooserOptionsButtonVisible = visible;
    updateGradientPresetChooser();
}

void KisGenericGradientEditor::setUseGradientPresetChooserPopUp(bool use)
{
    m_d->useGradientPresetChooserPopUp = use;
    updateGradientPresetChooser();
}

void KisGenericGradientEditor::setCompactGradientPresetChooserMode(bool compact)
{
    m_d->compactGradientPresetChooserMode = compact;
    updateGradientPresetChooser();
}

void KisGenericGradientEditor::setGradientPresetChooserViewMode(GradientPresetChooserViewMode newViewMode)
{
    m_d->gradientPresetChooserViewMode = newViewMode;
    updateGradientPresetChooser();
}

void KisGenericGradientEditor::setGradientPresetChooserItemSize(GradientPresetChooserItemSize newItemSize)
{
    m_d->gradientPresetChooserItemSize = newItemSize;
    updateGradientPresetChooser();
}

void KisGenericGradientEditor::setGradientPresetChooserItemSizeSmall(int newSize)
{
    if (newSize == m_d->gradientPresetChooserItemSizeSmall) {
        return;
    }

    m_d->gradientPresetChooserItemSizeSmall = newSize;
    updateGradientPresetChooser();
}

void KisGenericGradientEditor::setGradientPresetChooserItemSizeMedium(int newSize)
{
    if (newSize == m_d->gradientPresetChooserItemSizeMedium) {
        return;
    }

    m_d->gradientPresetChooserItemSizeMedium = newSize;
    updateGradientPresetChooser();
}

void KisGenericGradientEditor::setGradientPresetChooserItemSizeLarge(int newSize)
{
    if (newSize == m_d->gradientPresetChooserItemSizeLarge) {
        return;
    }

    m_d->gradientPresetChooserItemSizeLarge = newSize;
    updateGradientPresetChooser();
}

void KisGenericGradientEditor::setGradientPresetChooserItemSizeCustom(int newSize)
{
    if (newSize == m_d->gradientPresetChooserItemSizeCustom) {
        return;
    }

    m_d->gradientPresetChooserItemSizeCustom = newSize;
    updateGradientPresetChooser();
}

void KisGenericGradientEditor::setGradientPresetChooserItemSizeWidthFactor(qreal newFactor)
{
    if (newFactor == m_d->gradientPresetChooserItemSizeWidthFactor) {
        return;
    }

    m_d->gradientPresetChooserItemSizeWidthFactor = newFactor;
    updateGradientPresetChooser();
}

void KisGenericGradientEditor::setCompactGradientEditorMode(bool compact)
{
    m_d->compactGradientEditorMode = compact;
    updateGradientEditor();
}

bool KisGenericGradientEditor::event(QEvent *e)
{
    if (e->type() == QEvent::StyleChange) {
        updateWidgetSliderGradientPresetChooserItemSizeCustom();
    }
    return false;
}

void KisGenericGradientEditor::updateConvertGradientButton()
{
    bool isSegmentGradient = dynamic_cast<KisAutogradientEditor*>(m_d->widgetGradientEditor);

    m_d->buttonConvertGradient->setVisible(!m_d->compactMode && m_d->isConvertGradientButtonVisible);
    m_d->labelConvertGradientWarning->setVisible(!m_d->compactMode && m_d->isConvertGradientButtonVisible && isSegmentGradient);
    
    m_d->buttonConvertGradient->setText(
        m_d->gradient
        ?   isSegmentGradient
            ? i18nc("Convert the segment gradient to a stop gradient", "Convert to Stop Gradient")
            : i18nc("Convert the stop gradient to a segment gradient", "Convert to Segment Gradient")
        :   i18nc("A gradient wasn't set in the generic gradient editor", "No Gradient Set")
    );

    m_d->buttonConvertGradient->setEnabled(m_d->gradient);
}

void KisGenericGradientEditor::updateUpdateGradientButton()
{
    m_d->buttonUpdateGradient->setVisible(!m_d->compactMode && m_d->isUpdateGradientButtonVisible);
    KoResource *selectedGradient = m_d->widgetGradientPresetChooser->currentResource();
    bool sameKind =
        (dynamic_cast<KoStopGradient*>(selectedGradient) && dynamic_cast<KoStopGradient*>(m_d->gradient.data())) ||
        (dynamic_cast<KoSegmentGradient*>(selectedGradient) && dynamic_cast<KoSegmentGradient*>(m_d->gradient.data()));
    m_d->buttonUpdateGradient->setEnabled(
        m_d->gradient && selectedGradient && sameKind &&
        !selectedGradient->permanent()
    );
}

void KisGenericGradientEditor::updateAddGradientButton()
{
    m_d->buttonAddGradient->setVisible(!m_d->compactMode && m_d->isAddGradientButtonVisible);
    m_d->buttonAddGradient->setEnabled(m_d->gradient && !m_d->gradient->name().isEmpty());
}

void KisGenericGradientEditor::updateGradientPresetChooser()
{
    m_d->widgetGradientPresetChooser->showButtons(!m_d->compactGradientPresetChooserMode);
    m_d->widgetGradientPresetChooser->showTaggingBar(!m_d->compactGradientPresetChooserMode);
    m_d->widgetGradientPresetChooser->setViewModeButtonVisible(!m_d->compactGradientPresetChooserMode);

    updateGradientPresetChooserIcons();

    {
        int margin = !m_d->useGradientPresetChooserPopUp || m_d->compactGradientPresetChooserMode ? 0 : 10;
        m_d->widgetGradientPresetChooser->setContentsMargins(margin, margin, margin, margin);

        if (m_d->useGradientPresetChooserPopUp) {
            if (!m_d->widgetToolButtonGradientPresetChooser->children().contains(m_d->widgetGradientPresetChooser)) {
                layout()->removeWidget(m_d->widgetGradientPresetChooser);
                m_d->widgetToolButtonGradientPresetChooser->layout()->addWidget(m_d->widgetGradientPresetChooser);
            }
            m_d->widgetGradientPresetChooser->setMinimumSize(
                m_d->compactGradientPresetChooserMode ? 300 : 320,
                m_d->compactGradientPresetChooserMode ? 300 : 320
            );
            m_d->widgetGradientPresetChooser->adjustSize();
        } else {
            if (!children().contains(m_d->widgetGradientPresetChooser)) {
                m_d->widgetToolButtonGradientPresetChooser->layout()->removeWidget(m_d->widgetGradientPresetChooser);
                static_cast<QVBoxLayout*>(layout())->insertWidget(0, m_d->widgetGradientPresetChooser, 1);
            }
            m_d->widgetGradientPresetChooser->setMinimumSize(
                0,
                m_d->compactGradientPresetChooserMode ? 100 : 200
            );
        }
    }

    m_d->toolButtonGradientPresetChooser->setVisible(
        !m_d->compactMode &&
        m_d->isGradientPresetChooserVisible &&
        m_d->useGradientPresetChooserPopUp
    );
    m_d->toolButtonGradientPresetChooserOptions->setVisible(
        !m_d->compactMode &&
        m_d->isGradientPresetChooserVisible &&
        m_d->isGradientPresetChooserOptionsButtonVisible
    );
    m_d->actionUseGradientPresetChooserPopUp->setChecked(m_d->useGradientPresetChooserPopUp);
    m_d->actionCompactGradientPresetChooserMode->setChecked(m_d->compactGradientPresetChooserMode);
    if (m_d->gradientPresetChooserViewMode == GradientPresetChooserViewMode_Icon) {
        m_d->actionGradientPresetChooserViewModeIcon->setChecked(true);
    } else {
        m_d->actionGradientPresetChooserViewModeList->setChecked(true);
    }
    if (m_d->gradientPresetChooserItemSize == GradientPresetChooserItemSize_Small) {
        m_d->actionGradientPresetChooserItemSizeSmall->setChecked(true);
    } else if (m_d->gradientPresetChooserItemSize == GradientPresetChooserItemSize_Medium) {
        m_d->actionGradientPresetChooserItemSizeMedium->setChecked(true);
    } else if (m_d->gradientPresetChooserItemSize == GradientPresetChooserItemSize_Large) {
        m_d->actionGradientPresetChooserItemSizeLarge->setChecked(true);
    } else {
        m_d->actionGradientPresetChooserItemSizeCustom->setChecked(true);
    }
    {
        KisSignalsBlocker signalsBlocker(m_d->sliderGradientPresetChooserItemSizeCustom);
        m_d->sliderGradientPresetChooserItemSizeCustom->setValue(m_d->gradientPresetChooserItemSizeCustom);
    }
}

void KisGenericGradientEditor::updateGradientPresetChooserIcons()
{
    int rowHeight, columnWidth;
    if (m_d->gradientPresetChooserItemSize == GradientPresetChooserItemSize_Small) {
        rowHeight = m_d->gradientPresetChooserItemSizeSmall;
    } else if (m_d->gradientPresetChooserItemSize == GradientPresetChooserItemSize_Medium) {
        rowHeight = m_d->gradientPresetChooserItemSizeMedium;
    } else if (m_d->gradientPresetChooserItemSize == GradientPresetChooserItemSize_Large) {
        rowHeight = m_d->gradientPresetChooserItemSizeLarge;
    } else {
        rowHeight = m_d->gradientPresetChooserItemSizeCustom;
    }

    int nColumns;
    const int scrollbarWidth = style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    const int frameWidth = m_d->widgetGradientPresetChooser->itemView()->frameWidth();
    const int itemViewWidth = m_d->widgetGradientPresetChooser->itemView()->width();
    if (m_d->gradientPresetChooserViewMode == GradientPresetChooserViewMode_Icon) {
        columnWidth = static_cast<int>(qRound(rowHeight * m_d->gradientPresetChooserItemSizeWidthFactor));
        nColumns = (itemViewWidth - frameWidth - scrollbarWidth) / columnWidth;
    } else {
        columnWidth = itemViewWidth - frameWidth - scrollbarWidth;
        nColumns = 1;
    }

    m_d->widgetGradientPresetChooser->setRowHeight(rowHeight);
    m_d->widgetGradientPresetChooser->setColumnWidth(columnWidth);
    m_d->widgetGradientPresetChooser->setColumnCount(nColumns);
}

void KisGenericGradientEditor::updateGradientEditor()
{
    if (dynamic_cast<KisStopGradientEditor*>(m_d->widgetGradientEditor)) {
        dynamic_cast<KisStopGradientEditor*>(m_d->widgetGradientEditor)->setCompactMode(m_d->compactGradientEditorMode);
    } else if (dynamic_cast<KisAutogradientEditor*>(m_d->widgetGradientEditor)) {
        dynamic_cast<KisAutogradientEditor*>(m_d->widgetGradientEditor)->setCompactMode(m_d->compactGradientEditorMode);
    }
}

void KisGenericGradientEditor::updateWidgetSliderGradientPresetChooserItemSizeCustom()
{
    const int marginSize = style()->pixelMetric(QStyle::PM_ButtonMargin);
    const int indicatorSize = style()->pixelMetric(QStyle::PM_ExclusiveIndicatorWidth);
    const int spacingSize = style()->pixelMetric(QStyle::PM_RadioButtonLabelSpacing);
    m_d->widgetSliderGradientPresetChooserItemSizeCustom->layout()->setContentsMargins(
        indicatorSize + spacingSize, marginSize, marginSize, marginSize
    );
}

void KisGenericGradientEditor::on_buttonConvertGradient_clicked()
{
    if (dynamic_cast<KoStopGradient*>(m_d->gradient.data())) {
        setGradient(KoAbstractGradientSP(KisGradientConversion::toSegmentGradient(m_d->gradient.data())));
    } else if (m_d->gradient.dynamicCast<KoSegmentGradient>()) {
        setGradient(
            KoAbstractGradientSP(
                KisGradientConversion::toStopGradient(m_d->gradient.data(), m_d->foregroundColor, m_d->backgroundColor)
            )
        );
    }
}

void KisGenericGradientEditor::on_buttonUpdateGradient_clicked()
{
    if (!m_d->gradient || !m_d->gradient->valid()) {
        return;
    }

    KoAbstractGradient *selectedGradient =
        static_cast<KoAbstractGradient*>(m_d->widgetGradientPresetChooser->currentResource());
    if (!selectedGradient || selectedGradient->permanent()) {
        return;
    }

    if (dynamic_cast<KoStopGradient*>(selectedGradient)) {
        if (!dynamic_cast<KoStopGradient*>(m_d->gradient.data())) {
            return;
        }
        static_cast<KoStopGradient*>(selectedGradient)->setStops(
            static_cast<KoStopGradient*>(m_d->gradient.data())->stops()
        );
    } else if (dynamic_cast<KoSegmentGradient*>(selectedGradient)) {
        if (!dynamic_cast<KoSegmentGradient*>(m_d->gradient.data())) {
            return;
        }
        static_cast<KoSegmentGradient*>(selectedGradient)->setSegments(
            static_cast<KoSegmentGradient*>(m_d->gradient.data())->segments()
        );
    } else {
        return;
    }

    selectedGradient->setName(m_d->gradient->name());

    KoResourceServer<KoAbstractGradient> *gradientServer =
        KoResourceServerProvider::instance()->gradientServer();
    gradientServer->updateResource(selectedGradient);
}

void KisGenericGradientEditor::on_buttonAddGradient_clicked()
{
    if (!m_d->gradient || !m_d->gradient->valid() || m_d->gradient->name().isEmpty()) {
        return;
    }

    KoResourceServer<KoAbstractGradient> *gradientServer = KoResourceServerProvider::instance()->gradientServer();
    // Make a clone so that the current gradient can be changed without
    // modifying the resource
    KoAbstractGradient *gradient = m_d->gradient->clone();

    // Construct an unique name
    QDir saveLocation(gradientServer->saveLocation());
    QString uniqueName = gradient->name();
    int suffixId = 1;
    while (!saveLocation.entryList(QStringList() << (uniqueName + ".*")).isEmpty()) {
        uniqueName = gradient->name() + QString(" (%1)").arg(suffixId);
        ++suffixId;
    }
    
    gradient->setName(uniqueName);
    gradient->setFilename(gradientServer->saveLocation() + uniqueName + gradient->defaultFileExtension());

    gradientServer->addResource(gradient);
}

void KisGenericGradientEditor::on_widgetGradientPresetChooser_resourceClicked(KoResource *resource)
{
    // Make a clone to prevent changing the resource
    setGradient(KoAbstractGradientSP(static_cast<KoAbstractGradient*>(resource)->clone()));
}

void KisGenericGradientEditor::on_widgetGradientPresetChooserItemView_sigSizeChanged()
{
    updateGradientPresetChooserIcons();
}

void KisGenericGradientEditor::on_actionGroupGradientPresetChooserViewMode_triggered(QAction *triggeredAction)
{
    if (triggeredAction == m_d->actionGradientPresetChooserViewModeIcon) {
        setGradientPresetChooserViewMode(GradientPresetChooserViewMode_Icon);
    } else {
        setGradientPresetChooserViewMode(GradientPresetChooserViewMode_List);
    }
}

void KisGenericGradientEditor::on_actionGroupGradientPresetChooserItemSize_triggered(QAction *triggeredAction)
{
    if (triggeredAction == m_d->actionGradientPresetChooserItemSizeSmall) {
        setGradientPresetChooserItemSize(GradientPresetChooserItemSize_Small);
    } else if (triggeredAction == m_d->actionGradientPresetChooserItemSizeMedium) {
        setGradientPresetChooserItemSize(GradientPresetChooserItemSize_Medium);
    } else if (triggeredAction == m_d->actionGradientPresetChooserItemSizeLarge) {
        setGradientPresetChooserItemSize(GradientPresetChooserItemSize_Large);
    } else {
        setGradientPresetChooserItemSize(GradientPresetChooserItemSize_Custom);
    }
}

void KisGenericGradientEditor::on_sliderGradientPresetChooserItemSizeCustom_valueChanged(int newValue)
{
    m_d->gradientPresetChooserItemSize = GradientPresetChooserItemSize_Custom;
    setGradientPresetChooserItemSizeCustom(newValue);
}

void KisGenericGradientEditor::on_widgetGradientEditor_sigGradientChanged()
{
    updateUpdateGradientButton();
    updateAddGradientButton();
    emit sigGradientChanged();
}
