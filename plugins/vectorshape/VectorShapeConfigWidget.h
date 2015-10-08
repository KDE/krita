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
class KisUrlRequester;

class VectorShapeConfigWidget : public KoShapeConfigWidgetBase
{
    Q_OBJECT
public:
    VectorShapeConfigWidget();
    ~VectorShapeConfigWidget();

    /// reimplemented from KoShapeConfigWidgetBase
    virtual void open(KoShape *shape);
    /// reimplemented from KoShapeConfigWidgetBase
    virtual void save();
    /// reimplemented from KoShapeConfigWidgetBase
    virtual bool showOnShapeCreate();
    /// reimplemented from KoShapeConfigWidgetBase
    virtual bool showOnShapeSelect();

private:
    VectorShape *m_shape;
    KisUrlRequester *m_fileWidget;
};

#endif //VECTORSHAPECONFIGWIDGET_H
