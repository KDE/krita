/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_transform_config_widget.h"

#include <kis_icon.h>
#include "rotation_icons.h"
#include "kis_canvas2.h"
#include <KisSignalMapper.h>
#include "kis_liquify_properties.h"

#include "KisMainWindow.h"
#include "KisViewManager.h"
#include "kis_transform_utils.h"

#include <kstandardguiitem.h>


template<typename T> inline T sign(T x) {
    return x > 0 ? 1 : x == (T)0 ? 0 : -1;
}

const int KisToolTransformConfigWidget::DEFAULT_POINTS_PER_LINE = 3;


KisToolTransformConfigWidget::KisToolTransformConfigWidget(TransformTransactionProperties *transaction, KisCanvas2 *canvas, QWidget *parent)
    : QWidget(parent),
      m_transaction(transaction),
      m_notificationsBlocked(0),
      m_uiSlotsBlocked(0),
      m_configChanged(false)
{
    setupUi(this);
    flipXButton->setIcon(KisIconUtils::loadIcon("transform_icons_mirror_x"));
    flipYButton->setIcon(KisIconUtils::loadIcon("transform_icons_mirror_y"));
    rotateCWButton->setIcon(KisIconUtils::loadIcon("transform_icons_rotate_cw"));
    rotateCCWButton->setIcon(KisIconUtils::loadIcon("transform_icons_rotate_ccw"));

    // Granularity can only be specified in the power of 2's
    QStringList granularityValues{"4","8","16","32"};
    changeGranularity->addItems(granularityValues);
    changeGranularity->setCurrentIndex(1);
    granularityPreview->addItems(granularityValues);
    granularityPreview->setCurrentIndex(2);

    connect(changeGranularity,SIGNAL(currentIndexChanged(QString)),
            this,SLOT(slotGranularityChanged(QString)));
    connect(granularityPreview, SIGNAL(currentIndexChanged(QString)),
            this,SLOT(slotPreviewGranularityChanged(QString)));

    // Init Filter  combo
    cmbFilter->setIDList(KisFilterStrategyRegistry::instance()->listKeys());
    cmbFilter->setCurrent("Bicubic");
    cmbFilter->setToolTip(i18nc("@info:tooltip",
                                "<p>Select filtering mode:\n"
                                "<ul>"
                                "<li><b>Nearest neighbor</b> for pixel art. Does not produce new color.</li>"
                                "<li><b>Bilinear</b> for areas with uniform color to avoid artifacts.</li>"
                                "<li><b>Bicubic</b> for smoother results.</li>"
                                "<li><b>Lanczos3</b> for sharp results. May produce aerials.</li>"
                                "</ul></p>"));
    connect(cmbFilter, SIGNAL(activated(KoID)),
            this, SLOT(slotFilterChanged(KoID)));

    // Init Warp Type combo
    cmbWarpType->insertItem(KisWarpTransformWorker::AFFINE_TRANSFORM,i18n("Default (Affine)"));
    cmbWarpType->insertItem(KisWarpTransformWorker::RIGID_TRANSFORM,i18n("Strong (Rigid)"));
    cmbWarpType->insertItem(KisWarpTransformWorker::SIMILITUDE_TRANSFORM,i18n("Strongest (Similitude)"));
    cmbWarpType->setCurrentIndex(KisWarpTransformWorker::AFFINE_TRANSFORM);
    connect(cmbWarpType, SIGNAL(currentIndexChanged(int)), this, SLOT(slotWarpTypeChanged(int)));

    // Init Rotation Center buttons
    m_handleDir[0] = QPointF(1, 0);
    m_handleDir[1] = QPointF(1, -1);
    m_handleDir[2] = QPointF(0, -1);
    m_handleDir[3] = QPointF(-1, -1);
    m_handleDir[4] = QPointF(-1, 0);
    m_handleDir[5] = QPointF(-1, 1);
    m_handleDir[6] = QPointF(0, 1);
    m_handleDir[7] = QPointF(1, 1);
    m_handleDir[8] = QPointF(0, 0); // also add the center

    m_rotationCenterButtons = new QButtonGroup(0);
    // we set the ids to match m_handleDir
    m_rotationCenterButtons->addButton(middleRightButton, 0);
    m_rotationCenterButtons->addButton(topRightButton, 1);
    m_rotationCenterButtons->addButton(middleTopButton, 2);
    m_rotationCenterButtons->addButton(topLeftButton, 3);
    m_rotationCenterButtons->addButton(middleLeftButton, 4);
    m_rotationCenterButtons->addButton(bottomLeftButton, 5);
    m_rotationCenterButtons->addButton(middleBottomButton, 6);
    m_rotationCenterButtons->addButton(bottomRightButton, 7);
    m_rotationCenterButtons->addButton(centerButton, 8);

    QToolButton *nothingSelected = new QToolButton(0);
    nothingSelected->setCheckable(true);
    nothingSelected->setAutoExclusive(true);
    nothingSelected->hide(); // a convenient button for when no button is checked in the group
    m_rotationCenterButtons->addButton(nothingSelected, 9);


    // initialize values for free transform sliders
    shearXBox->setSuffix(QChar(Qt::Key_Percent));
    shearYBox->setSuffix(QChar(Qt::Key_Percent));
    shearXBox->setRange(-500, 500, 2);
    shearYBox->setRange(-500, 500, 2);
    shearXBox->setSingleStep(1);
    shearYBox->setSingleStep(1);
    shearXBox->setValue(0.0);
    shearYBox->setValue(0.0);


    translateXBox->setSuffix(i18n(" px"));
    translateYBox->setSuffix(i18n(" px"));
    translateXBox->setRange(-10000, 10000);
    translateYBox->setRange(-10000, 10000);


    scaleXBox->setRange(-10000, 10000);
    scaleYBox->setRange(-10000, 10000);
    scaleXBox->setValue(100.0);
    scaleYBox->setValue(100.0);

    m_scaleRatio = 1.0;


    aXBox->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);
    aYBox->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);
    aZBox->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);
    aXBox->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_MenuButton);
    aYBox->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_MenuButton);
    aZBox->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_MenuButton);

    cameraHeightBox->setRange(1, 20000, 2);

    connect(m_rotationCenterButtons, SIGNAL(buttonPressed(int)), this, SLOT(slotRotationCenterChanged(int)));
    connect(btnTransformAroundPivotPoint, SIGNAL(clicked(bool)), this, SLOT(slotTransformAroundRotationCenter(bool)));

    // Init Free Transform Values
    connect(scaleXBox, SIGNAL(valueChanged(int)), this, SLOT(slotSetScaleX(int)));
    connect(scaleYBox, SIGNAL(valueChanged(int)), this, SLOT(slotSetScaleY(int)));
    connect(shearXBox, SIGNAL(valueChanged(qreal)), this, SLOT(slotSetShearX(qreal)));
    connect(shearYBox, SIGNAL(valueChanged(qreal)), this, SLOT(slotSetShearY(qreal)));
    connect(translateXBox, SIGNAL(valueChanged(int)), this, SLOT(slotSetTranslateX(int)));
    connect(translateYBox, SIGNAL(valueChanged(int)), this, SLOT(slotSetTranslateY(int)));
    connect(aXBox, SIGNAL(angleChanged(qreal)), this, SLOT(slotSetAX(qreal)));
    connect(aYBox, SIGNAL(angleChanged(qreal)), this, SLOT(slotSetAY(qreal)));
    connect(aZBox, SIGNAL(angleChanged(qreal)), this, SLOT(slotSetAZ(qreal)));
    connect(cameraHeightBox, SIGNAL(valueChanged(qreal)), this, SLOT(slotSetCameraHeight(qreal)));
    connect(aspectButton, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(slotSetKeepAspectRatio(bool)));
    connect(flipXButton, SIGNAL(clicked(bool)), this, SLOT(slotFlipX()));
    connect(flipYButton, SIGNAL(clicked(bool)), this, SLOT(slotFlipY()));
    connect(rotateCWButton, SIGNAL(clicked(bool)), this, SLOT(slotRotateCW()));
    connect(rotateCCWButton, SIGNAL(clicked(bool)), this, SLOT(slotRotateCCW()));

    // toggle visibility of different free buttons
    connect(freeMoveRadioButton, SIGNAL(clicked(bool)), SLOT(slotTransformAreaVisible(bool)));
    connect(freeRotationRadioButton, SIGNAL(clicked(bool)), SLOT(slotTransformAreaVisible(bool)));
    connect(freeScaleRadioButton, SIGNAL(clicked(bool)), SLOT(slotTransformAreaVisible(bool)));
    connect(freeShearRadioButton, SIGNAL(clicked(bool)), SLOT(slotTransformAreaVisible(bool)));

    // only first group for free transform
    rotationGroup->hide();
    moveGroup->show();
    scaleGroup->hide();
    shearGroup->hide();



    // Init Warp Transform Values
    alphaBox->setSingleStep(0.1);
    alphaBox->setRange(0, 10, 1);

    connect(alphaBox, SIGNAL(valueChanged(qreal)), this, SLOT(slotSetWarpAlpha(qreal)));
    connect(densityBox, SIGNAL(valueChanged(int)), this, SLOT(slotSetWarpDensity(int)));

    connect(defaultRadioButton, SIGNAL(clicked(bool)), this, SLOT(slotWarpDefaultPointsButtonClicked(bool)));
    connect(customRadioButton, SIGNAL(clicked(bool)), this, SLOT(slotWarpCustomPointsButtonClicked(bool)));
    connect(lockUnlockPointsButton, SIGNAL(clicked()), this, SLOT(slotWarpLockPointsButtonClicked()));
    connect(resetPointsButton, SIGNAL(clicked()), this, SLOT(slotWarpResetPointsButtonClicked()));

    // Init Cage Transform Values
    cageTransformButtonGroup->setId(cageAddEditRadio, 0); // we need to set manually since Qt Designer generates negative by default
    cageTransformButtonGroup->setId(cageDeformRadio, 1);
    connect(cageTransformButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotCageOptionsChanged(int)));

    // Init Liquify Transform Values
    liquifySizeSlider->setRange(KisLiquifyProperties::minSize(),
                                KisLiquifyProperties::maxSize(), 2);
    liquifySizeSlider->setExponentRatio(4);
    liquifySizeSlider->setValue(60.0);
    connect(liquifySizeSlider, SIGNAL(valueChanged(qreal)), this, SLOT(liquifySizeChanged(qreal)));
    liquifySizeSlider->setToolTip(i18nc("@info:tooltip", "Size of the deformation brush"));

    liquifyAmountSlider->setRange(0.0, 1.0, 2);
    liquifyAmountSlider->setValue(0.05);
    connect(liquifyAmountSlider, SIGNAL(valueChanged(qreal)), this, SLOT(liquifyAmountChanged(qreal)));
    liquifyAmountSlider->setToolTip(i18nc("@info:tooltip", "Amount of the deformation you get"));

    liquifyFlowSlider->setRange(0.0, 1.0, 2);
    liquifyFlowSlider->setValue(1.0);
    connect(liquifyFlowSlider, SIGNAL(valueChanged(qreal)), this, SLOT(liquifyFlowChanged(qreal)));
    liquifyFlowSlider->setToolTip(i18nc("@info:tooltip", "When in non-buildup mode, shows how fast the deformation limit is reached."));

    buidupModeComboBox->setCurrentIndex(0); // set to build-up mode by default
    connect(buidupModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(liquifyBuildUpChanged(int)));
    buidupModeComboBox->setToolTip("<p>" + i18nc("@info:tooltip", "Switch between Build Up and Wash mode of painting. Build Up mode adds deformations one on top of the other without any limits. Wash mode gradually deforms the piece to the selected deformation level.") + "</p>");

    liquifySpacingSlider->setRange(0.0, 3.0, 2);
    liquifySizeSlider->setExponentRatio(3);
    liquifySpacingSlider->setSingleStep(0.01);
    liquifySpacingSlider->setValue(0.2);
    connect(liquifySpacingSlider, SIGNAL(valueChanged(qreal)), this, SLOT(liquifySpacingChanged(qreal)));
    liquifySpacingSlider->setToolTip(i18nc("@info:tooltip", "Space between two sequential applications of the deformation"));

    liquifySizePressureBox->setChecked(true);
    connect(liquifySizePressureBox, SIGNAL(toggled(bool)), this, SLOT(liquifySizePressureChanged(bool)));
    liquifySizePressureBox->setToolTip(i18nc("@info:tooltip", "Scale <b>Size</b> value according to current stylus pressure"));

    liquifyAmountPressureBox->setChecked(true);
    connect(liquifyAmountPressureBox, SIGNAL(toggled(bool)), this, SLOT(liquifyAmountPressureChanged(bool)));
    liquifyAmountPressureBox->setToolTip(i18nc("@info:tooltip", "Scale <b>Amount</b> value according to current stylus pressure"));

    liquifyReverseDirectionChk->setChecked(false);
    connect(liquifyReverseDirectionChk, SIGNAL(toggled(bool)), this, SLOT(liquifyReverseDirectionChanged(bool)));
    liquifyReverseDirectionChk->setToolTip(i18nc("@info:tooltip", "Reverse direction of the current deformation tool"));

    KisSignalMapper *liquifyModeMapper = new KisSignalMapper(this);
    connect(liquifyMove, SIGNAL(toggled(bool)), liquifyModeMapper, SLOT(map()));
    connect(liquifyScale, SIGNAL(toggled(bool)), liquifyModeMapper, SLOT(map()));
    connect(liquifyRotate, SIGNAL(toggled(bool)), liquifyModeMapper, SLOT(map()));
    connect(liquifyOffset, SIGNAL(toggled(bool)), liquifyModeMapper, SLOT(map()));
    connect(liquifyUndo, SIGNAL(toggled(bool)), liquifyModeMapper, SLOT(map()));
    liquifyModeMapper->setMapping(liquifyMove, (int)KisLiquifyProperties::MOVE);
    liquifyModeMapper->setMapping(liquifyScale, (int)KisLiquifyProperties::SCALE);
    liquifyModeMapper->setMapping(liquifyRotate, (int)KisLiquifyProperties::ROTATE);
    liquifyModeMapper->setMapping(liquifyOffset, (int)KisLiquifyProperties::OFFSET);
    liquifyModeMapper->setMapping(liquifyUndo, (int)KisLiquifyProperties::UNDO);
    connect(liquifyModeMapper, SIGNAL(mapped(int)), SLOT(slotLiquifyModeChanged(int)));

    liquifyMove->setToolTip(i18nc("@info:tooltip", "Move: drag the image along the brush stroke"));
    liquifyScale->setToolTip(i18nc("@info:tooltip", "Scale: grow/shrink image under cursor"));
    liquifyRotate->setToolTip(i18nc("@info:tooltip", "Rotate: twirl image under cursor"));
    liquifyOffset->setToolTip(i18nc("@info:tooltip", "Offset: shift the image to the right of the stroke direction"));
    liquifyUndo->setToolTip(i18nc("@info:tooltip", "Undo: erase actions of other tools"));

    // Connect all edit boxes to the Editing Finished signal
    connect(densityBox, SIGNAL(editingFinished()), this, SLOT(notifyEditingFinished()));



    // Connect other widget (not having editingFinished signal) to
    // the same slot. From Qt 4.6 onwards the sequence of the signal
    // delivery is definite.
    connect(cmbFilter, SIGNAL(activated(KoID)), this, SLOT(notifyEditingFinished()));
    connect(cmbWarpType, SIGNAL(currentIndexChanged(int)), this, SLOT(notifyEditingFinished()));
    connect(m_rotationCenterButtons, SIGNAL(buttonPressed(int)), this, SLOT(notifyEditingFinished()));
    connect(aspectButton, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(notifyEditingFinished()));

    connect(lockUnlockPointsButton, SIGNAL(clicked()), this, SLOT(notifyEditingFinished()));
    connect(resetPointsButton, SIGNAL(clicked()), this, SLOT(notifyEditingFinished()));

    connect(defaultRadioButton, SIGNAL(clicked(bool)), this, SLOT(notifyEditingFinished()));
    connect(customRadioButton, SIGNAL(clicked(bool)), this, SLOT(notifyEditingFinished()));

    // Liquify
    //
    // liquify brush options do not affect the image directly and are not
    // saved to undo, so we don't emit notifyEditingFinished() for them

    // Connect Apply/Reset buttons
    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(slotButtonBoxClicked(QAbstractButton*)));

    // Mesh. First set up, then connect.
    intNumRows->setRange(1, 999);
    intNumColumns->setRange(1, 999);

    connect(chkShowControlPoints, SIGNAL(toggled(bool)), this, SLOT(slotMeshShowHandlesChanged()));
    connect(chkSymmetricalHandles, SIGNAL(toggled(bool)), this, SLOT(slotMeshSymmetricalHandlesChanged()));
    connect(chkScaleHandles, SIGNAL(toggled(bool)), this, SLOT(slotMeshScaleHandlesChanged()));
    connect(intNumColumns, SIGNAL(valueChanged(int)), this, SLOT(slotMeshSizeChanged()));
    connect(intNumRows, SIGNAL(valueChanged(int)), this, SLOT(slotMeshSizeChanged()));
    connect(intNumColumns, SIGNAL(editingFinished()), this, SLOT(notifyEditingFinished()));
    connect(intNumRows, SIGNAL(editingFinished()), this, SLOT(notifyEditingFinished()));

    // Mode switch buttons
    connect(freeTransformButton, SIGNAL(clicked(bool)), this, SLOT(slotSetFreeTransformModeButtonClicked(bool)));
    connect(warpButton, SIGNAL(clicked(bool)), this, SLOT(slotSetWarpModeButtonClicked(bool)));
    connect(cageButton, SIGNAL(clicked(bool)), this, SLOT(slotSetCageModeButtonClicked(bool)));
    connect(perspectiveTransformButton, SIGNAL(clicked(bool)), this, SLOT(slotSetPerspectiveModeButtonClicked(bool)));
    connect(liquifyButton, SIGNAL(clicked(bool)), this, SLOT(slotSetLiquifyModeButtonClicked(bool)));
    connect(meshButton, SIGNAL(clicked(bool)), this, SLOT(slotSetMeshModeButtonClicked(bool)));


    tooBigLabelWidget->hide();

    connect(canvas->viewManager()->mainWindow(), SIGNAL(themeChanged()), SLOT(slotUpdateIcons()), Qt::UniqueConnection);
    slotUpdateIcons();

    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Apply), KStandardGuiItem::apply());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Reset), KStandardGuiItem::reset());
}

