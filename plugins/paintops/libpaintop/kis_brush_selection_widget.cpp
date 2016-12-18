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
    button->setAutoRaise(true);
    button->setCheckable(true);
    uiWdgBrushChooser.brushChooserButtonLayout->addWidget(button);

    m_buttonGroup->addButton(button, id);
    m_chooserMap[m_buttonGroup->id(button)] = widget;
    widget->hide();
}


#include "moc_kis_brush_selection_widget.cpp"
