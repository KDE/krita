/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2009 Sven Langkamp <sven.langkamp@gmail.com>
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
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

#include "kis_brush_chooser.h"

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

#include <KoResourceItemChooser.h>

#include <kis_icon.h>
#include "kis_brush_registry.h"
#include "kis_brush_server.h"
#include "widgets/kis_slider_spin_box.h"
#include "widgets/kis_multipliers_double_slider_spinbox.h"
#include "kis_spacing_selection_widget.h"
#include "kis_signals_blocker.h"

#include "kis_custom_brush_widget.h"
#include "kis_clipboard_brush_widget.h"

#include "kis_global.h"
#include "kis_gbr_brush.h"
#include "kis_debug.h"
#include "kis_image.h"

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

    KisBrush *brush = static_cast<KisBrush*>(index.internalPointer());

    QRect itemRect = option.rect;
    QImage thumbnail = brush->image();

    if (thumbnail.height() > itemRect.height() || thumbnail.width() > itemRect.width()) {
        thumbnail = thumbnail.scaled(itemRect.size() , Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    painter->save();
    int dx = (itemRect.width() - thumbnail.width()) / 2;
    int dy = (itemRect.height() - thumbnail.height()) / 2;
    painter->drawImage(itemRect.x() + dx, itemRect.y() + dy, thumbnail);

    if (option.state & QStyle::State_Selected) {
        painter->setPen(QPen(option.palette.highlight(), 2.0));
        painter->drawRect(option.rect);
        painter->setCompositionMode(QPainter::CompositionMode_HardLight);
        painter->setOpacity(0.65);
        painter->fillRect(option.rect, option.palette.highlight());
    }

    painter->restore();
}


KisPredefinedBrushChooser::KisPredefinedBrushChooser(QWidget *parent, const char *name)
    : QWidget(parent),
      m_stampBrushWidget(0),
      m_clipboardBrushWidget(0)
{
    setObjectName(name);

    setupUi(this);

    brushSizeSpinBox->setRange(0, 1000, 2);
    brushSizeSpinBox->setValue(5);
    brushSizeSpinBox->setExponentRatio(3.0);
    brushSizeSpinBox->setSuffix(i18n(" px"));
    brushSizeSpinBox->setExponentRatio(3.0);

    QObject::connect(brushSizeSpinBox, SIGNAL(valueChanged(qreal)), this, SLOT(slotSetItemSize(qreal)));


    brushRotationSpinBox->setRange(0, 360, 0);
    brushRotationSpinBox->setValue(0);
    brushRotationSpinBox->setSuffix(QChar(Qt::Key_degree));
    QObject::connect(brushRotationSpinBox, SIGNAL(valueChanged(qreal)), this, SLOT(slotSetItemRotation(qreal)));

    brushSpacingSelectionWidget->setSpacing(true, 1.0);
    connect(brushSpacingSelectionWidget, SIGNAL(sigSpacingChanged()), SLOT(slotSpacingChanged()));


    QObject::connect(useColorAsMaskCheckbox, SIGNAL(toggled(bool)), this, SLOT(slotSetItemUseColorAsMask(bool)));


    KisBrushResourceServer* rServer = KisBrushServer::instance()->brushServer();
    QSharedPointer<KisBrushResourceServerAdapter> adapter(new KisBrushResourceServerAdapter(rServer));


    m_itemChooser = new KoResourceItemChooser(adapter, this);

    m_itemChooser->showTaggingBar(true);
    m_itemChooser->setColumnCount(10);
    m_itemChooser->setRowHeight(30);
    m_itemChooser->setItemDelegate(new KisBrushDelegate(this));
    m_itemChooser->setCurrentItem(0, 0);
    m_itemChooser->setSynced(true);
    m_itemChooser->setMinimumWidth(100);
    m_itemChooser->setMinimumHeight(100);



    presetsLayout->addWidget(m_itemChooser);


    connect(m_itemChooser, SIGNAL(resourceSelected(KoResource *)), this, SLOT(update(KoResource *)));

    //QPushButton *stampButton = new QPushButton(KisIconUtils::loadIcon("list-add"), i18n("Stamp"), this);

    stampButton->setIcon(KisIconUtils::loadIcon("list-add"));
    stampButton->setToolTip(i18n("Creates a brush tip from the current image selection."
                               "\n If no selection is present the whole image will be used."));

    clipboardButton->setIcon(KisIconUtils::loadIcon("edit-paste"));
    clipboardButton->setToolTip(i18n("Creates a brush tip from the image in the clipboard."));


    connect(stampButton, SIGNAL(clicked()), this,  SLOT(slotOpenStampBrush()));
    connect(clipboardButton, SIGNAL(clicked()), SLOT(slotOpenClipboardBrush()));

    QGridLayout *spacingLayout = new QGridLayout();
    spacingLayout->setObjectName("spacing grid layout");

    resetBrushButton->setToolTip(i18n("Reloads Spacing from file\nSets Scale to 1.0\nSets Rotation to 0.0"));
    connect(resetBrushButton, SIGNAL(clicked()), SLOT(slotResetBrush()));

    update(m_itemChooser->currentResource());
}

KisPredefinedBrushChooser::~KisPredefinedBrushChooser()
{
}

void KisPredefinedBrushChooser::setBrush(KisBrushSP _brush)
{
    m_itemChooser->setCurrentResource(_brush.data());
    update(_brush.data());
}

void KisPredefinedBrushChooser::slotResetBrush()
{
    KisBrush *brush = dynamic_cast<KisBrush *>(m_itemChooser->currentResource());
    if (brush) {
        brush->load();
        brush->setScale(1.0);
        brush->setAngle(0.0);

        slotActivatedBrush(brush);
        update(brush);
        emit sigBrushChanged();
    }
}

void KisPredefinedBrushChooser::slotSetItemSize(qreal sizeValue)
{
    KisBrush *brush = dynamic_cast<KisBrush *>(m_itemChooser->currentResource());

    if (brush) {
        int brushWidth = brush->width();

        brush->setScale(sizeValue / qreal(brushWidth));
        slotActivatedBrush(brush);
        emit sigBrushChanged();
    }
}

void KisPredefinedBrushChooser::slotSetItemRotation(qreal rotationValue)
{
    KisBrush *brush = dynamic_cast<KisBrush *>(m_itemChooser->currentResource());
    if (brush) {
        brush->setAngle(rotationValue / 180.0 * M_PI);
        slotActivatedBrush(brush);

        emit sigBrushChanged();
    }
}

void KisPredefinedBrushChooser::slotSpacingChanged()
{
    KisBrush *brush = dynamic_cast<KisBrush *>(m_itemChooser->currentResource());
    if (brush) {
        brush->setSpacing(brushSpacingSelectionWidget->spacing());
        brush->setAutoSpacing(brushSpacingSelectionWidget->autoSpacingActive(), brushSpacingSelectionWidget->autoSpacingCoeff());
        slotActivatedBrush(brush);

        emit sigBrushChanged();
    }
}

void KisPredefinedBrushChooser::slotSetItemUseColorAsMask(bool useColorAsMask)
{
    KisGbrBrush *brush = dynamic_cast<KisGbrBrush *>(m_itemChooser->currentResource());
    if (brush) {
        brush->setUseColorAsMask(useColorAsMask);
        slotActivatedBrush(brush);

        emit sigBrushChanged();
    }
}

void KisPredefinedBrushChooser::slotOpenStampBrush()
{
    if(!m_stampBrushWidget) {
        m_stampBrushWidget = new KisCustomBrushWidget(this, i18n("Stamp"), m_image);
        m_stampBrushWidget->setModal(false);
        connect(m_stampBrushWidget, SIGNAL(sigNewPredefinedBrush(KoResource *)),
                                    SLOT(slotNewPredefinedBrush(KoResource *)));
    }

    QDialog::DialogCode result = (QDialog::DialogCode)m_stampBrushWidget->exec();

    if(result) {
        update(m_itemChooser->currentResource());
    }
}
void KisPredefinedBrushChooser::slotOpenClipboardBrush()
{
    if(!m_clipboardBrushWidget) {
        m_clipboardBrushWidget = new KisClipboardBrushWidget(this, i18n("Clipboard"), m_image);
        m_clipboardBrushWidget->setModal(true);
        connect(m_clipboardBrushWidget, SIGNAL(sigNewPredefinedBrush(KoResource *)),
                                        SLOT(slotNewPredefinedBrush(KoResource *)));
    }

    QDialog::DialogCode result = (QDialog::DialogCode)m_clipboardBrushWidget->exec();

    if(result) {
        update(m_itemChooser->currentResource());
    }
}

void KisPredefinedBrushChooser::update(KoResource * resource)
{
    KisBrush* brush = dynamic_cast<KisBrush*>(resource);

    if (brush) {


        brushTipNameLabel->setText(i18n(brush->name().toUtf8().data()));

        //find out if it is an animated tip...
        qDebug() << "brush type: " << QString::number(brush->brushType()); // 3 means it has multi-tips

        QString brushTypeString = "";

        if (brush->brushType() == INVALID) {
            brushTypeString = i18n("Invalid");
        } else if (brush->brushType() == MASK) {
            brushTypeString = i18n("Mask");
        } else if (brush->brushType() == IMAGE) {
            brushTypeString = i18n("GBR");
        } else if (brush->brushType() == PIPE_MASK ) {
            brushTypeString = i18n("Animated Mask");
        } else if (brush->brushType() == PIPE_IMAGE ) {
            brushTypeString = i18n("Animated Image");
        }


        QString brushDetailsText = QString("%1 (%2 x %3)")
                       .arg(brushTypeString)
                       .arg(brush->width())
                       .arg(brush->height());

        brushDetailsLabel->setText(brushDetailsText);


        brushSpacingSelectionWidget->setSpacing(brush->autoSpacingActive(),
                                brush->autoSpacingActive() ?
                                brush->autoSpacingCoeff() : brush->spacing());

        brushRotationSpinBox->setValue(brush->angle() * 180 / M_PI);
        brushSizeSpinBox->setValue(brush->width() * brush->scale());

        // useColorAsMask support is only in gimp brush so far
        KisGbrBrush *gimpBrush = dynamic_cast<KisGbrBrush*>(resource);
        if (gimpBrush) {
            useColorAsMaskCheckbox->setChecked(gimpBrush->useColorAsMask());
        }
        useColorAsMaskCheckbox->setEnabled(brush->hasColor() && gimpBrush);

        slotActivatedBrush(brush);
        emit sigBrushChanged();
    }
}

void KisPredefinedBrushChooser::slotActivatedBrush(KoResource * resource)
{
    KisBrush* brush = dynamic_cast<KisBrush*>(resource);

    if (m_brush != brush) {
        if (m_brush) {
            m_brush->clearBrushPyramid();
        }

        m_brush = brush;

        if (m_brush) {
            m_brush->prepareBrushPyramid();
        }
    }
}

void KisPredefinedBrushChooser::slotNewPredefinedBrush(KoResource *resource)
{
    m_itemChooser->setCurrentResource(resource);
    update(resource);
}

void KisPredefinedBrushChooser::setBrushSize(qreal xPixels, qreal yPixels)
{
    Q_UNUSED(yPixels);
    qreal oldWidth = m_brush->width() * m_brush->scale();
    qreal newWidth = oldWidth + xPixels;

    newWidth = qMax(newWidth, qreal(0.1));

    brushSizeSpinBox->setValue(newWidth);
}

void KisPredefinedBrushChooser::setImage(KisImageWSP image)
{
    m_image = image;
}

#include "moc_kis_brush_chooser.cpp"


