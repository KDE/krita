/* This file is part of the KDE project
 * Copyright (c) 2009-2010 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KARBONFILTEREFFECTSTOOL_H
#define KARBONFILTEREFFECTSTOOL_H

#include "KoInteractionTool.h"

class KoResource;
class KoInteractionStrategy;

class KarbonFilterEffectsTool : public KoInteractionTool
{
    Q_OBJECT
public:
    enum EditMode {
        None,
        MoveAll,
        MoveLeft,
        MoveRight,
        MoveTop,
        MoveBottom
    };

    explicit KarbonFilterEffectsTool(KoCanvasBase *canvas);
    virtual ~KarbonFilterEffectsTool();

    /// reimplemented from KoToolBase
    virtual void paint(QPainter &painter, const KoViewConverter &converter);
    /// reimplemented from KoToolBase
    virtual void repaintDecorations();
    /// reimplemented from KoToolBase
    virtual void mouseMoveEvent(KoPointerEvent *event);

    /// reimplemented from KoToolBase
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape *> &shapes);

protected:
    /// reimplemented from KoToolBase
    virtual QList<QPointer<QWidget> > createOptionWidgets();
    /// reimplemented from KoToolBase
    virtual KoInteractionStrategy *createStrategy(KoPointerEvent *event);
private Q_SLOTS:
    void editFilter();
    void clearFilter();
    void filterChanged();
    void filterSelected(int index);
    void selectionChanged();
    void presetSelected(KoResource *resource);
    void regionXChanged(double x);
    void regionYChanged(double y);
    void regionWidthChanged(double width);
    void regionHeightChanged(double height);
private:
    class Private;
    Private *const d;
};

#endif // KARBONFILTEREFFECTSTOOL_H
