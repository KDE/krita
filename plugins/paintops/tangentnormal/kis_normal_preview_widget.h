/* This file is part of the KDE project
 *
 * Copyright (C) 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KISNORMALPREVIEWWIDGET_H
#define KISNORMALPREVIEWWIDGET_H

#include <QLabel>

/* This is a widget that takes an image of a normal semi-sphere, and inverts the channels based on the values
 * of m_redChannel, m_greenChannel and m_blueChannel, this is used to give feedback on a set of normal-swizzles.
 * setRedChannel, setGreenChannel and setBlueChannel automatically updates the image with the new channel configuration.
 */

class KisNormalPreviewWidget : public QLabel
{
    Q_OBJECT
public:

    KisNormalPreviewWidget(QWidget* parent = 0);
    ~KisNormalPreviewWidget() override;

public Q_SLOTS:
    /* for the following functions 0=X+, 1=X-, 2=Y+, 3=Y-, 4=Z+ and 5=Z-*/
    void setRedChannel(int index);
    void setGreenChannel(int index);
    void setBlueChannel(int index);

private:

    void updateImage();
    QImage swizzleTransformPreview (QImage preview);
    int previewTransform(int const horizontal, int const vertical, int const depth, int index, int maxvalue);

    int m_redChannel;
    int m_greenChannel;
    int m_blueChannel;
    QString m_fileName;
    int m_previewSize;
};

#endif // KISNORMALPREVIEWWIDGET_H
