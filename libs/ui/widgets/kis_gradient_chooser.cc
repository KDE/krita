/*
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "widgets/kis_gradient_chooser.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QSlider>
#include <QWidgetAction>
#include <QSet>

#include <klocalizedstring.h>
#include <resources/KoAbstractGradient.h>
#include <KoResource.h>
#include <resources/KoSegmentGradient.h>
#include <KisResourceItemListView.h>
#include <KisKineticScroller.h>
#include <KoStopGradient.h>
#include <KoColorSpaceRegistry.h>
#include <KisResourceItemChooser.h>
#include <KoResourceServerProvider.h>
#include <kis_icon.h>
#include <kis_config.h>
#include <kis_signals_blocker.h>
#include "kis_autogradient.h"
#include "kis_canvas_resource_provider.h"
#include "kis_stopgradient_editor.h"
#include "KisPopupButton.h"
#include <KisTagFilterResourceProxyModel.h>

#include <KoCanvasResourcesIds.h>
#include <KoCanvasResourcesInterface.h>

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <ksqueezedtextlabel.h>

class KisCustomGradientDialog : public KoDialog
{
    Q_OBJECT

public:
    KisCustomGradientDialog(KoAbstractGradientSP gradient,
                            QWidget *parent,
                            const char *name,
                            KoCanvasResourcesInterfaceSP canvasResourcesInterface);

private:
    QWidget * m_page;
};

KisCustomGradientDialog::KisCustomGradientDialog(KoAbstractGradientSP gradient,
                                                 QWidget *parent,
                                                 const char *name,
                                                 KoCanvasResourcesInterfaceSP canvasResourcesInterface)
    : KoDialog(parent, Qt::Dialog)
{
    setButtons(Ok|Cancel);
    setDefaultButton(Ok);
    setObjectName(name);
    setModal(false);

    connect(this, SIGNAL(okClicked()), this, SLOT(accept()));
    connect(this, SIGNAL(cancelClicked()), this, SLOT(reject()));

    KoStopGradientSP stopGradient = gradient.dynamicCast<KoStopGradient>();
    if (stopGradient) {
        m_page = new KisStopGradientEditor(stopGradient, this, "autogradient", i18n("Custom Stop Gradient"), canvasResourcesInterface);
    }
    else {
        KoSegmentGradientSP segmentedGradient = gradient.dynamicCast<KoSegmentGradient>();
        if (segmentedGradient) {
            m_page = new KisAutogradientEditor(segmentedGradient, this, "autogradient", i18n("Custom Segmented Gradient"), canvasResourcesInterface);
        }
    }
    setCaption(m_page->windowTitle());
    setMainWidget(m_page);
}

class Q_DECL_HIDDEN KisGradientChooser::Private : public QObject
{
    Q_OBJECT

public:
    struct ViewOptions
    {
        ViewMode viewMode{ViewMode_Icon};
        ItemSize itemSize{ItemSize_Medium};
        int itemSizeCustom{32};
        static constexpr int itemSizeSmall{32};
        static constexpr int itemSizeMedium{48};
        static constexpr int itemSizeLarge{64};
        static constexpr qreal itemSizeWidthFactor{2.0};
    };

    KisGradientChooser *q;
    KSqueezedTextLabel *labelName;
    KisResourceItemChooser * itemChooser;

    KoCanvasResourcesInterfaceSP canvasResourcesInterface;

    QWidget *containerEditWidgets;
    QToolButton *buttonAddGradient;
    QPushButton *buttonEditGradient;

    QAction *actionViewModeIcon;
    QAction *actionViewModeList;
    QAction *actionItemSizeSmall;
    QAction *actionItemSizeMedium;
    QAction *actionItemSizeLarge;
    QAction *actionItemSizeCustom;
    QSlider *sliderItemSizeCustom;
    QWidget *containerSliderItemSizeCustom;

    bool useGlobalViewOptions;
    static ViewOptions globalViewOptions;
    static QSet<KisGradientChooser*> globalChoosers;
    ViewOptions *viewOptions;

    bool isNameLabelVisible;
    bool areEditOptionsVisible;

public Q_SLOTS:
    void update(KoResourceSP resource);
    void addStopGradient();
    void addSegmentedGradient();
    void editGradient();
    void addGradient(KoAbstractGradientSP gradient, bool editGradient = false);

    void on_actionGroupViewMode_triggered(QAction *triggeredAction);
    void on_actionGroupItemSize_triggered(QAction *triggeredAction);
    void on_sliderItemSizeCustom_valueChanged(int newValue);

public:
    void updatePresetChooser(bool globalUpdate = true);
    void updatePresetChooserIcons();
    void updateContainerSliderItemSizeCustom();
};
KisGradientChooser::Private::ViewOptions KisGradientChooser::Private::globalViewOptions;
QSet<KisGradientChooser*> KisGradientChooser::Private::globalChoosers;

KisGradientChooser::KisGradientChooser(QWidget *parent, const char *name, bool useGlobalViewOptions)
    : QFrame(parent)
    , m_d(new Private)
{
    setObjectName(name);

    m_d->q = this;

    // Name label
    m_d->labelName = new KSqueezedTextLabel;
    m_d->labelName->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

    // Resource Item Chooser
    m_d->itemChooser = new KisResourceItemChooser(ResourceType::Gradients, false, this);
    m_d->itemChooser->showTaggingBar(true);
    m_d->itemChooser->setViewModeButtonVisible(true);

    // View menu
    QActionGroup *actionGroupViewMode = new QActionGroup(this);
    m_d->actionViewModeIcon = new QAction(this);
    m_d->actionViewModeIcon->setCheckable(true);
    m_d->actionViewModeIcon->setActionGroup(actionGroupViewMode);
    m_d->actionViewModeIcon->setText(
        i18nc("Set the gradient chooser to show icons instead of a list", "Icon view")
    );
    m_d->actionViewModeList = new QAction(this);
    m_d->actionViewModeList->setCheckable(true);
    m_d->actionViewModeList->setActionGroup(actionGroupViewMode);
    m_d->actionViewModeList->setText(
        i18nc("Set the gradient chooser to show a list instead of icons", "List view")
    );
    QAction *separatorViewMode1 = new QAction(this);
    separatorViewMode1->setSeparator(true);
    QActionGroup *actionGroupItemSize = new QActionGroup(this);
    m_d->actionItemSizeSmall = new QAction(this);
    m_d->actionItemSizeSmall->setCheckable(true);
    m_d->actionItemSizeSmall->setActionGroup(actionGroupItemSize);
    m_d->actionItemSizeSmall->setText(
        i18nc("Set the gradient chooser to show small items", "Small items")
    );
    m_d->actionItemSizeMedium = new QAction(this);
    m_d->actionItemSizeMedium->setCheckable(true);
    m_d->actionItemSizeMedium->setActionGroup(actionGroupItemSize);
    m_d->actionItemSizeMedium->setText(
        i18nc("Set the gradient chooser to show medium size items", "Medium size items")
    );
    m_d->actionItemSizeLarge = new QAction(this);
    m_d->actionItemSizeLarge->setCheckable(true);
    m_d->actionItemSizeLarge->setActionGroup(actionGroupItemSize);
    m_d->actionItemSizeLarge->setText(
        i18nc("Set the gradient chooser to show large items", "Large items")
    );
    m_d->actionItemSizeCustom = new QAction(this);
    m_d->actionItemSizeCustom->setCheckable(true);
    m_d->actionItemSizeCustom->setActionGroup(actionGroupItemSize);
    m_d->actionItemSizeCustom->setText(
        i18nc("Set the gradient chooser to show custom size items", "Custom size items")
    );
    m_d->sliderItemSizeCustom = new QSlider(this);
    m_d->sliderItemSizeCustom->setRange(16, 128);
    m_d->sliderItemSizeCustom->setOrientation(Qt::Horizontal);
    m_d->containerSliderItemSizeCustom = new QWidget(this);
    QVBoxLayout *layoutContainerSliderItemSizeCustom = new QVBoxLayout;
    layoutContainerSliderItemSizeCustom->addWidget(m_d->sliderItemSizeCustom);
    m_d->containerSliderItemSizeCustom->setLayout(layoutContainerSliderItemSizeCustom);
    QWidgetAction *widgetActionSliderItemSizeCustom = new QWidgetAction(this);
    widgetActionSliderItemSizeCustom->setDefaultWidget(m_d->containerSliderItemSizeCustom);
    QMenu* menu = new QMenu(this);
    menu->setStyleSheet("margin: 6px");
    menu->addActions(actionGroupViewMode->actions());
    menu->addAction(separatorViewMode1);
    menu->addActions(actionGroupItemSize->actions());
    menu->addAction(widgetActionSliderItemSizeCustom);
    KisPopupButton *toolButtonItemChooserViewMode = m_d->itemChooser->viewModeButton();
    toolButtonItemChooserViewMode->setPopupWidget(menu);


    // Edit widgets
    QHBoxLayout* layoutEditWidgets = new QHBoxLayout;
    layoutEditWidgets->setMargin(0);
    m_d->containerEditWidgets = new QWidget(this);

    m_d->buttonAddGradient = new QToolButton(this);
    m_d->buttonAddGradient->setText(i18n("Add..."));
    m_d->buttonAddGradient->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    layoutEditWidgets->addWidget(m_d->buttonAddGradient);

    QMenu *menuAddGradient = new QMenu(m_d->buttonAddGradient);
    QAction* addStopGradient = new QAction(i18n("Stop gradient"), this);
    menuAddGradient->addAction(addStopGradient);
    QAction* addSegmentedGradient = new QAction(i18n("Segmented gradient"), this);
    menuAddGradient->addAction(addSegmentedGradient);

    m_d->buttonAddGradient->setMenu(menuAddGradient);
    m_d->buttonAddGradient->setPopupMode(QToolButton::MenuButtonPopup);

    m_d->buttonEditGradient = new QPushButton();
    m_d->buttonEditGradient->setText(i18n("Edit..."));
    m_d->buttonEditGradient->setEnabled(false);
    layoutEditWidgets->addWidget(m_d->buttonEditGradient);

    layoutEditWidgets->addStretch(0);

    m_d->containerEditWidgets->setLayout(layoutEditWidgets);

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setObjectName("main layout");
    mainLayout->setMargin(0);
    mainLayout->addWidget(m_d->labelName);
    mainLayout->addWidget(m_d->itemChooser);
    mainLayout->addWidget(m_d->containerEditWidgets);
    setLayout(mainLayout);

    // Resource item chooser connections
    connect(m_d->itemChooser, SIGNAL(resourceSelected(KoResourceSP)),
            m_d.data(), SLOT(update(KoResourceSP)));
    connect(m_d->itemChooser, SIGNAL(resourceSelected(KoResourceSP)),
            this, SIGNAL(resourceSelected(KoResourceSP)));
    connect(m_d->itemChooser, SIGNAL(resourceClicked(KoResourceSP)),
            this, SIGNAL(resourceClicked(KoResourceSP)));

    // View menu
    connect(actionGroupViewMode, SIGNAL(triggered(QAction*)),
            m_d.data(), SLOT(on_actionGroupViewMode_triggered(QAction*)));
    connect(actionGroupItemSize, SIGNAL(triggered(QAction*)),
            m_d.data(), SLOT(on_actionGroupItemSize_triggered(QAction*)));
    connect(m_d->sliderItemSizeCustom, SIGNAL(valueChanged(int)),
            m_d.data(), SLOT(on_sliderItemSizeCustom_valueChanged(int)));
    // Edit widgets
    connect(m_d->buttonAddGradient, SIGNAL(clicked()), m_d.data(), SLOT(addStopGradient()));
    connect(addStopGradient, SIGNAL(triggered(bool)), m_d.data(), SLOT(addStopGradient()));
    connect(addSegmentedGradient, SIGNAL(triggered(bool)), m_d.data(), SLOT(addSegmentedGradient()));
    connect(m_d->buttonEditGradient, SIGNAL(clicked()), m_d.data(), SLOT(editGradient()));

    m_d->isNameLabelVisible = true;
    m_d->areEditOptionsVisible = true;
    slotUpdateIcons();

    m_d->useGlobalViewOptions = useGlobalViewOptions;
    if (useGlobalViewOptions) {
        m_d->viewOptions = &(m_d->globalViewOptions);
        if (m_d->globalChoosers.size() == 0) {
            loadViewSettings();
        }
        m_d->globalChoosers.insert(this);
    } else {
        m_d->viewOptions = new Private::ViewOptions;
    }
    m_d->updatePresetChooser();
    m_d->updateContainerSliderItemSizeCustom();
}

KisGradientChooser::~KisGradientChooser()
{
    if (m_d->useGlobalViewOptions) {
        m_d->globalChoosers.remove(this);
        if (m_d->globalChoosers.size() == 0) {
            saveViewSettings();
        }
    } else {
        delete m_d->viewOptions;
    }
}

void KisGradientChooser::setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    m_d->canvasResourcesInterface = canvasResourcesInterface;
}

KoCanvasResourcesInterfaceSP KisGradientChooser::canvasResourcesInterface() const
{
    return m_d->canvasResourcesInterface;
}

KoResourceSP KisGradientChooser::currentResource()
{
    return m_d->itemChooser->currentResource();
}

void KisGradientChooser::setCurrentResource(KoResourceSP resource)
{
    m_d->itemChooser->setCurrentResource(resource);
}

void KisGradientChooser::setCurrentItem(int row)
{
    m_d->itemChooser->setCurrentItem(row);
    if (currentResource())
        m_d->update(currentResource());
}

void KisGradientChooser::loadViewSettings(const QString &prefix)
{
    KConfigGroup configGroup(KSharedConfig::openConfig(), "GradientChooser");
    const QString pfx = prefix.isEmpty() ? QString("global/") : prefix + "/";

    QString strViewMode = configGroup.readEntry(pfx + "viewMode", QString());
    if (strViewMode == "icon") {
        m_d->viewOptions->viewMode = ViewMode_Icon;
    } else if (strViewMode == "list") {
        m_d->viewOptions->viewMode = ViewMode_List;
    }
    QString strItemSize = configGroup.readEntry(pfx + "itemSize", QString());
    if (strItemSize == "small") {
        m_d->viewOptions->itemSize = ItemSize_Small;
    } else if (strItemSize == "medium") {
        m_d->viewOptions->itemSize = ItemSize_Medium;
    } else if (strItemSize == "large") {
        m_d->viewOptions->itemSize = ItemSize_Large;
    } else if (strItemSize == "custom") {
        m_d->viewOptions->itemSize = ItemSize_Custom;
    }
    m_d->viewOptions->itemSizeCustom = configGroup.readEntry(pfx + "itemSizeCustom", m_d->viewOptions->itemSizeCustom);

    m_d->updatePresetChooser();
}

void KisGradientChooser::saveViewSettings(const QString &prefix)
{
    KConfigGroup configGroup(KSharedConfig::openConfig(), "GradientChooser");
    const QString pfx = prefix.isEmpty() ? QString("global/") : prefix + "/";

    if (m_d->viewOptions->viewMode == ViewMode_Icon) {
        configGroup.writeEntry(pfx + "viewMode", "icon");
    } else {
        configGroup.writeEntry(pfx + "viewMode", "list");
    }
    if (m_d->viewOptions->itemSize == ItemSize_Small) {
        configGroup.writeEntry(pfx + "itemSize", "small");
    } else if (m_d->viewOptions->itemSize == ItemSize_Medium) {
        configGroup.writeEntry(pfx + "itemSize", "medium");
    } else if (m_d->viewOptions->itemSize == ItemSize_Large) {
        configGroup.writeEntry(pfx + "itemSize", "large");
    } else {
        configGroup.writeEntry(pfx + "itemSize", "custom");
    }
    configGroup.writeEntry(pfx + "itemSizeCustom", m_d->viewOptions->itemSizeCustom);
}

KisGradientChooser::ViewMode KisGradientChooser::viewMode() const
{
    return m_d->viewOptions->viewMode;
}

KisGradientChooser::ItemSize KisGradientChooser::itemSize() const
{
    return m_d->viewOptions->itemSize;
}

int KisGradientChooser::itemSizeCustom() const
{
    return m_d->viewOptions->itemSizeCustom;
}

KisResourceItemChooser* KisGradientChooser::resourceItemChooser() const
{
    return m_d->itemChooser;
}

bool KisGradientChooser::isNameLabelVisible() const
{
    return m_d->isNameLabelVisible;
}

bool KisGradientChooser::areEditOptionsVisible() const
{
    return m_d->areEditOptionsVisible;
}

void KisGradientChooser::setViewMode(ViewMode newViewMode)
{
    m_d->viewOptions->viewMode = newViewMode;
    m_d->updatePresetChooser();
}

void KisGradientChooser::setItemSize(ItemSize newItemSize)
{
    m_d->viewOptions->itemSize = newItemSize;
    m_d->updatePresetChooser();
}

void KisGradientChooser::setItemSizeCustom(int newSize)
{
    if (newSize == m_d->viewOptions->itemSizeCustom) {
        return;
    }

    m_d->viewOptions->itemSizeCustom = newSize;
    m_d->updatePresetChooser();
}

void KisGradientChooser::setNameLabelVisible(bool newNameLabelVisible)
{
    m_d->isNameLabelVisible = newNameLabelVisible;
    m_d->labelName->setVisible(newNameLabelVisible);
}

void KisGradientChooser::setEditOptionsVisible(bool newEditOptionsVisible)
{
    m_d->areEditOptionsVisible = newEditOptionsVisible;
    m_d->containerEditWidgets->setVisible(newEditOptionsVisible);
}


void KisGradientChooser::slotUpdateIcons()
{
    if (m_d->buttonAddGradient && m_d->buttonEditGradient) {
        m_d->buttonAddGradient->setIcon(KisIconUtils::loadIcon("list-add"));
    }
}

bool KisGradientChooser::event(QEvent *e)
{
    if (e->type() == QEvent::StyleChange) {
        m_d->updateContainerSliderItemSizeCustom();
    }
    return false;
}

void KisGradientChooser::Private::update(KoResourceSP resource)
{
    KoAbstractGradientSP gradient = resource.staticCast<KoAbstractGradient>();
    labelName->setText(gradient ? i18n(gradient->name().toUtf8().data()) : "");
    buttonEditGradient->setEnabled(true);
}

void KisGradientChooser::Private::addStopGradient()
{
    KoStopGradientSP gradient(new KoStopGradient(""));

    QList<KoGradientStop> stops;
    stops << KoGradientStop(0.0, KoColor(QColor(250, 250, 0), KoColorSpaceRegistry::instance()->rgb8()), COLORSTOP)
        << KoGradientStop(1.0, KoColor(QColor(255, 0, 0, 255), KoColorSpaceRegistry::instance()->rgb8()), COLORSTOP);
    gradient->setType(QGradient::LinearGradient);
    gradient->setName(i18n("unnamed"));
    gradient->setStops(stops);
    addGradient(gradient);
}

void KisGradientChooser::Private::addSegmentedGradient()
{
    KoSegmentGradientSP gradient(new KoSegmentGradient(""));
    gradient->createSegment(INTERP_LINEAR, COLOR_INTERP_RGB, 0.0, 1.0, 0.5, Qt::black, Qt::white);
    gradient->setName(i18n("unnamed"));
    addGradient(gradient);
}

void KisGradientChooser::Private::addGradient(KoAbstractGradientSP gradient, bool editGradient)
{
    if (!gradient) return;

    KoResourceServer<KoAbstractGradient> *rserver = KoResourceServerProvider::instance()->gradientServer();
    QString saveLocation = rserver->saveLocation();

    gradient->updateVariableColors(canvasResourcesInterface);

    KisCustomGradientDialog dialog(gradient, q, "KisCustomGradientDialog", canvasResourcesInterface);

    QString oldname = gradient->name();

    bool shouldSaveResource = true;

    bool fileOverwriteAccepted = false;
    while(!fileOverwriteAccepted) {
        if (dialog.exec() == KoDialog::Accepted) {

            if (gradient->name().isEmpty()) {
                shouldSaveResource = false;
                break;
            }

            if (editGradient && oldname == gradient->name()) {
                fileOverwriteAccepted = true;
                continue;
            }

            const QFileInfo fileInfo(saveLocation + gradient->name().split(" ").join("_") + gradient->defaultFileExtension());
            if (fileInfo.exists()) {
                int res = QMessageBox::warning(q, i18nc("@title:window", "Name Already Exists")
                                               , i18n("The name '%1' already exists, do you wish to overwrite it?", gradient->name())
                                               , QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                if (res == QMessageBox::Yes) fileOverwriteAccepted = true;
            } else {
                fileOverwriteAccepted = true;
            }
        } else {
            shouldSaveResource = false;
            break;
        }
    }

    if (shouldSaveResource) {
        gradient->setFilename(gradient->name() + gradient->defaultFileExtension());
        gradient->setValid(true);
        rserver->addResource(gradient);
        itemChooser->tagFilterModel()->sort(Qt::DisplayRole);
        itemChooser->setCurrentResource(gradient);
    } else {
        // TODO: revert the changes made to the resource
    }
}

void KisGradientChooser::Private::editGradient()
{
    if (!(q->currentResource())) {
        itemChooser->setCurrentItem(0);
    }
    Q_ASSERT(q->currentResource());
    addGradient(q->currentResource().staticCast<KoAbstractGradient>(), true);
}

void KisGradientChooser::Private::on_actionGroupViewMode_triggered(QAction *triggeredAction)
{
    if (triggeredAction == actionViewModeIcon) {
        q->setViewMode(ViewMode_Icon);
    } else {
        q->setViewMode(ViewMode_List);
    }
}

void KisGradientChooser::Private::on_actionGroupItemSize_triggered(QAction *triggeredAction)
{
    if (triggeredAction == actionItemSizeSmall) {
        q->setItemSize(ItemSize_Small);
    } else if (triggeredAction == actionItemSizeMedium) {
        q->setItemSize(ItemSize_Medium);
    } else if (triggeredAction == actionItemSizeLarge) {
        q->setItemSize(ItemSize_Large);
    } else {
        q->setItemSize(ItemSize_Custom);
    }
}

void KisGradientChooser::Private::on_sliderItemSizeCustom_valueChanged(int newValue)
{
    q->setItemSizeCustom(newValue);
    q->setItemSize(ItemSize_Custom);
}

void KisGradientChooser::Private::updatePresetChooser(bool globalUpdate)
{
    if (useGlobalViewOptions && globalUpdate) {
        for (KisGradientChooser *chooser : globalChoosers) {
            chooser->m_d->updatePresetChooser(false);
        }
        return;
    }

    itemChooser->itemView()->setViewMode(
        viewOptions->viewMode == ViewMode_Icon
        ? QListView::IconMode
        : QListView::ListMode
    );

    updatePresetChooserIcons();

    if (viewOptions->viewMode == ViewMode_Icon) {
        actionViewModeIcon->setChecked(true);
    } else {
        actionViewModeList->setChecked(true);
    }
    if (viewOptions->itemSize == ItemSize_Small) {
        actionItemSizeSmall->setChecked(true);
    } else if (viewOptions->itemSize == ItemSize_Medium) {
        actionItemSizeMedium->setChecked(true);
    } else if (viewOptions->itemSize == ItemSize_Large) {
        actionItemSizeLarge->setChecked(true);
    } else {
        actionItemSizeCustom->setChecked(true);
    }
    {
        KisSignalsBlocker signalsBlocker(sliderItemSizeCustom);
        sliderItemSizeCustom->setValue(viewOptions->itemSizeCustom);
    }
}

void KisGradientChooser::Private::updatePresetChooserIcons()
{
    int itemHeight;
    if (viewOptions->itemSize == ItemSize_Small) {
        itemHeight = viewOptions->itemSizeSmall;
    } else if (viewOptions->itemSize == ItemSize_Medium) {
        itemHeight = viewOptions->itemSizeMedium;
    } else if (viewOptions->itemSize == ItemSize_Large) {
        itemHeight = viewOptions->itemSizeLarge;
    } else {
        itemHeight = viewOptions->itemSizeCustom;
    }
    itemChooser->setRowHeight(itemHeight);
    itemChooser->setColumnWidth(
        static_cast<int>(qRound(itemHeight * viewOptions->itemSizeWidthFactor))
    );
}

void KisGradientChooser::Private::updateContainerSliderItemSizeCustom()
{
    const int marginSize = q->style()->pixelMetric(QStyle::PM_ButtonMargin);
    const int indicatorSize = q->style()->pixelMetric(QStyle::PM_ExclusiveIndicatorWidth);
    const int spacingSize = q->style()->pixelMetric(QStyle::PM_RadioButtonLabelSpacing);
    containerSliderItemSizeCustom->layout()->setContentsMargins(
        indicatorSize + spacingSize, marginSize, marginSize, marginSize
    );
}

#include "kis_gradient_chooser.moc"
