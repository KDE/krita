/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QToolButton>
#include <QWidgetAction>
#include <QSlider>

#include <KoStopGradient.h>
#include <KoSegmentGradient.h>
#include <KisStopGradientEditor.h>
#include <KisSegmentGradientEditor.h>
#include <KisGradientChooser.h>
#include <kis_icon_utils.h>
#include <KisGradientConversion.h>
#include <kis_signals_blocker.h>
#include <KoResourceServerProvider.h>
#include <KisResourceItemChooser.h>

#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include "KisGenericGradientEditor.h"

class Q_DECL_HIDDEN KisGenericGradientEditor::Private
{
public:
    KoAbstractGradientSP gradient;
    KoCanvasResourcesInterfaceSP canvasResourcesInterface;

    QPushButton *buttonConvertGradient;
    QPushButton *buttonUpdateGradient;
    QPushButton *buttonAddGradient;
    QLabel *labelConvertGradientWarning;
    KisGradientChooser *widgetGradientPresetChooser;
    QToolButton *toolButtonGradientPresetChooser;
    QWidget *widgetToolButtonGradientPresetChooser;
    QToolButton *toolButtonGradientPresetChooserOptions;
    QAction *actionUseGradientPresetChooserPopUp;
    QAction *actionCompactGradientPresetChooserMode;
    QWidget *widgetGradientEditor;

    bool compactMode;
    bool isConvertGradientButtonVisible;
    bool isUpdateGradientButtonVisible;
    bool isAddGradientButtonVisible;
    bool isGradientPresetChooserVisible;
    bool isGradientPresetChooserOptionsButtonVisible;
    bool useGradientPresetChooserPopUp;
    bool compactGradientPresetChooserMode;
    bool compactGradientEditorMode;
};

KisGenericGradientEditor::KisGenericGradientEditor(QWidget* parent)
    : QWidget(parent)
    , m_d(new Private)
{
    m_d->gradient = nullptr;
    m_d->canvasResourcesInterface = nullptr;
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

    m_d->widgetGradientPresetChooser = new KisGradientChooser(this);
    m_d->widgetGradientPresetChooser->setNameLabelVisible(false);
    m_d->widgetGradientPresetChooser->setEditOptionsVisible(false);

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
    m_d->toolButtonGradientPresetChooserOptions->setIcon(KisIconUtils::loadIcon("view-choose"));
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
    m_d->compactGradientEditorMode = false;

    updateConvertGradientButton();
    updateUpdateGradientButton();
    updateAddGradientButton();
    updateGradientPresetChooser();
    updateGradientEditor();

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
        SIGNAL(resourceClicked(KoResourceSP)),
        this,
        SLOT(on_widgetGradientPresetChooser_resourceClicked(KoResourceSP))
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
}

KisGenericGradientEditor::~KisGenericGradientEditor()
{}

void KisGenericGradientEditor::loadUISettings(const QString &prefix)
{
    KConfigGroup configGroup(KSharedConfig::openConfig(), "GenericGradientEditor");
    const QString pfx = prefix.isEmpty() ? QString("global/") : prefix + "/";

    m_d->useGradientPresetChooserPopUp = configGroup.readEntry(pfx + "useGradientPresetChooserPopUp", m_d->useGradientPresetChooserPopUp);
    m_d->compactGradientPresetChooserMode = configGroup.readEntry(pfx + "compactGradientPresetChooserMode", m_d->compactGradientPresetChooserMode);
    
    updateGradientPresetChooser();
}

void KisGenericGradientEditor::saveUISettings(const QString &prefix)
{
    KConfigGroup configGroup(KSharedConfig::openConfig(), "GenericGradientEditor");
    const QString pfx = prefix.isEmpty() ? QString("global/") : prefix + "/";

    configGroup.writeEntry(pfx + "useGradientPresetChooserPopUp", m_d->useGradientPresetChooserPopUp);
    configGroup.writeEntry(pfx + "compactGradientPresetChooserMode", m_d->compactGradientPresetChooserMode);
}

KoAbstractGradientSP KisGenericGradientEditor::gradient() const
{
    if (m_d->gradient) {
        return m_d->gradient->clone().dynamicCast<KoAbstractGradient>();
    }
    return nullptr;
}

