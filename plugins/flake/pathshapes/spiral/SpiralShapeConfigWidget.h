/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Rob Buis <buis@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef SPIRALSHAPECONFIGWIDGET_H
#define SPIRALSHAPECONFIGWIDGET_H

#include "SpiralShape.h"
#include <ui_SpiralShapeConfigWidget.h>

#include <KoShapeConfigWidgetBase.h>

class SpiralShapeConfigWidget : public KoShapeConfigWidgetBase
{
    Q_OBJECT
public:
    SpiralShapeConfigWidget();
    /// reimplemented
    void open(KoShape *shape) override;
    /// reimplemented
    void save() override;
    /// reimplemented
    bool showOnShapeCreate() override
    {
        return false;
    }
    /// reimplemented
    KUndo2Command *createCommand() override;
private:
    Ui::SpiralShapeConfigWidget widget;
    SpiralShape *m_spiral;
};

#endif // SPIRALSHAPECONFIGWIDGET_H