void KisToolTransformConfigWidget::slotUpdateIcons()
{
    freeTransformButton->setIcon(KisIconUtils::loadIcon("transform_icons_main"));
    warpButton->setIcon(KisIconUtils::loadIcon("transform_icons_warp"));
    cageButton->setIcon(KisIconUtils::loadIcon("transform_icons_cage"));
    perspectiveTransformButton->setIcon(KisIconUtils::loadIcon("transform_icons_perspective"));
    liquifyButton->setIcon(KisIconUtils::loadIcon("transform_icons_liquify_main"));
    meshButton->setIcon(KisIconUtils::loadIcon("transform_icons_mesh"));

    liquifyMove->setIcon(KisIconUtils::loadIcon("transform_icons_liquify_move"));
    liquifyScale->setIcon(KisIconUtils::loadIcon("transform_icons_liquify_resize"));
    liquifyRotate->setIcon(KisIconUtils::loadIcon("transform_icons_liquify_rotate"));
    liquifyOffset->setIcon(KisIconUtils::loadIcon("transform_icons_liquify_offset"));
    liquifyUndo->setIcon(KisIconUtils::loadIcon("transform_icons_liquify_erase"));



    middleRightButton->setIcon(KisIconUtils::loadIcon("arrow-right"));
    topRightButton->setIcon(KisIconUtils::loadIcon("arrow-topright"));
    middleTopButton->setIcon(KisIconUtils::loadIcon("arrow-up"));
    topLeftButton->setIcon(KisIconUtils::loadIcon("arrow-topleft"));
    middleLeftButton->setIcon(KisIconUtils::loadIcon("arrow-left"));
    bottomLeftButton->setIcon(KisIconUtils::loadIcon("arrow-downleft"));
    middleBottomButton->setIcon(KisIconUtils::loadIcon("arrow-down"));
    bottomRightButton->setIcon(KisIconUtils::loadIcon("arrow-downright"));

    btnTransformAroundPivotPoint->setIcon(KisIconUtils::loadIcon("pivot-point"));


    // pressure icons
    liquifySizePressureBox->setIcon(KisIconUtils::loadIcon("transform_icons_penPressure"));
    liquifyAmountPressureBox->setIcon(KisIconUtils::loadIcon("transform_icons_penPressure"));
}

