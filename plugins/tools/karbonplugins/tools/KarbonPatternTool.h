/* This file is part of the KDE project
 * Copyright (C) 2007,2009 Jan Hambrecht <jaham@gmx.net>
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

#ifndef _KARBONPATTERNTOOL_H_
#define _KARBONPATTERNTOOL_H_

#include <KoToolBase.h>
#include <KoResource.h>

#include <QMap>

class QPainter;

class KarbonPatternEditStrategyBase;
class KarbonPatternOptionsWidget;
class KoShape;

class KarbonPatternTool : public KoToolBase
{
    Q_OBJECT
public:
    explicit KarbonPatternTool(KoCanvasBase *canvas);
    ~KarbonPatternTool() override;

    void paint(QPainter &painter, const KoViewConverter &converter) override;
    void repaintDecorations() override;

    void mousePressEvent(KoPointerEvent *event) override;
    void mouseMoveEvent(KoPointerEvent *event) override;
    void mouseReleaseEvent(KoPointerEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void activate(ToolActivation activation, const QSet<KoShape *> &shapes) override;
    void deactivate() override;

public Q_SLOTS:
    void documentResourceChanged(int key, const QVariant &res) override;

protected:
    QList<QPointer<QWidget> > createOptionWidgets() override;

private Q_SLOTS:
    void patternSelected(KoResourceSP resource);
    void initialize();
    /// updates options widget from selected pattern
    void updateOptionsWidget();
    void patternChanged();
private:
    QMap<KoShape *, KarbonPatternEditStrategyBase *> m_strategies; ///< the list of editing strategies, one for each shape
    KarbonPatternEditStrategyBase *m_currentStrategy;  ///< the current editing strategy
    KarbonPatternOptionsWidget *m_optionsWidget;
};

#endif // _KARBONPATTERNTOOL_H_
