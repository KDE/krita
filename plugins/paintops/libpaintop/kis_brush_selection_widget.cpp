/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2014 Mohit Goyal    <mohit.bits2011@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

#include <klocalizedstring.h>

#include <widgets/kis_preset_chooser.h>
#include <kis_image.h>
#include <kis_fixed_paint_device.h>

#include "kis_brush.h"
#include "kis_auto_brush.h"
#include "kis_imagepipe_brush.h"
#include "kis_brush_chooser.h"
#include "kis_auto_brush_widget.h"
#include "kis_custom_brush_widget.h"
#include "kis_clipboard_brush_widget.h"
#include "kis_text_brush_chooser.h"

KisBrushSelectionWidget::KisBrushSelectionWidget(QWidget * parent)
    : QWidget(parent), m_currentBrushWidget(0)
{
    uiWdgBrushChooser.setupUi(this);

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);

    m_layout = new QGridLayout(uiWdgBrushChooser.settingsFrame);
    m_autoBrushWidget = new KisAutoBrushWidget(this, "autobrush");
    connect(m_autoBrushWidget, SIGNAL(sigBrushChanged()), SIGNAL(sigBrushChanged()));
    addChooser(i18n("Auto"), m_autoBrushWidget, AUTOBRUSH, KoGroupButton::GroupLeft);

    m_predefinedBrushWidget = new KisPredefinedBrushChooser(this);
    connect(m_predefinedBrushWidget, SIGNAL(sigBrushChanged()), SIGNAL(sigBrushChanged()));
    addChooser(i18n("Predefined"), m_predefinedBrushWidget, PREDEFINEDBRUSH, KoGroupButton::GroupCenter);

    m_textBrushWidget = new KisTextBrushChooser(this, "textbrush", i18n("Text"));
    connect(m_textBrushWidget, SIGNAL(sigBrushChanged()), SIGNAL(sigBrushChanged()));
    addChooser(i18n("Text"), m_textBrushWidget, TEXTBRUSH, KoGroupButton::GroupRight);

    connect(m_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(buttonClicked(int)));

    Q_FOREACH (QWidget * widget, m_chooserMap.values()) {
        m_mininmumSize = m_mininmumSize.expandedTo(widget->sizeHint());
    }

    setCurrentWidget(m_autoBrushWidget);

    uiWdgBrushChooser.sliderPrecision->setRange(1, 5);
    uiWdgBrushChooser.sliderPrecision->setSingleStep(1);
    uiWdgBrushChooser.sliderPrecision->setPageStep(1);
    connect(uiWdgBrushChooser.sliderPrecision, SIGNAL(valueChanged(int)), SLOT(precisionChanged(int)));
    connect(uiWdgBrushChooser.autoPrecisionCheckBox, SIGNAL(stateChanged(int)), SLOT(setAutoPrecisionEnabled(int)));
    connect(uiWdgBrushChooser.deltaValueSpinBox, SIGNAL(valueChanged(double)), SLOT(setDeltaValue(double)));
    connect(uiWdgBrushChooser.sizeToStartFromSpinBox, SIGNAL(valueChanged(double)), SLOT(setSizeToStartFrom(double)));
    uiWdgBrushChooser.sliderPrecision->setValue(4);
    setPrecisionEnabled(false);
    uiWdgBrushChooser.label->setVisible(false);
    uiWdgBrushChooser.label_2->setVisible(false);
    uiWdgBrushChooser.deltaValueSpinBox->setVisible(false);
    uiWdgBrushChooser.sizeToStartFromSpinBox->setVisible(false);
    uiWdgBrushChooser.lblPrecisionValue->setVisible(false);
    uiWdgBrushChooser.label ->setToolTip(i18n("Use to set the size from which the Automatic Precision Setting should begin. \nThe Precision will remain 5 before this value."));
    uiWdgBrushChooser.label_2 ->setToolTip(i18n("Use to set the interval at which the Automatic Precision will change. \nThe Precision will decrease as brush size increases."));

    m_presetIsValid = true;
}


KisBrushSelectionWidget::~KisBrushSelectionWidget()
{
}

KisBrushSP KisBrushSelectionWidget::brush() const
{
    KisBrushSP theBrush;
    switch (m_buttonGroup->checkedId()) {
    case AUTOBRUSH:
        theBrush = m_autoBrushWidget->brush();
        break;
    case PREDEFINEDBRUSH:
        theBrush = m_predefinedBrushWidget->brush();
        break;
    case TEXTBRUSH:
        theBrush = m_textBrushWidget->brush();
        break;
    default:
        ;
    }
    // Fallback to auto brush if no brush selected
    // Can happen if there is no predefined brush found
    if (!theBrush)
        theBrush = m_autoBrushWidget->brush();

    return theBrush;

}


void KisBrushSelectionWidget::setAutoBrush(bool on)
{
    m_buttonGroup->button(AUTOBRUSH)->setVisible(on);
}

void KisBrushSelectionWidget::setPredefinedBrushes(bool on)
{
    m_buttonGroup->button(PREDEFINEDBRUSH)->setVisible(on);
}

void KisBrushSelectionWidget::setCustomBrush(bool on)
{
    m_buttonGroup->button(CUSTOMBRUSH)->setVisible(on);
}

void KisBrushSelectionWidget::setClipboardBrush(bool on)
{
    m_buttonGroup->button(CLIPBOARDBRUSH)->setVisible(on);
}

void KisBrushSelectionWidget::setTextBrush(bool on)
{
    m_buttonGroup->button(TEXTBRUSH)->setVisible(on);
}