void KisToolTransformConfigWidget::updateLiquifyControls()
{
    blockUiSlots();

    ToolTransformArgs *config = m_transaction->currentConfig();
    KisLiquifyProperties *props =
        config->liquifyProperties();

    const bool useWashMode = props->useWashMode();

    liquifySizeSlider->setValue(props->size());
    liquifyAmountSlider->setValue(props->amount());
    liquifyFlowSlider->setValue(props->flow());
    buidupModeComboBox->setCurrentIndex(useWashMode);

    liquifySpacingSlider->setValue(props->spacing());
    liquifySizePressureBox->setChecked(props->sizeHasPressure());
    liquifyAmountPressureBox->setChecked(props->amountHasPressure());
    liquifyReverseDirectionChk->setChecked(props->reverseDirection());


    KisLiquifyProperties::LiquifyMode mode =
        static_cast<KisLiquifyProperties::LiquifyMode>(props->mode());

    bool canInverseDirection =
        mode != KisLiquifyProperties::UNDO;

    bool canUseWashMode = mode != KisLiquifyProperties::UNDO;

    liquifyReverseDirectionChk->setEnabled(canInverseDirection);
    liquifyFlowSlider->setEnabled(canUseWashMode && useWashMode);
    buidupModeComboBox->setEnabled(canUseWashMode);

    const qreal maxAmount = canUseWashMode ? 5.0 : 1.0;
    liquifyAmountSlider->setRange(0.0, maxAmount, 2);

    unblockUiSlots();
}

