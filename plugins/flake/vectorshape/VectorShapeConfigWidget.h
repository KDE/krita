/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright 2012 Friedrich W. H. Kossebau <kossebau@kde.org>
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

#ifndef VECTORSHAPECONFIGWIDGET_H
#define VECTORSHAPECONFIGWIDGET_H

#include <KoShapeConfigWidgetBase.h>
#include <QWidget>

class VectorShape;
class KisFileNameRequester;

class VectorShapeConfigWidget : public KoShapeConfigWidgetBase
{
    Q_OBJECT
public:
    VectorShapeConfigWidget();
    ~VectorShapeConfigWidget() override;

    /// reimplemented from KoShapeConfigWidgetBase
    void open(KoShape *shape) override;
    /// reimplemented from KoShapeConfigWidgetBase
    void save() override;
    /// reimplemented from KoShapeConfigWidgetBase
    bool showOnShapeCreate() override;
    /// reimplemented from KoShapeConfigWidgetBase
    bool showOnShapeSelect() override;

private:
    VectorShape *m_shape;
    KisFileNameRequester *m_fileWidget;
};

#endif //VECTORSHAPECONFIGWIDGET_H
