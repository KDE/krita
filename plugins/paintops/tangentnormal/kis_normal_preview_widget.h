/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
};

#endif // KISNORMALPREVIEWWIDGET_H