void KisToolTransformConfigWidget::slotLiquifyModeChanged(int value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();

    KisLiquifyProperties *props =
        config->liquifyProperties();

    KisLiquifyProperties::LiquifyMode mode =
        static_cast<KisLiquifyProperties::LiquifyMode>(value);

    if (mode == props->mode()) return;

    props->setMode(mode);
    props->loadMode();

    updateLiquifyControls();

    notifyConfigChanged();
}

void KisToolTransformConfigWidget::liquifySizeChanged(qreal value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();
    KisLiquifyProperties *props =
        config->liquifyProperties();

    props->setSize(value);
    notifyConfigChanged();
}

void KisToolTransformConfigWidget::liquifyAmountChanged(qreal value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();
    KisLiquifyProperties *props =
        config->liquifyProperties();

    props->setAmount(value);
    notifyConfigChanged();
}

void KisToolTransformConfigWidget::liquifyFlowChanged(qreal value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();
    KisLiquifyProperties *props =
        config->liquifyProperties();

    props->setFlow(value);
    notifyConfigChanged();
}

void KisToolTransformConfigWidget::liquifyBuildUpChanged(int value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();
    KisLiquifyProperties *props =
        config->liquifyProperties();

    props->setUseWashMode(value); // 0 == build up mode / 1 == wash mode

    notifyConfigChanged();

    // we need to enable/disable flow slider
    updateLiquifyControls();
}

void KisToolTransformConfigWidget::liquifySpacingChanged(qreal value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();
    KisLiquifyProperties *props =
        config->liquifyProperties();

    props->setSpacing(value);
    notifyConfigChanged();
}

void KisToolTransformConfigWidget::liquifySizePressureChanged(bool value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();
    KisLiquifyProperties *props =
        config->liquifyProperties();

    props->setSizeHasPressure(value);
    notifyConfigChanged();
}

