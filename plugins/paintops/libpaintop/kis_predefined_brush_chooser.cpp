/*
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2009 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_predefined_brush_chooser.h"

#include <QtMath>
#include <QLabel>
#include <QLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPainter>
#include <QAbstractItemDelegate>
#include <klocalizedstring.h>

#include <KoFileDialog.h>
#include <KisKineticScroller.h>

#include <KisResourceItemView.h>
#include <KisResourceItemChooser.h>
#include <KisResourceModel.h>

#include <kis_icon.h>
#include "KisBrushServerProvider.h"
#include "kis_algebra_2d.h"
#include "kis_painting_tweaks.h"
#include "kis_slider_spin_box.h"
#include "krita_utils.h"
#include "kis_spacing_selection_widget.h"
#include "kis_signals_blocker.h"

#include "kis_imagepipe_brush.h"
#include "kis_custom_brush_widget.h"
#include "kis_clipboard_brush_widget.h"
#include <kis_image_config.h>
#include <KisMimeDatabase.h>

#include "kis_global.h"
#include "kis_gbr_brush.h"
#include "kis_png_brush.h"
#include "kis_debug.h"
#include "kis_image.h"
#include <KisGlobalResourcesInterface.h>
#include <KisResourceLoaderRegistry.h>
#include <KisTagFilterResourceProxyModel.h>
#include <KisStorageModel.h>
#include <KisResourceUserOperations.h>

#include <KisWidgetConnectionUtils.h>

#include <lager/state.hpp>
#include <lager/constant.hpp>
#include <kis_predefined_brush_factory.h>
#include <KisZug.h>

using namespace KisBrushModel;
using namespace KisWidgetConnectionUtils;

/// The resource item delegate for rendering the resource preview
class KisBrushDelegate : public QAbstractItemDelegate
{
public:
    KisBrushDelegate(QObject * parent = 0) : QAbstractItemDelegate(parent) {}
    ~KisBrushDelegate() override {}
    /// reimplemented
    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const override;
    /// reimplemented
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex &) const override {
        return option.decorationSize;
    }
};

void KisBrushDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    if (! index.isValid())
        return;

    QImage thumbnail = index.data(Qt::UserRole + KisAbstractResourceModel::Thumbnail).value<QImage>();

    const QRect itemRect = kisGrowRect(option.rect, -1);
    const qreal devicePixelRatioF = painter->device()->devicePixelRatioF();

    const QSize hidpiSize = itemRect.size() * devicePixelRatioF;
    thumbnail = thumbnail.scaled(hidpiSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    thumbnail.setDevicePixelRatio(devicePixelRatioF);

    painter->save();

    const QMap<QString, QVariant> metadata =
        index.data(Qt::UserRole + KisAbstractResourceModel::MetaData).value<QMap<QString, QVariant>>();

    const bool hasImageType =
        metadata.value(KisBrush::brushTypeMetaDataKey,
                       QVariant::fromValue(false)).toBool();


    if (hasImageType) {
        KisPaintingTweaks::PenBrushSaver s(painter);

        const int baseSize = qCeil(itemRect.width() / 5.0);
        QImage brush(2 * baseSize, 2 * baseSize, QImage::Format_ARGB32);
        brush.fill(Qt::white);
        QPainter gc(&brush);

        gc.setPen(Qt::NoPen);
        gc.setBrush(QColor(200,200,200));
        gc.drawRect(QRect(0,0,baseSize,baseSize));
        gc.drawRect(QRect(baseSize,baseSize,baseSize,baseSize));

        painter->setBrush(brush);

        painter->setBrushOrigin(itemRect.topLeft());
        painter->drawRect(itemRect);
        painter->setBrush(Qt::NoBrush);

    } else {
        KisPaintingTweaks::PenBrushSaver s(painter);
        painter->setBrush(Qt::white);
        painter->setPen(Qt::NoPen);
        painter->drawRect(itemRect);
    }

    int dx = (itemRect.width() * devicePixelRatioF - thumbnail.width()) / 2 / devicePixelRatioF;
    int dy = (itemRect.height() * devicePixelRatioF - thumbnail.height()) / 2 / devicePixelRatioF;
    painter->drawImage(itemRect.x() + dx, itemRect.y() + dy, thumbnail);

    if (option.state & QStyle::State_Selected) {
        painter->setClipRect(option.rect);
        painter->setPen(QPen(option.palette.highlight(), 2.0));
        KritaUtils::renderExactRect(painter, itemRect);
        painter->setCompositionMode(QPainter::CompositionMode_HardLight);
        painter->setOpacity(0.65);
        painter->fillRect(itemRect, option.palette.highlight());
    }

    painter->restore();
}

auto brushSizeLens = lager::lenses::getset(
    [](std::tuple<QSize, qreal> x) -> qreal { return std::get<0>(x).width() * std::get<1>(x); },
    [](std::tuple<QSize, qreal> x, qreal brushSize) -> std::tuple<QSize, qreal> {
        return std::make_tuple(std::get<0>(x), brushSize / std::get<0>(x).width());
    });


ComboBoxState calcApplicationSwitchState(enumBrushType brushType, bool supportsHSLBrushTips, enumBrushApplication application)
{
    QStringList values;
    QStringList toolTips;
    values << i18n("Alpha Mask");
    toolTips << i18nc("@info:tooltip", "Luminosity of the brush tip image is used as alpha channel for the stroke");
    if (brushType == IMAGE || brushType == PIPE_IMAGE) {
        values << i18n("Color Image");
        toolTips << i18nc("@info:tooltip", "The brush tip image is painted as it is");
        if (supportsHSLBrushTips) {
            values << i18n("Lightness Map");
            toolTips << i18nc("@info:tooltip", "Luminosity of the brush tip image is used as lightness correction for the painting color. Alpha channel of the brush tip image is used as alpha for the final stroke");
            values << i18n("Gradient Map");
            toolTips << i18nc("@info:tooltip", "The brush tip maps its value to the currently selected gradient. Alpha channel of the brush tip image is used as alpha for the final stroke");
        }
    }


    int currentValue = std::clamp(static_cast<int>(application), 0, values.size() - 1);
    return ComboBoxState{values, currentValue, values.size() > 1, toolTips};
}

QString calcBrushDetails(PredefinedBrushData data)
{
    QString brushTypeString = "";

    QString animatedBrushTipSelectionMode;

    if (data.brushType == INVALID) {
        brushTypeString = i18n("Invalid");
    } else if (data.brushType == MASK) {
        brushTypeString = i18n("Mask");
    } else if (data.brushType == IMAGE) {
        brushTypeString = i18n("Image");
    } else if (data.brushType == PIPE_MASK ) {
        brushTypeString = i18n("Animated Mask"); // GIH brush
        animatedBrushTipSelectionMode = data.parasiteSelection;
    } else if (data.brushType == PIPE_IMAGE ) {
        brushTypeString = i18n("Animated Image");
    }

    const QString brushDetailsText = QString("%1 (%2 x %3) %4")
            .arg(brushTypeString)
            .arg(data.baseSize.width())
            .arg(data.baseSize.height())
            .arg(animatedBrushTipSelectionMode);

    return brushDetailsText;
}

class PredefinedBrushModel : public QObject
{
    Q_OBJECT
public:
    PredefinedBrushModel(lager::cursor<CommonData> commonData,
                         lager::cursor<PredefinedBrushData> predefinedBrushData,
                         bool supportsHSLBrushTips)
        : m_commonData(commonData),
          m_predefinedBrushData(predefinedBrushData),
          m_supportsHSLBrushTips(supportsHSLBrushTips),
          LAGER_QT(resourceSignature) {m_predefinedBrushData[&PredefinedBrushData::resourceSignature]},
          LAGER_QT(baseSize) {m_predefinedBrushData[&PredefinedBrushData::baseSize]},
          LAGER_QT(scale) {m_predefinedBrushData[&PredefinedBrushData::scale]},
          LAGER_QT(brushSize) {lager::with(LAGER_QT(baseSize), LAGER_QT(scale))
                      .zoom(brushSizeLens)},
          LAGER_QT(application) {m_predefinedBrushData[&PredefinedBrushData::application]
                      .zoom(kiszug::lenses::do_static_cast<enumBrushApplication, int>)},
          LAGER_QT(hasColorAndTransparency) {m_predefinedBrushData[&PredefinedBrushData::hasColorAndTransparency]},
          LAGER_QT(autoAdjustMidPoint) {m_predefinedBrushData[&PredefinedBrushData::autoAdjustMidPoint]},
          LAGER_QT(adjustmentMidPoint) {m_predefinedBrushData[&PredefinedBrushData::adjustmentMidPoint]
                      .zoom(kiszug::lenses::do_static_cast<quint8, int>)},
          LAGER_QT(brightnessAdjustment) {m_predefinedBrushData[&PredefinedBrushData::brightnessAdjustment]
                      .xform(kiszug::map_mupliply<qreal>(100.0) | kiszug::map_round,
                             kiszug::map_static_cast<qreal> | kiszug::map_mupliply<qreal>(0.01))},
          LAGER_QT(contrastAdjustment) {m_predefinedBrushData[&PredefinedBrushData::contrastAdjustment]
                      .xform(kiszug::map_mupliply<qreal>(100.0) | kiszug::map_round,
                             kiszug::map_static_cast<qreal> | kiszug::map_mupliply<qreal>(0.01))},
          LAGER_QT(angle) {m_commonData[&CommonData::angle]
                      .zoom(kiszug::lenses::scale<qreal>(180.0 / M_PI))},
          LAGER_QT(spacing) {m_commonData[&CommonData::spacing]},
          LAGER_QT(useAutoSpacing) {m_commonData[&CommonData::useAutoSpacing]},
          LAGER_QT(autoSpacingCoeff) {m_commonData[&CommonData::autoSpacingCoeff]},
          LAGER_QT(aggregatedSpacing) {lager::with(LAGER_QT(spacing),
                                                   LAGER_QT(useAutoSpacing),
                                                   LAGER_QT(autoSpacingCoeff))
                      .xform(zug::map(ToSpacingState{}),
                             zug::map(FromSpacingState{}))},
          LAGER_QT(applicationSwitchState){lager::with(
                      m_predefinedBrushData[&PredefinedBrushData::brushType],
                      m_supportsHSLBrushTips,
                      m_predefinedBrushData[&PredefinedBrushData::application])
                      .xform(zug::map(&calcApplicationSwitchState))},
          LAGER_QT(adjustmentsEnabled){LAGER_QT(applicationSwitchState)[&ComboBoxState::currentIndex]
                      .xform(kiszug::map_greater<int>(1))},
          LAGER_QT(brushName) {LAGER_QT(resourceSignature)[&KoResourceSignature::name]},
          LAGER_QT(brushDetails) {m_predefinedBrushData.map(&calcBrushDetails)},
          LAGER_QT(lightnessModeEnabled)
              {LAGER_QT(applicationSwitchState)
                    [&ComboBoxState::currentIndex].
                      xform(kiszug::map_equal<int>(LIGHTNESSMAP))}
    {
    }

    // the state must be declared **before** any cursors or readers
    lager::cursor<CommonData> m_commonData;
    lager::cursor<PredefinedBrushData> m_predefinedBrushData;
    lager::constant<bool> m_supportsHSLBrushTips;

    LAGER_QT_CURSOR(KoResourceSignature, resourceSignature);
    LAGER_QT_CURSOR(QSize, baseSize);
    LAGER_QT_CURSOR(qreal, scale);
    LAGER_QT_CURSOR(qreal, brushSize);
    LAGER_QT_CURSOR(int, application);
    LAGER_QT_CURSOR(bool, hasColorAndTransparency);
    LAGER_QT_CURSOR(bool, autoAdjustMidPoint);
    LAGER_QT_CURSOR(int, adjustmentMidPoint);
    LAGER_QT_CURSOR(int, brightnessAdjustment);
    LAGER_QT_CURSOR(int, contrastAdjustment);

    LAGER_QT_CURSOR(qreal, angle);
    LAGER_QT_CURSOR(qreal, spacing);
    LAGER_QT_CURSOR(bool, useAutoSpacing);
    LAGER_QT_CURSOR(qreal, autoSpacingCoeff);
    LAGER_QT_CURSOR(SpacingState, aggregatedSpacing);
    LAGER_QT_READER(ComboBoxState, applicationSwitchState);
    LAGER_QT_READER(bool, adjustmentsEnabled);
    LAGER_QT_READER(QString, brushName);
    LAGER_QT_READER(QString, brushDetails);
    LAGER_QT_READER(bool, lightnessModeEnabled);

    void sanitize() {
        setapplication(applicationSwitchState().currentIndex);
    }
};

struct KisPredefinedBrushChooser::Private
{
    Private(lager::cursor<KisBrushModel::CommonData> _commonBrushData,
            lager::cursor<KisBrushModel::PredefinedBrushData> _predefinedBrushData,
            bool supportsHSLBrushTips)
        : commonBrushData(_commonBrushData),
          predefinedBrushData(_predefinedBrushData),
          model(commonBrushData, predefinedBrushData, supportsHSLBrushTips)
    {
    }

    lager::cursor<KisBrushModel::CommonData> commonBrushData;
    lager::cursor<KisBrushModel::PredefinedBrushData> predefinedBrushData;
    PredefinedBrushModel model;
};


KisPredefinedBrushChooser::KisPredefinedBrushChooser(int maxBrushSize,
                                                     lager::cursor<KisBrushModel::CommonData> commonBrushData,
                                                     lager::cursor<KisBrushModel::PredefinedBrushData> predefinedBrushData,
                                                     bool supportsHSLBrushTips,
                                                     QWidget *parent, const char *name)
    : QWidget(parent),
      m_d(new Private(commonBrushData, predefinedBrushData, supportsHSLBrushTips)),
      m_stampBrushWidget(0),
      m_clipboardBrushWidget(0)
{
    setObjectName(name);

    setupUi(this);

    brushSizeSpinBox->setRange(0, maxBrushSize, 2);
    brushSizeSpinBox->setValue(5);
    brushSizeSpinBox->setExponentRatio(3.0);
    brushSizeSpinBox->setSuffix(i18n(" px"));
    brushSizeSpinBox->setExponentRatio(3.0);

    connect(&m_d->model, &PredefinedBrushModel::brushNameChanged,
            brushTipNameLabel, &QLabel::setText);
    m_d->model.LAGER_QT(brushName).nudge();

    connect(&m_d->model, &PredefinedBrushModel::brushDetailsChanged,
            brushDetailsLabel, &QLabel::setText);
    m_d->model.LAGER_QT(brushName).nudge();

    connectControl(brushSizeSpinBox, &m_d->model, "brushSize");

    brushRotationAngleSelector->setDecimals(0);

    connectControl(brushRotationAngleSelector, &m_d->model, "angle");

    brushSpacingSelectionWidget->setSpacing(true, 1.0);

    connectControl(brushSpacingSelectionWidget, &m_d->model, "aggregatedSpacing");

    m_itemChooser = new KisResourceItemChooser(ResourceType::Brushes, false, this);
    m_itemChooser->setObjectName("brush_selector");

    m_itemChooser->showTaggingBar(true);
    m_itemChooser->setRowHeight(30);
    m_itemChooser->setItemDelegate(new KisBrushDelegate(this));
    m_itemChooser->setCurrentItem(0);
    m_itemChooser->setSynced(true);
    m_itemChooser->setMinimumWidth(100);
    m_itemChooser->setMinimumHeight(150);
    m_itemChooser->showButtons(false); // turn the import and delete buttons since we want control over them

    presetsLayout->addWidget(m_itemChooser);

    connect(m_itemChooser, &KisResourceItemChooser::resourceSelected,
            this, &KisPredefinedBrushChooser::slotBrushSelected);
    connect(&m_d->model, &PredefinedBrushModel::resourceSignatureChanged,
            this, &KisPredefinedBrushChooser::slotBrushPropertyChanged);

    slotBrushPropertyChanged(m_d->model.resourceSignature());


    addPresetButton->setIcon(KisIconUtils::loadIcon("list-add"));
    deleteBrushTipButton->setIcon(KisIconUtils::loadIcon("edit-delete"));

    connect(addPresetButton, SIGNAL(clicked(bool)), this, SLOT(slotImportNewBrushResource()));
    connect(deleteBrushTipButton, SIGNAL(clicked(bool)), this, SLOT(slotDeleteBrushResource()));

    stampButton->setIcon(KisIconUtils::loadIcon("list-add"));
    stampButton->setToolTip(i18n("Creates a brush tip from the current image selection."
                               "\n If no selection is present the whole image will be used."));

    clipboardButton->setIcon(KisIconUtils::loadIcon("list-add"));
    clipboardButton->setToolTip(i18n("Creates a brush tip from the image in the clipboard."));

    connect(stampButton, SIGNAL(clicked()), this,  SLOT(slotOpenStampBrush()));
    connect(clipboardButton, SIGNAL(clicked()), SLOT(slotOpenClipboardBrush()));

    resetBrushButton->setToolTip(i18n("Reloads Spacing from file\nSets Scale to 1.0\nSets Rotation to 0.0"));
    connect(resetBrushButton, SIGNAL(clicked()), SLOT(slotResetBrush()));

    intAdjustmentMidPoint->setRange(0, 255);
    intAdjustmentMidPoint->setPageStep(10);
    intAdjustmentMidPoint->setSingleStep(1);
    intAdjustmentMidPoint->setPrefix(i18nc("@label:slider", "Neutral point: "));
    connectControl(intAdjustmentMidPoint, &m_d->model, "adjustmentMidPoint");
    connectControl(chkAutoMidPoint, &m_d->model, "autoAdjustMidPoint");

    intBrightnessAdjustment->setRange(-100, 100);
    intBrightnessAdjustment->setPageStep(10);
    intBrightnessAdjustment->setSingleStep(1);
    intBrightnessAdjustment->setSuffix("%");
    intBrightnessAdjustment->setPrefix(i18nc("@label:slider", "Brightness: "));
    connectControl(intBrightnessAdjustment, &m_d->model, "brightnessAdjustment");

    intContrastAdjustment->setRange(-100, 100);
    intContrastAdjustment->setPageStep(10);
    intContrastAdjustment->setSingleStep(1);
    intContrastAdjustment->setSuffix("%");
    intContrastAdjustment->setPrefix(i18nc("@label:slider", "Contrast: "));
    connectControl(intContrastAdjustment, &m_d->model, "contrastAdjustment");

    btnResetAdjustments->setToolTip(i18nc("@info:tooltip", "Resets all the adjustments to default values:\n Neutral Point: 127\n Brightness: 0%\n Contrast: 0%"));
    connect(btnResetAdjustments, SIGNAL(clicked()), SLOT(slotResetAdjustments()));

    connectControlState(cmbBrushMode, &m_d->model, "applicationSwitchState", "application");

    connect(&m_d->model, &PredefinedBrushModel::adjustmentsEnabledChanged,
            intAdjustmentMidPoint, &KisSliderSpinBox::setEnabled);
    connect(&m_d->model, &PredefinedBrushModel::adjustmentsEnabledChanged,
            intBrightnessAdjustment, &KisSliderSpinBox::setEnabled);
    connect(&m_d->model, &PredefinedBrushModel::adjustmentsEnabledChanged,
            intContrastAdjustment, &KisSliderSpinBox::setEnabled);
    connect(&m_d->model, &PredefinedBrushModel::adjustmentsEnabledChanged,
            chkAutoMidPoint, &KisSliderSpinBox::setEnabled);
    connect(&m_d->model, &PredefinedBrushModel::adjustmentsEnabledChanged,
            btnResetAdjustments, &KisSliderSpinBox::setEnabled);

    m_d->model.LAGER_QT(adjustmentsEnabled).nudge();
}

KisPredefinedBrushChooser::~KisPredefinedBrushChooser()
{
}

void KisPredefinedBrushChooser::slotResetBrush()
{
    KisBrushSP brush = m_itemChooser->currentResource().dynamicCast<KisBrush>();
    if (brush) {
        KisBrushModel::CommonData commonData;
        KisBrushModel::PredefinedBrushData predefinedData;

        KisPredefinedBrushFactory::loadFromBrushResource(commonData, predefinedData, brush);

        if (m_d->model.applicationSwitchState().items.size() >= LIGHTNESSMAP) {
            predefinedData.application = LIGHTNESSMAP;
        }

        m_d->commonBrushData.set(commonData);
        m_d->predefinedBrushData.set(predefinedData);
    }
}

void KisPredefinedBrushChooser::slotOpenStampBrush()
{
    if(!m_stampBrushWidget) {
        m_stampBrushWidget = new KisCustomBrushWidget(this, i18n("Stamp"), m_image);
        m_stampBrushWidget->setModal(false);
        connect(m_stampBrushWidget, SIGNAL(sigNewPredefinedBrush(KoResourceSP )),
                                    SLOT(slotNewPredefinedBrush(KoResourceSP )));
    } else {
        m_stampBrushWidget->setImage(m_image);
    }

    QDialog::DialogCode result = (QDialog::DialogCode)m_stampBrushWidget->exec();

    if(result) {
        // noop
    }
}
void KisPredefinedBrushChooser::slotOpenClipboardBrush()
{
    if(!m_clipboardBrushWidget) {
        m_clipboardBrushWidget = new KisClipboardBrushWidget(this, i18n("Clipboard"), m_image);
        m_clipboardBrushWidget->setModal(true);
        connect(m_clipboardBrushWidget, SIGNAL(sigNewPredefinedBrush(KoResourceSP )),
                                        SLOT(slotNewPredefinedBrush(KoResourceSP )));
    }

    QDialog::DialogCode result = (QDialog::DialogCode)m_clipboardBrushWidget->exec();

    if(result) {
        // noop
    }
}

void KisPredefinedBrushChooser::slotBrushSelected(KoResourceSP resource)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(resource);

    KisBrushModel::CommonData commonBrushData = *m_d->commonBrushData;
    KisBrushModel::PredefinedBrushData predefinedBrushData = *m_d->predefinedBrushData;

    KisPredefinedBrushFactory::loadFromBrushResource(commonBrushData, predefinedBrushData, resource.dynamicCast<KisBrush>());

    predefinedBrushData.scale = 1.0;

    auto commonBrushState = lager::make_state(commonBrushData, lager::automatic_tag{});
    auto predefinedBrushState = lager::make_state(predefinedBrushData, lager::automatic_tag{});

    // TODO: extract appliction logic into a lens
    PredefinedBrushModel model(commonBrushState, predefinedBrushState, *m_d->model.m_supportsHSLBrushTips);
    model.sanitize();

    // TODO: check what happens when we add a new brush
    if (this->preserveBrushPresetSettings->isChecked()) {
        model.setbrushSize(m_d->model.brushSize());
        model.setspacing(m_d->model.spacing());
        model.setuseAutoSpacing(m_d->model.useAutoSpacing());
        model.setautoSpacingCoeff(m_d->model.autoSpacingCoeff());
    }

    m_d->commonBrushData.set(*commonBrushState);
    m_d->predefinedBrushData.set(*predefinedBrushState);
}

void KisPredefinedBrushChooser::slotBrushPropertyChanged(KoResourceSignature signature)
{
    auto source = KisGlobalResourcesInterface::instance()->source<KisBrush>(ResourceType::Brushes);
    m_itemChooser->setCurrentResource(source.bestMatch(signature.md5sum, signature.filename, signature.name));
}

void KisPredefinedBrushChooser::slotResetAdjustments()
{
    m_d->predefinedBrushData.update(
        [] (KisBrushModel::PredefinedBrushData brush) {
            KisBrushModel::PredefinedBrushData defaultBrush;

            brush.adjustmentMidPoint = defaultBrush.adjustmentMidPoint;
            brush.brightnessAdjustment = defaultBrush.brightnessAdjustment;
            brush.contrastAdjustment = defaultBrush.contrastAdjustment;
            brush.autoAdjustMidPoint = defaultBrush.autoAdjustMidPoint;

            return brush;
        });
}

void KisPredefinedBrushChooser::slotNewPredefinedBrush(KoResourceSP resource)
{
    m_itemChooser->setCurrentResource(resource);
}

void KisPredefinedBrushChooser::setImage(KisImageWSP image)
{
    m_image = image;
}

lager::reader<bool> KisPredefinedBrushChooser::lightnessModeEnabled() const
{
    return m_d->model.LAGER_QT(lightnessModeEnabled);
}

void KisPredefinedBrushChooser::slotImportNewBrushResource() {
    // reflects m_itemChooser->slotButtonClicked(KisResourceItemChooser::Button_Import)
    // but adds the .abr files support, as it was in Krita 4
    QStringList mimeTypes = KisResourceLoaderRegistry::instance()->mimeTypes(ResourceType::Brushes);
    QString abrMimeType = "image/x-adobe-brushlibrary";
    mimeTypes.append(abrMimeType);
    KoFileDialog dialog(0, KoFileDialog::OpenFiles, "OpenDocument");
    dialog.setMimeTypeFilters(mimeTypes);
    dialog.setCaption(i18nc("@title:window", "Choose File to Add"));
    Q_FOREACH(const QString &filename, dialog.filenames()) {
        if (QFileInfo(filename).exists() && QFileInfo(filename).isReadable()) {
            if (KisMimeDatabase::mimeTypeForFile(filename).contains(abrMimeType)) {
                KisStorageModel::instance()->importStorage(filename, KisStorageModel::None);
            } else {
                KisResourceUserOperations::importResourceFileWithUserInput(this, "", ResourceType::Brushes, filename);
            }
        }
    }
    m_itemChooser->tagFilterModel()->sort(Qt::DisplayRole);
}

void KisPredefinedBrushChooser::slotDeleteBrushResource() {
    m_itemChooser->slotButtonClicked(KisResourceItemChooser::Button_Remove);
}


#include "kis_predefined_brush_chooser.moc"