KoCanvasResourcesInterfaceSP KisGenericGradientEditor::canvasResourcesInterface() const
{
    return m_d->canvasResourcesInterface;
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

    m_d->gradient = newGradient->clone().staticCast<KoAbstractGradient>();
    if (m_d->canvasResourcesInterface) {
        m_d->gradient->updateVariableColors(m_d->canvasResourcesInterface);
    }
    
    QWidget *newGradientEditorWidget = nullptr;
    if (m_d->gradient.dynamicCast<KoStopGradient>()) {
        if (dynamic_cast<KisStopGradientEditor*>(m_d->widgetGradientEditor)) {
            dynamic_cast<KisStopGradientEditor*>(m_d->widgetGradientEditor)->setGradient(m_d->gradient.dynamicCast<KoStopGradient>());
        } else {
            newGradientEditorWidget =
                new KisStopGradientEditor(m_d->gradient.dynamicCast<KoStopGradient>(), nullptr, "", "", m_d->canvasResourcesInterface);
        }
    } else if (m_d->gradient.dynamicCast<KoSegmentGradient>()) {
        if (dynamic_cast<KisSegmentGradientEditor*>(m_d->widgetGradientEditor)) {
            dynamic_cast<KisSegmentGradientEditor*>(m_d->widgetGradientEditor)->setGradient(m_d->gradient.dynamicCast<KoSegmentGradient>());
        } else {
            newGradientEditorWidget =
                new KisSegmentGradientEditor(m_d->gradient.dynamicCast<KoSegmentGradient>(), nullptr, "", "", m_d->canvasResourcesInterface);
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
            dynamic_cast<KisSegmentGradientEditor*>(m_d->widgetGradientEditor)->setCompactMode(m_d->compactGradientPresetChooserMode);
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

void KisGenericGradientEditor::setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP newCanvasResourcesInterface)
{
    m_d->canvasResourcesInterface = newCanvasResourcesInterface;

    if (dynamic_cast<KisStopGradientEditor*>(m_d->widgetGradientEditor)) {
        dynamic_cast<KisStopGradientEditor*>(m_d->widgetGradientEditor)->setCanvasResourcesInterface(m_d->canvasResourcesInterface);
    } else if (dynamic_cast<KisSegmentGradientEditor*>(m_d->widgetGradientEditor)) {
        dynamic_cast<KisSegmentGradientEditor*>(m_d->widgetGradientEditor)->setCanvasResourcesInterface(m_d->canvasResourcesInterface);
    }

    m_d->widgetGradientPresetChooser->setCanvasResourcesInterface(newCanvasResourcesInterface);
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

void KisGenericGradientEditor::setCompactGradientEditorMode(bool compact)
{
    m_d->compactGradientEditorMode = compact;
    updateGradientEditor();
}

void KisGenericGradientEditor::updateConvertGradientButton()
{
    bool isSegmentGradient = dynamic_cast<KisSegmentGradientEditor*>(m_d->widgetGradientEditor);

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
    KoResourceSP selectedGradient = m_d->widgetGradientPresetChooser->currentResource();
    m_d->buttonUpdateGradient->setEnabled(
        m_d->gradient && selectedGradient &&
        m_d->gradient->resourceId() == selectedGradient->resourceId() &&
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
    m_d->widgetGradientPresetChooser->resourceItemChooser()->showButtons(!m_d->compactGradientPresetChooserMode);
    m_d->widgetGradientPresetChooser->resourceItemChooser()->setStoragePopupButtonVisible(!m_d->compactGradientPresetChooserMode);
    m_d->widgetGradientPresetChooser->resourceItemChooser()->showTaggingBar(!m_d->compactGradientPresetChooserMode);
    m_d->widgetGradientPresetChooser->resourceItemChooser()->setViewModeButtonVisible(!m_d->compactGradientPresetChooserMode);

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
            m_d->widgetGradientPresetChooser->show();
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
}

void KisGenericGradientEditor::updateGradientEditor()
{
    if (dynamic_cast<KisStopGradientEditor*>(m_d->widgetGradientEditor)) {
        dynamic_cast<KisStopGradientEditor*>(m_d->widgetGradientEditor)->setCompactMode(m_d->compactGradientEditorMode);
    } else if (dynamic_cast<KisSegmentGradientEditor*>(m_d->widgetGradientEditor)) {
        dynamic_cast<KisSegmentGradientEditor*>(m_d->widgetGradientEditor)->setCompactMode(m_d->compactGradientEditorMode);
    }
}

void KisGenericGradientEditor::on_buttonConvertGradient_clicked()
{
    if (m_d->gradient.dynamicCast<KoStopGradient>()) {
        setGradient(KisGradientConversion::toSegmentGradient(m_d->gradient));
    } else if (m_d->gradient.dynamicCast<KoSegmentGradient>()) {
        setGradient(KisGradientConversion::toStopGradient(m_d->gradient));
    }
}

void KisGenericGradientEditor::on_buttonUpdateGradient_clicked()
{
    if (!m_d->gradient || !m_d->gradient->valid() || !(m_d->gradient->resourceId() > -1)) {
        return;
    }

    KoResourceSP selectedGradient = m_d->widgetGradientPresetChooser->currentResource();
    if (!selectedGradient ||
        !(m_d->gradient->resourceId() == selectedGradient->resourceId()) ||
        selectedGradient->permanent()) {
        return;
    }

    KoResourceServer<KoAbstractGradient> *gradientServer =
        KoResourceServerProvider::instance()->gradientServer();

    gradientServer->updateResource(m_d->gradient);
}

void KisGenericGradientEditor::on_buttonAddGradient_clicked()
{
    if (!m_d->gradient || !m_d->gradient->valid() || m_d->gradient->name().isEmpty()) {
        return;
    }

    KoResourceServer<KoAbstractGradient> *gradientServer =
        KoResourceServerProvider::instance()->gradientServer();
    
    gradientServer->addResource(m_d->gradient);
}

void KisGenericGradientEditor::on_widgetGradientPresetChooser_resourceClicked(KoResourceSP resource)
{
    setGradient(resource.dynamicCast<KoAbstractGradient>());
}

void KisGenericGradientEditor::on_widgetGradientEditor_sigGradientChanged()
{
    updateUpdateGradientButton();
    updateAddGradientButton();
    emit sigGradientChanged();
}