void KisToolTransformConfigWidget::liquifyAmountPressureChanged(bool value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();
    KisLiquifyProperties *props =
        config->liquifyProperties();

    props->setAmountHasPressure(value);
    notifyConfigChanged();
}

void KisToolTransformConfigWidget::liquifyReverseDirectionChanged(bool value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();
    KisLiquifyProperties *props =
        config->liquifyProperties();

    props->setReverseDirection(value);
    notifyConfigChanged();
}

void KisToolTransformConfigWidget::updateConfig(const ToolTransformArgs &config)
{
    blockUiSlots();
 
    if (config.mode() == ToolTransformArgs::FREE_TRANSFORM ||
        config.mode() == ToolTransformArgs::PERSPECTIVE_4POINT) {

        quickTransformGroup->setEnabled(config.mode() == ToolTransformArgs::FREE_TRANSFORM);

        stackedWidget->setCurrentIndex(0);

        bool freeTransformIsActive = config.mode() == ToolTransformArgs::FREE_TRANSFORM;

        if (freeTransformIsActive)
        {
            freeTransformButton->setChecked(true);
        }
        else
        {
            perspectiveTransformButton->setChecked(true);
        }

        aXBox->setEnabled(freeTransformIsActive);
        aYBox->setEnabled(freeTransformIsActive);
        aZBox->setEnabled(freeTransformIsActive);
        cameraHeightBox->setEnabled(freeTransformIsActive);
        freeRotationRadioButton->setEnabled(freeTransformIsActive);

        scaleXBox->setValue(config.scaleX() * 100.);
        scaleYBox->setValue(config.scaleY() * 100.);
        shearXBox->setValue(config.shearX() * 100.);
        shearYBox->setValue(config.shearY() * 100.);

        const QPointF anchorPoint = config.originalCenter() + config.rotationCenterOffset();
        const KisTransformUtils::MatricesPack m(config);
        const QPointF anchorPointView = m.finalTransform().map(anchorPoint);

        translateXBox->setValue(anchorPointView.x());
        translateYBox->setValue(anchorPointView.y());

        aXBox->setAngle(normalizeAngleDegrees(kisRadiansToDegrees(config.aX())));
        aYBox->setAngle(normalizeAngleDegrees(kisRadiansToDegrees(config.aY())));
        aZBox->setAngle(normalizeAngleDegrees(kisRadiansToDegrees(config.aZ())));
        cameraHeightBox->setValue(config.cameraPos().z());
        aspectButton->setKeepAspectRatio(config.keepAspectRatio());
        cmbFilter->setCurrent(config.filterId());

        QPointF pt = m_transaction->currentConfig()->rotationCenterOffset();
        pt.rx() /= m_transaction->originalHalfWidth();
        pt.ry() /= m_transaction->originalHalfHeight();

        for (int i = 0; i < 9; i++) {
            if (qFuzzyCompare(m_handleDir[i].x(), pt.x()) &&
                qFuzzyCompare(m_handleDir[i].y(), pt.y())) {

                m_rotationCenterButtons->button(i)->setChecked(true);
                break;
            }
        }

        btnTransformAroundPivotPoint->setChecked(config.transformAroundRotationCenter());

    } else if (config.mode() == ToolTransformArgs::WARP) {

        stackedWidget->setCurrentIndex(1);
        warpButton->setChecked(true);

        if (config.defaultPoints()) {
            densityBox->setValue(std::sqrt(config.numPoints()));
        }

        cmbWarpType->setCurrentIndex((int)config.warpType());
        defaultRadioButton->setChecked(config.defaultPoints());
        customRadioButton->setChecked(!config.defaultPoints());
        densityBox->setEnabled(config.defaultPoints());
        customWarpWidget->setEnabled(!config.defaultPoints());

        updateLockPointsButtonCaption();

    } else if (config.mode() == ToolTransformArgs::CAGE) {

        // default UI options
        resetUIOptions();

        // we need at least 3 point before we can start actively deforming
        if (config.origPoints().size() >= 3)
        {
            cageTransformDirections->setText(i18n("Switch between editing and deforming cage"));
            cageAddEditRadio->setVisible(true);
            cageDeformRadio->setVisible(true);

            // update to correct radio button
            if (config.isEditingTransformPoints())
                cageAddEditRadio->setChecked(true);
            else
                 cageDeformRadio->setChecked(true);

            changeGranularity->setCurrentIndex(log2(config.pixelPrecision()) - 2);
            granularityPreview->setCurrentIndex(log2(config.previewPixelPrecision()) - 2);

        }

    } else if (config.mode() == ToolTransformArgs::LIQUIFY) {

        stackedWidget->setCurrentIndex(3);
        liquifyButton->setChecked(true);

        const KisLiquifyProperties *props =
            config.liquifyProperties();

        switch (props->mode()) {
        case KisLiquifyProperties::MOVE:
            liquifyMove->setChecked(true);
            break;
        case KisLiquifyProperties::SCALE:
            liquifyScale->setChecked(true);
            break;
        case KisLiquifyProperties::ROTATE:
            liquifyRotate->setChecked(true);
            break;
        case KisLiquifyProperties::OFFSET:
            liquifyOffset->setChecked(true);
            break;
        case KisLiquifyProperties::UNDO:
            liquifyUndo->setChecked(true);
            break;
        case KisLiquifyProperties::N_MODES:
            qFatal("Unsupported mode");
        }

        updateLiquifyControls();
    } else if (config.mode() == ToolTransformArgs::MESH) {
        stackedWidget->setCurrentIndex(4);
        intNumColumns->setValue(config.meshTransform()->size().width() - 1);
        intNumRows->setValue(config.meshTransform()->size().height() - 1);
        chkShowControlPoints->setChecked(config.meshShowHandles());
        chkSymmetricalHandles->setChecked(config.meshSymmetricalHandles());
        chkScaleHandles->setChecked(config.meshScaleHandles());
    }

    unblockUiSlots();
}

void KisToolTransformConfigWidget::updateLockPointsButtonCaption()
{
    ToolTransformArgs *config = m_transaction->currentConfig();

    if (config->isEditingTransformPoints()) {
        lockUnlockPointsButton->setText(i18n("Lock Points"));
    } else {
        lockUnlockPointsButton->setText(i18n("Unlock Points"));
    }
}

