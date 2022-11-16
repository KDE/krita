/*
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2014 Mohit Goyal <mohit.bits2011@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_brush_selection_widget.h"
#include <QLayout>
#include <QTabWidget>
#include <QFrame>
#include <QImage>
#include <QPainter>
#include <QBrush>
#include <QColor>
#include <QToolButton>
#include <QButtonGroup>
#include <QStackedWidget>

#include <klocalizedstring.h>

#include <widgets/kis_preset_chooser.h>
#include <kis_image.h>
#include <kis_fixed_paint_device.h>

#include "kis_brush.h"
#include "kis_auto_brush.h"
#include "kis_imagepipe_brush.h"
#include "kis_predefined_brush_chooser.h"
#include "kis_auto_brush_widget.h"
#include "kis_custom_brush_widget.h"
#include "kis_clipboard_brush_widget.h"
#include "kis_text_brush_chooser.h"
#include "KisWidgetConnectionUtils.h"
#include <KisZug.h>
#include <KisLager.h>
#include <lager/constant.hpp>

using namespace KisBrushModel;
using namespace KisWidgetConnectionUtils;

QString calcPrecisionToolTip(int precisionLevel) {
    QString toolTip;

    KIS_SAFE_ASSERT_RECOVER_NOOP(precisionLevel >= 1);
    KIS_SAFE_ASSERT_RECOVER_NOOP(precisionLevel <= 5);

    switch (precisionLevel) {
    case 1:
        toolTip =
            i18n("Precision Level 1 (fastest)\n"
                 "Subpixel precision: disabled\n"
                 "Brush size precision: 5%\n"
                 "\n"
                 "Optimal for very big brushes");
        break;
    case 2:
        toolTip =
            i18n("Precision Level 2\n"
                 "Subpixel precision: disabled\n"
                 "Brush size precision: 1%\n"
                 "\n"
                 "Optimal for big brushes");
        break;
    case 3:
        toolTip =
            i18n("Precision Level 3\n"
                 "Subpixel precision: disabled\n"
                 "Brush size precision: exact");
        break;
    case 4:
        toolTip =
            i18n("Precision Level 4 (optimal)\n"
                 "Subpixel precision: 50%\n"
                 "Brush size precision: exact\n"
                 "\n"
                 "Gives up to 50% better performance in comparison to Level 5");
        break;
    case 5:
        toolTip =
            i18n("Precision Level 5 (best quality)\n"
                 "Subpixel precision: exact\n"
                 "Brush size precision: exact\n"
                 "\n"
                 "The slowest performance. Best quality.");
        break;
    }
    return toolTip;
}

class PrecisionModel : public QObject
{
    Q_OBJECT

public:

    PrecisionModel(lager::cursor<PrecisionData> source)
        : m_source(source),
          LAGER_QT(precisionLevel) {m_source[&PrecisionData::precisionLevel]},
          LAGER_QT(useAutoPrecision) {m_source[&PrecisionData::useAutoPrecision]},
          LAGER_QT(precisionToolTip) {LAGER_QT(precisionLevel).map(&calcPrecisionToolTip)}
    {
    }

    lager::cursor<PrecisionData> m_source;

    LAGER_QT_CURSOR(int, precisionLevel);
    LAGER_QT_CURSOR(bool, useAutoPrecision);
    LAGER_QT_READER(QString, precisionToolTip);
};

struct KisBrushSelectionWidget::Private
{
    Private(lager::cursor<PrecisionData> precisionData)
        : model(precisionData)
    {
    }

    PrecisionModel model;
    lager::cursor<int> brushType;
};

KisBrushSelectionWidget::KisBrushSelectionWidget(int maxBrushSize,
                                                 KisAutoBrushModel *autoBrushModel,
                                                 KisPredefinedBrushModel *predefinedBrushModel,
                                                 KisTextBrushModel *textBrushModel,
                                                 lager::cursor<BrushType> brushType,
                                                 lager::cursor<PrecisionData> precisionData,
                                                 KisBrushOptionWidgetFlags flags,
                                                 QWidget *parent)
    : QWidget(parent)
    , m_currentBrushWidget(0)
    , m_d(new Private(precisionData))
{
    uiWdgBrushChooser.setupUi(this);

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);
    m_layout = new QGridLayout(uiWdgBrushChooser.settingsFrame);

    m_stackedWidget = new QStackedWidget(this);
    m_layout->addWidget(m_stackedWidget);

    m_autoBrushWidget = new KisAutoBrushWidget(maxBrushSize, autoBrushModel, this, "autobrush");
    addChooser(i18nc("Autobrush Brush tip mode", "Auto"), m_autoBrushWidget, AUTOBRUSH, KoGroupButton::GroupLeft);
    m_predefinedBrushWidget = new KisPredefinedBrushChooser(maxBrushSize, predefinedBrushModel, this);
    addChooser(i18nc("Predefined Brush tip mode", "Predefined"), m_predefinedBrushWidget, PREDEFINEDBRUSH, KoGroupButton::GroupCenter);

    m_textBrushWidget = new KisTextBrushChooser(textBrushModel, this);
    addChooser(i18nc("Text Brush tip mode", "Text"), m_textBrushWidget, TEXTBRUSH, KoGroupButton::GroupRight);

    m_d->brushType = brushType.zoom(kislager::lenses::do_static_cast<BrushType, int>);

    m_d->brushType.bind([this](int value) {m_stackedWidget->setCurrentIndex(value); });
    m_d->brushType.bind([this](int value) {m_buttonGroup->button(value)->setChecked(true); });
    connect(m_buttonGroup, qOverload<int>(&QButtonGroup::buttonClicked),
            [this] (int id) {m_d->brushType.set(id);});


    Q_FOREACH (QWidget *widget, m_chooserMap.values()) {
        m_minimumSize = m_minimumSize.expandedTo(widget->sizeHint());
    }


    setCurrentWidget(m_autoBrushWidget);

    uiWdgBrushChooser.sliderPrecision->setRange(1, 5);
    uiWdgBrushChooser.sliderPrecision->setSingleStep(1);
    uiWdgBrushChooser.sliderPrecision->setPageStep(1);


    if (flags & KisBrushOptionWidgetFlag::SupportsPrecision) {
        connectControl(uiWdgBrushChooser.sliderPrecision, &m_d->model, "precisionLevel");
        connectControl(uiWdgBrushChooser.autoPrecisionCheckBox, &m_d->model, "useAutoPrecision");
        connect(&m_d->model, &PrecisionModel::precisionToolTipChanged,
                uiWdgBrushChooser.sliderPrecision, &KisSliderSpinBox::setToolTip);

        connect(&m_d->model, &PrecisionModel::useAutoPrecisionChanged,
                uiWdgBrushChooser.sliderPrecision, &KisSliderSpinBox::setDisabled);
        connect(&m_d->model, &PrecisionModel::useAutoPrecisionChanged,
                uiWdgBrushChooser.lblPrecision, &KisSliderSpinBox::setDisabled);
    } else {
        uiWdgBrushChooser.sliderPrecision->setVisible(false);
        uiWdgBrushChooser.autoPrecisionCheckBox->setVisible(false);
        uiWdgBrushChooser.lblPrecision->setVisible(false);
    }
}


KisBrushSelectionWidget::~KisBrushSelectionWidget()
{
}

KisBrushSP KisBrushSelectionWidget::brush() const
{
    KisBrushSP theBrush;

    // Fallback to auto brush if no brush selected
    // Can happen if there is no predefined brush found
    if (!theBrush)
        theBrush = m_autoBrushWidget->brush();

    return theBrush;

}
void KisBrushSelectionWidget::setImage(KisImageWSP image)
{
    m_predefinedBrushWidget->setImage(image);
}

void KisBrushSelectionWidget::hideOptions(const QStringList &options)
{
    Q_FOREACH(const QString &option, options) {
        QStringList l = option.split("/");
        if (l.count() != 2) {
            continue;
        }
        QObject *o = 0;
        if (l[0] == "KisAutoBrushWidget") {
            o = m_autoBrushWidget->findChild<QObject*>(l[1]);
        }
        else if (l[0] == "KisBrushChooser") {
            o = m_predefinedBrushWidget->findChild<QObject*>(l[1]);
        }
        else if (l[0] == "KisTextBrushChooser") {
            o = m_textBrushWidget->findChild<QObject*>(l[1]);
        }
        else {
            qWarning() << "KisBrushSelectionWidget: Invalid option given to disable:" << option;
        }

        if (o) {
            QWidget *w = qobject_cast<QWidget*>(o);
            if (w) {
                w->setVisible(false);
            }
            o = 0;
        }
    }
}

lager::reader<bool> KisBrushSelectionWidget::lightnessModeEnabled() const
{
    return lager::with(m_d->brushType.xform(kiszug::map_equal<int>(Predefined)),
                       m_predefinedBrushWidget->lightnessModeEnabled())
            .map(std::logical_and{});
}

void KisBrushSelectionWidget::setCurrentWidget(QWidget* widget)
{
    return;

    if (widget == m_currentBrushWidget) return;

    if (m_currentBrushWidget) {
        m_layout->removeWidget(m_currentBrushWidget);
        m_currentBrushWidget->setParent(this);
        m_currentBrushWidget->hide();
    }
    widget->setMinimumSize(m_minimumSize);

    m_currentBrushWidget = widget;
    m_layout->addWidget(widget);

    m_currentBrushWidget->show();
    m_buttonGroup->button(m_chooserMap.key(widget))->setChecked(true);
}

void KisBrushSelectionWidget::addChooser(const QString& text, QWidget* widget, int id, KoGroupButton::GroupPosition pos)
{
    KoGroupButton *button = new KoGroupButton(this);
    button->setGroupPosition(pos);
    button->setText(text);
    button->setAutoRaise(false);
    button->setCheckable(true);
    uiWdgBrushChooser.brushChooserButtonLayout->addWidget(button);
    m_buttonGroup->addButton(button, id);

    m_stackedWidget->insertWidget(id, widget);
}

#include "kis_brush_selection_widget.moc"