void KisBrushSelectionWidget::setImage(KisImageWSP image)
{
    m_predefinedBrushWidget->setImage(image);
}

void KisBrushSelectionWidget::setCurrentBrush(KisBrushSP brush)
{
    if (!brush) {
        return;
    }
    // XXX: clever code have brush plugins know their configuration
    //      pane, so we don't have to have this if statement and
    //      have an extensible set of brush types
    if (dynamic_cast<KisAutoBrush*>(brush.data())) {
        setCurrentWidget(m_autoBrushWidget);
        m_autoBrushWidget->setBrush(brush);
    }
    else if (dynamic_cast<KisTextBrush*>(brush.data())) {
        setCurrentWidget(m_textBrushWidget);
        m_textBrushWidget->setBrush(brush);
    }
    else {
        setCurrentWidget(m_predefinedBrushWidget);
        m_predefinedBrushWidget->setBrush(brush);
    }

}

void KisBrushSelectionWidget::buttonClicked(int id)
{
    setCurrentWidget(m_chooserMap[id]);
    emit sigBrushChanged();
}

void KisBrushSelectionWidget::precisionChanged(int value)
{
    QString toolTip;

    switch (value) {
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
    uiWdgBrushChooser.sliderPrecision->blockSignals(true);
    uiWdgBrushChooser.sliderPrecision->setValue(value);
    uiWdgBrushChooser.sliderPrecision->blockSignals(false);
    uiWdgBrushChooser.sliderPrecision->setToolTip(toolTip);
    m_precisionOption.setPrecisionLevel(value);
    emit sigPrecisionChanged();
}

void KisBrushSelectionWidget::writeOptionSetting(KisPropertiesConfigurationSP settings) const
{
    m_precisionOption.writeOptionSetting(settings);
}

void KisBrushSelectionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    m_precisionOption.readOptionSetting(setting);
    uiWdgBrushChooser.sliderPrecision->setValue(m_precisionOption.precisionLevel());
    uiWdgBrushChooser.autoPrecisionCheckBox->setChecked(m_precisionOption.autoPrecisionEnabled());
    uiWdgBrushChooser.deltaValueSpinBox ->setValue(m_precisionOption.deltaValue());
    uiWdgBrushChooser.sizeToStartFromSpinBox ->setValue(m_precisionOption.sizeToStartFrom());
}

void KisBrushSelectionWidget::setPrecisionEnabled(bool value)
{
    uiWdgBrushChooser.sliderPrecision->setVisible(value);
    uiWdgBrushChooser.lblPrecision->setVisible(value);
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

void KisBrushSelectionWidget::setCurrentWidget(QWidget* widget)
{
    if (widget == m_currentBrushWidget) return;

    if (m_currentBrushWidget) {
        m_layout->removeWidget(m_currentBrushWidget);
        m_currentBrushWidget->setParent(this);
        m_currentBrushWidget->hide();
    }
    widget->setMinimumSize(m_mininmumSize);

    m_currentBrushWidget = widget;
    m_layout->addWidget(widget);

    m_currentBrushWidget->show();
    m_buttonGroup->button(m_chooserMap.key(widget))->setChecked(true);

    m_presetIsValid = (m_buttonGroup->checkedId() != CUSTOMBRUSH);
}

void KisBrushSelectionWidget::addChooser(const QString& text, QWidget* widget, int id, KoGroupButton::GroupPosition pos)
{
    KoGroupButton * button = new KoGroupButton(this);
    button->setGroupPosition(pos);
    button->setText(text);
    button->setAutoRaise(false);
    button->setCheckable(true);
    uiWdgBrushChooser.brushChooserButtonLayout->addWidget(button);

    m_buttonGroup->addButton(button, id);
    m_chooserMap[m_buttonGroup->id(button)] = widget;
    widget->hide();
}
void KisBrushSelectionWidget::setAutoPrecisionEnabled(int value)
{
    m_precisionOption.setAutoPrecisionEnabled(value);
    if(m_precisionOption.autoPrecisionEnabled())
    {
        m_precisionOption.setAutoPrecision(brush()->width());
        setPrecisionEnabled(false);
        precisionChanged(m_precisionOption.precisionLevel());
        uiWdgBrushChooser.label->setVisible(true);
        uiWdgBrushChooser.label_2->setVisible(true);
        uiWdgBrushChooser.deltaValueSpinBox->setVisible(true);
        uiWdgBrushChooser.sizeToStartFromSpinBox->setVisible(true);
        uiWdgBrushChooser.lblPrecisionValue->setVisible(true);
        uiWdgBrushChooser.lblPrecisionValue->setText("Precision:"+QString::number(m_precisionOption.precisionLevel()));

    }
    else
    {
        setPrecisionEnabled(true);
        uiWdgBrushChooser.label->setVisible(false);
        uiWdgBrushChooser.label_2->setVisible(false);
        uiWdgBrushChooser.deltaValueSpinBox->setVisible(false);
        uiWdgBrushChooser.sizeToStartFromSpinBox->setVisible(false);
        uiWdgBrushChooser.lblPrecisionValue->setVisible(false);
    }
    emit sigPrecisionChanged();
}
void KisBrushSelectionWidget::setSizeToStartFrom(double value)
{
    m_precisionOption.setSizeToStartFrom(value);
    emit sigPrecisionChanged();
}

void KisBrushSelectionWidget::setDeltaValue(double value)
{
    m_precisionOption.setDeltaValue(value);
    emit sigPrecisionChanged();

}

#include "moc_kis_brush_selection_widget.cpp"