void KisToolTransformConfigWidget::setApplyResetDisabled(bool disabled)
{
    QAbstractButton *applyButton = buttonBox->button(QDialogButtonBox::Apply);
    QAbstractButton *resetButton = buttonBox->button(QDialogButtonBox::Reset);

    Q_ASSERT(applyButton);
    Q_ASSERT(resetButton);

    applyButton->setDisabled(disabled);
    resetButton->setDisabled(disabled);
}

void KisToolTransformConfigWidget::resetRotationCenterButtons()
{
    int checkedId = m_rotationCenterButtons->checkedId();

    if (checkedId >= 0 && checkedId <= 8) {
        // uncheck the current checked button
        m_rotationCenterButtons->button(9)->setChecked(true);
    }
}

void KisToolTransformConfigWidget::setTooBigLabelVisible(bool value)
{
    tooBigLabelWidget->setVisible(value);
}

void KisToolTransformConfigWidget::blockNotifications()
{
    m_notificationsBlocked++;
}

void KisToolTransformConfigWidget::unblockNotifications()
{
    m_notificationsBlocked--;
}

void KisToolTransformConfigWidget::notifyConfigChanged()
{
    if (!m_notificationsBlocked) {
        emit sigConfigChanged();
    }
    m_configChanged = true;
}

void KisToolTransformConfigWidget::blockUiSlots()
{
    m_uiSlotsBlocked++;
}

void KisToolTransformConfigWidget::unblockUiSlots()
{
    m_uiSlotsBlocked--;
}

void KisToolTransformConfigWidget::notifyEditingFinished()
{
    if (m_uiSlotsBlocked || m_notificationsBlocked || !m_configChanged) return;

    emit sigEditingFinished();
    m_configChanged = false;
}


void KisToolTransformConfigWidget::resetUIOptions()
{
    // reset tool states since we are done (probably can encapsulate this later)
    ToolTransformArgs *config = m_transaction->currentConfig();
    if (config->mode() == ToolTransformArgs::CAGE)
    {
        cageAddEditRadio->setVisible(false);
        cageAddEditRadio->setChecked(true);
        cageDeformRadio->setVisible(false);
        cageTransformDirections->setText(i18n("Create 3 points on the canvas to begin"));

        // ensure we are on the right options view
        stackedWidget->setCurrentIndex(2);
    }



}

void KisToolTransformConfigWidget::slotButtonBoxClicked(QAbstractButton *button)
{
    QAbstractButton *applyButton = buttonBox->button(QDialogButtonBox::Apply);
    QAbstractButton *resetButton = buttonBox->button(QDialogButtonBox::Reset);

    resetUIOptions();

    if (button == applyButton) {
        emit sigApplyTransform();
    }
    else if (button == resetButton) {
        emit sigCancelTransform();
    }

}

void KisToolTransformConfigWidget::slotSetMeshModeButtonClicked(bool value)
{
    if (!value) return;

    lblTransformType->setText(meshButton->toolTip());

    emit sigResetTransform(ToolTransformArgs::MESH);
}

void KisToolTransformConfigWidget::slotSetFreeTransformModeButtonClicked(bool value)
{
    if (!value) return;

    lblTransformType->setText(freeTransformButton->toolTip());

    emit sigResetTransform(ToolTransformArgs::FREE_TRANSFORM);
}

void KisToolTransformConfigWidget::slotSetWarpModeButtonClicked(bool value)
{
    if (!value) return;

    lblTransformType->setText(warpButton->toolTip());

    emit sigResetTransform(ToolTransformArgs::WARP);
}

void KisToolTransformConfigWidget::slotSetCageModeButtonClicked(bool value)
{
    if (!value) return;

    lblTransformType->setText(cageButton->toolTip());

    emit sigResetTransform(ToolTransformArgs::CAGE);
}

void KisToolTransformConfigWidget::slotSetLiquifyModeButtonClicked(bool value)
{
    if (!value) return;

    lblTransformType->setText(liquifyButton->toolTip());

    emit sigResetTransform(ToolTransformArgs::LIQUIFY);
}

void KisToolTransformConfigWidget::slotSetPerspectiveModeButtonClicked(bool value)
{
    if (!value) return;

    lblTransformType->setText(perspectiveTransformButton->toolTip());

    emit sigResetTransform(ToolTransformArgs::PERSPECTIVE_4POINT);
}

void KisToolTransformConfigWidget::slotFilterChanged(const KoID &filterId)
{
    ToolTransformArgs *config = m_transaction->currentConfig();
    config->setFilterId(filterId.id());
    notifyConfigChanged();
}

void KisToolTransformConfigWidget::slotRotationCenterChanged(int index)
{
    if (m_uiSlotsBlocked) return;

    if (index >= 0 && index <= 8) {
        ToolTransformArgs *config = m_transaction->currentConfig();

        double i = m_handleDir[index].x();
        double j = m_handleDir[index].y();

        config->setRotationCenterOffset(QPointF(i * m_transaction->originalHalfWidth(),
                                                j * m_transaction->originalHalfHeight()));

        notifyConfigChanged();
        updateConfig(*config);
    }
}

void KisToolTransformConfigWidget::slotTransformAroundRotationCenter(bool value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();
    config->setTransformAroundRotationCenter(value);
    notifyConfigChanged();
    notifyEditingFinished();
}

void KisToolTransformConfigWidget::slotSetScaleX(int value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();

    {
        KisTransformUtils::AnchorHolder keeper(config->transformAroundRotationCenter(), config);
        config->setScaleX(value / 100.);
    }

    if (config->keepAspectRatio()) {

        blockNotifications();
         int calculatedValue = int( value/ m_scaleRatio );

        scaleYBox->blockSignals(true);
        scaleYBox->setValue(calculatedValue);
        {
            KisTransformUtils::AnchorHolder keeper(config->transformAroundRotationCenter(), config);
            config->setScaleY(calculatedValue / 100.);
        }
        scaleYBox->blockSignals(false);

        unblockNotifications();
    }

    notifyConfigChanged();
    notifyEditingFinished();
}

