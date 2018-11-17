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

#include <KoResource.h>

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
    ~KarbonFilterEffectsTool() override;

    /// reimplemented from KoToolBase
    void paint(QPainter &painter, const KoViewConverter &converter) override;
    /// reimplemented from KoToolBase
    void repaintDecorations() override;
    /// reimplemented from KoToolBase
    void mouseMoveEvent(KoPointerEvent *event) override;

    /// reimplemented from KoToolBase
    void activate(ToolActivation toolActivation, const QSet<KoShape *> &shapes) override;

protected:
    /// reimplemented from KoToolBase
    QList<QPointer<QWidget> > createOptionWidgets() override;
    /// reimplemented from KoToolBase
    KoInteractionStrategy *createStrategy(KoPointerEvent *event) override;
private Q_SLOTS:
    void editFilter();
    void clearFilter();
    void filterChanged();
    void filterSelected(int index);
    void selectionChanged();
    void presetSelected(KoResourceSP resource);
    void regionXChanged(double x);
    void regionYChanged(double y);
    void regionWidthChanged(double width);
    void regionHeightChanged(double height);
private:
    class Private;
    Private *const d;
};

#endif // KARBONFILTEREFFECTSTOOL_H
