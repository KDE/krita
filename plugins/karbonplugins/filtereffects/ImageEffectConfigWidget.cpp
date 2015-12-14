/* This file is part of the KDE project
 * Copyright (c) 2010 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ImageEffectConfigWidget.h"
#include "ImageEffect.h"

#include "KoFilterEffect.h"
#include <KoFileDialog.h>

#include <klocalizedstring.h>

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QImageReader>
#include <KConfigGroup>

ImageEffectConfigWidget::ImageEffectConfigWidget(QWidget *parent)
    : KoFilterEffectConfigWidgetBase(parent)
    , m_effect(0)
{
    QGridLayout *g = new QGridLayout(this);

    m_image = new QLabel(this);
    QPushButton *button = new QPushButton(i18n("Select image..."), this);

    g->addWidget(m_image, 0, 0, Qt::AlignCenter);
    g->addWidget(button, 0, 1);

    setLayout(g);

    connect(button, SIGNAL(clicked()), this, SLOT(selectImage()));
}

bool ImageEffectConfigWidget::editFilterEffect(KoFilterEffect *filterEffect)
{
    m_effect = dynamic_cast<ImageEffect *>(filterEffect);
    if (!m_effect) {
        return false;
    }

    m_image->setPixmap(QPixmap::fromImage(m_effect->image().scaledToWidth(80)));

    return true;
}

void ImageEffectConfigWidget::selectImage()
{
    if (!m_effect) {
        return;
    }

    QStringList imageFilter;
    // add filters for all formats supported by QImage
    Q_FOREACH (const QByteArray &format, QImageReader::supportedImageFormats()) {
        imageFilter << QString("image/") + format;
    }

    KoFileDialog dialog(0, KoFileDialog::OpenFile, "OpenDocument");
    dialog.setCaption(i18n("Select image"));
    dialog.setImageFilters();

    QString fname = dialog.filename();

    if (fname.isEmpty()) {
        return;
    }

    QImage newImage;
    if (!newImage.load(fname)) {
        return;
    }

    m_effect->setImage(newImage);
    editFilterEffect(m_effect);

    emit filterChanged();
}