void KisToolTransformConfigWidget::slotSetScaleY(int value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();

    {
        KisTransformUtils::AnchorHolder keeper(config->transformAroundRotationCenter(), config);
        config->setScaleY(value / 100.);
    }

    if (config->keepAspectRatio()) {
        blockNotifications();
        int calculatedValue = int(m_scaleRatio * value);
        scaleXBox->blockSignals(true);
        scaleXBox->setValue(calculatedValue);
        {
            KisTransformUtils::AnchorHolder keeper(config->transformAroundRotationCenter(), config);
            config->setScaleX(calculatedValue / 100.);
        }
        scaleXBox->blockSignals(false);
        unblockNotifications();
    }

    notifyConfigChanged();
    notifyEditingFinished();
}

void KisToolTransformConfigWidget::slotSetShearX(qreal value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();

    {
        KisTransformUtils::AnchorHolder keeper(config->transformAroundRotationCenter(), config);
        config->setShearX((double)value / 100.);
    }

    notifyConfigChanged();
    notifyEditingFinished();
}

void KisToolTransformConfigWidget::slotSetShearY(qreal value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();

    {
        KisTransformUtils::AnchorHolder keeper(config->transformAroundRotationCenter(), config);
        config->setShearY((double)value / 100.);
    }


    notifyConfigChanged();
    notifyEditingFinished();
}

void KisToolTransformConfigWidget::slotSetTranslateX(int value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();

    const QPointF anchorPoint = config->originalCenter() + config->rotationCenterOffset();
    const KisTransformUtils::MatricesPack m(*config);

    const QPointF anchorPointView = m.finalTransform().map(anchorPoint);
    const QPointF newAnchorPointView(value, anchorPointView.y());
    config->setTransformedCenter(config->transformedCenter() + newAnchorPointView - anchorPointView);
    translateXBox->setValue(value);
    notifyConfigChanged();
}

void KisToolTransformConfigWidget::slotSetTranslateY(int value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();

    const QPointF anchorPoint = config->originalCenter() + config->rotationCenterOffset();
    const KisTransformUtils::MatricesPack m(*config);

    const QPointF anchorPointView = m.finalTransform().map(anchorPoint);
    const QPointF newAnchorPointView(anchorPointView.x(), value);
    config->setTransformedCenter(config->transformedCenter() + newAnchorPointView - anchorPointView);
    translateYBox->setValue(value);
    notifyConfigChanged();
}

void KisToolTransformConfigWidget::slotSetAX(qreal value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();
    {
        KisTransformUtils::AnchorHolder keeper(config->transformAroundRotationCenter(), config);
        config->setAX(normalizeAngle(kisDegreesToRadians(value)));
    }
    notifyConfigChanged();
    notifyEditingFinished();
}

void KisToolTransformConfigWidget::slotSetAY(qreal value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();

    {
        KisTransformUtils::AnchorHolder keeper(config->transformAroundRotationCenter(), config);
        config->setAY(normalizeAngle(kisDegreesToRadians(value)));
    }

    notifyConfigChanged();
    notifyEditingFinished();
}

void KisToolTransformConfigWidget::slotSetAZ(qreal value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();

    {
        KisTransformUtils::AnchorHolder keeper(config->transformAroundRotationCenter(), config);
        config->setAZ(normalizeAngle(kisDegreesToRadians(value)));
    }

    notifyConfigChanged();
    notifyEditingFinished();
}

void KisToolTransformConfigWidget::slotSetCameraHeight(qreal value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();
    config->setCameraPos(QVector3D(0,0,value));

    notifyConfigChanged();
    notifyEditingFinished();
}

void KisToolTransformConfigWidget::slotFlipX()
{
    ToolTransformArgs *config = m_transaction->currentConfig();

    {
        KisTransformUtils::AnchorHolder keeper(config->transformAroundRotationCenter(), config);
        config->setScaleX(config->scaleX() * -1);
    }

    notifyConfigChanged();
    notifyEditingFinished();
}

void KisToolTransformConfigWidget::slotFlipY()
{
    ToolTransformArgs *config = m_transaction->currentConfig();

    {
        KisTransformUtils::AnchorHolder keeper(config->transformAroundRotationCenter(), config);
        config->setScaleY(config->scaleY() * -1);
    }

    notifyConfigChanged();
    notifyEditingFinished();
}

void KisToolTransformConfigWidget::slotRotateCW()
{
    ToolTransformArgs *config = m_transaction->currentConfig();

    {
        KisTransformUtils::AnchorHolder keeper(config->transformAroundRotationCenter(), config);
        config->setAZ(normalizeAngle(config->aZ() + M_PI_2));
    }

    notifyConfigChanged();
    notifyEditingFinished();
}

void KisToolTransformConfigWidget::slotRotateCCW()
{
    ToolTransformArgs *config = m_transaction->currentConfig();

    {
        KisTransformUtils::AnchorHolder keeper(config->transformAroundRotationCenter(), config);
        config->setAZ(normalizeAngle(config->aZ() - M_PI_2));
    }

    notifyConfigChanged();
    notifyEditingFinished();
}


// change free transform setting we want to alter (all radio buttons call this)
void KisToolTransformConfigWidget::slotTransformAreaVisible(bool value)
{
    Q_UNUSED(value);

    //QCheckBox sender = (QCheckBox)(*)(QObject::sender());
    QString senderName = QObject::sender()->objectName();


    // only show setting with what we have selected
    rotationGroup->hide();
    shearGroup->hide();
    scaleGroup->hide();
    moveGroup->hide();


    if ("freeMoveRadioButton" == senderName)
    {
        moveGroup->show();
    }
    else  if ("freeShearRadioButton" == senderName)
    {
        shearGroup->show();
    }
    else if ("freeScaleRadioButton" == senderName)
    {
        scaleGroup->show();
    }
    else
    {
        rotationGroup->show();
    }

}

void KisToolTransformConfigWidget::slotSetKeepAspectRatio(bool value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();
    config->setKeepAspectRatio(value);

    if (value) {
       blockNotifications();
       int tmpXScaleBox = scaleXBox->value();
       int tmpYScaleBox = scaleYBox->value();
       m_scaleRatio = (tmpXScaleBox / (double) tmpYScaleBox);
       unblockNotifications();
    }

    notifyConfigChanged();
}

void KisToolTransformConfigWidget::slotSetWarpAlpha(qreal value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();

    config->setAlpha((double)value);
    notifyConfigChanged();
    notifyEditingFinished();
}

void KisToolTransformConfigWidget::slotSetWarpDensity(int value)
{
    if (m_uiSlotsBlocked) return;
    setDefaultWarpPoints(value);
}

void KisToolTransformConfigWidget::setDefaultWarpPoints(int pointsPerLine)
{
    ToolTransformArgs *config = m_transaction->currentConfig();

    KisTransformUtils::setDefaultWarpPoints(pointsPerLine, m_transaction, config);
    notifyConfigChanged();
}

void KisToolTransformConfigWidget::activateCustomWarpPoints(bool enabled)
{
    ToolTransformArgs *config = m_transaction->currentConfig();

    densityBox->setEnabled(!enabled);
    customWarpWidget->setEnabled(enabled);

    if (!enabled) {
        config->setEditingTransformPoints(false);
        setDefaultWarpPoints(densityBox->value());
        config->setWarpCalculation(KisWarpTransformWorker::WarpCalculation::GRID);
    } else {
        config->setEditingTransformPoints(true);
        config->setWarpCalculation(KisWarpTransformWorker::WarpCalculation::DRAW);
        setDefaultWarpPoints(0);
    }




    updateLockPointsButtonCaption();
}

void KisToolTransformConfigWidget::slotWarpDefaultPointsButtonClicked(bool value)
{
    if (m_uiSlotsBlocked) return;

    activateCustomWarpPoints(!value);
}

void KisToolTransformConfigWidget::slotWarpCustomPointsButtonClicked(bool value)
{
    if (m_uiSlotsBlocked) return;

    activateCustomWarpPoints(value);
}

void KisToolTransformConfigWidget::slotWarpResetPointsButtonClicked()
{
    if (m_uiSlotsBlocked) return;

    activateCustomWarpPoints(true);
}

void KisToolTransformConfigWidget::slotWarpLockPointsButtonClicked()
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();
    config->setEditingTransformPoints(!config->isEditingTransformPoints());

    if (config->isEditingTransformPoints()) {
        // reinit the transf points to their original value
        ToolTransformArgs *config = m_transaction->currentConfig();
        int nbPoints = config->origPoints().size();
        for (int i = 0; i < nbPoints; ++i) {
            config->transfPoint(i) = config->origPoint(i);
        }
    }

    updateLockPointsButtonCaption();
    notifyConfigChanged();
}

void KisToolTransformConfigWidget::slotWarpTypeChanged(int index)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();

    switch (index) {
    case KisWarpTransformWorker::AFFINE_TRANSFORM:
    case KisWarpTransformWorker::SIMILITUDE_TRANSFORM:
    case KisWarpTransformWorker::RIGID_TRANSFORM:
        config->setWarpType((KisWarpTransformWorker::WarpType)index);
        break;
    default:
        config->setWarpType(KisWarpTransformWorker::RIGID_TRANSFORM);
        break;
    }

    notifyConfigChanged();
}

void KisToolTransformConfigWidget::slotCageOptionsChanged(int value)
{
    if ( value == 0)
    {
        slotEditCagePoints(true);
    }
    else
    {
        slotEditCagePoints(false);
    }

    notifyEditingFinished();
}

void KisToolTransformConfigWidget::slotEditCagePoints(bool value)
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();
    config->refTransformedPoints() = config->origPoints();

    config->setEditingTransformPoints(value);
    notifyConfigChanged();
}

void KisToolTransformConfigWidget::slotGranularityChanged(QString value)
{
    if (m_uiSlotsBlocked) return;
    KIS_SAFE_ASSERT_RECOVER_RETURN(value.toInt() > 1);
    ToolTransformArgs *config = m_transaction->currentConfig();
    config->setPixelPrecision(value.toInt());
    notifyConfigChanged();
}

void KisToolTransformConfigWidget::slotPreviewGranularityChanged(QString value)
{
    if (m_uiSlotsBlocked) return;
    KIS_SAFE_ASSERT_RECOVER_RETURN(value.toInt() > 1);
    ToolTransformArgs *config = m_transaction->currentConfig();
    config->setPreviewPixelPrecision(value.toInt());
    notifyConfigChanged();
}

void KisToolTransformConfigWidget::slotMeshSizeChanged()
{
    if (m_uiSlotsBlocked) return;

    ToolTransformArgs *config = m_transaction->currentConfig();
    KisBezierTransformMesh &mesh = *config->meshTransform();

    if (mesh.size().width() != intNumColumns->value() + 1) {
        mesh.reshapeMeshHorizontally(intNumColumns->value() + 1);
    }

    if (mesh.size().height() != intNumRows->value() + 1) {
        mesh.reshapeMeshVertically(intNumRows->value() + 1);
    }

    notifyConfigChanged();
}

void KisToolTransformConfigWidget::slotMeshShowHandlesChanged()
{
    if (m_uiSlotsBlocked) return;
    ToolTransformArgs *config = m_transaction->currentConfig();
    config->setMeshShowHandles(this->chkShowControlPoints->isChecked());
    notifyConfigChanged();
    notifyEditingFinished();
}

void KisToolTransformConfigWidget::slotMeshSymmetricalHandlesChanged()
{
    if (m_uiSlotsBlocked) return;
    ToolTransformArgs *config = m_transaction->currentConfig();
    config->setMeshSymmetricalHandles(this->chkSymmetricalHandles->isChecked());
    notifyConfigChanged();
    notifyEditingFinished();
}

void KisToolTransformConfigWidget::slotMeshScaleHandlesChanged()
{
    if (m_uiSlotsBlocked) return;
    ToolTransformArgs *config = m_transaction->currentConfig();
    config->setMeshScaleHandles(this->chkScaleHandles->isChecked());
    notifyConfigChanged();
    notifyEditingFinished();
}
