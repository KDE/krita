/* This file is part of the KDE project
 * Copyright (C) 2007-2009 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2012 Inge Wallin <inge@lysator.liu.se>
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

#ifndef STROKEFILLWIDGET_H
#define STROKEFILLWIDGET_H

#include <QWidget>
#include <QTime>

class QToolButton;
class QStackedWidget;
class QSpacerItem;
class QGridLayout;

class StylePreview;
class StyleButtonBox;
class KoShapeStrokeModel;
class KoShapeBackground;
class KoColorBackground;
class KoResource;
class KoShape;
class KoColor;
class KoColorPopupAction;
class KoPathShape;
class KoSliderCombo;

class StrokeFillWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StrokeFillWidget(QWidget * parent = 0L);
    virtual ~StrokeFillWidget();

    void updateWidget(KoShapeStrokeModel *stroke, KoShapeBackground *fill, int opacity,
                      QColor &currentColor, int activeStyle);

    /// Useful when in changing circumstances like in a set of dock widgets
    //void locationChanged(Qt::DockWidgetArea area);
    enum StretchPolicy {
        StretchWidth,
        StretchHeight
    };
    void setStretchPolicy(StretchPolicy policy);

signals:
    /// Is emitted when stroke or fill is selected in the StylePreview widget.
    /// @param aspect KoFlake::Foreground or KoFlake::Background
    void aspectSelected(int aspect);

    // These signals are emitted when the user changes something in the UI.
    void noColorSelected();
    void fillruleChanged(Qt::FillRule fillRule);
    void colorChanged(const KoColor &color);
    void gradientChanged(KoResource *item);
    void patternChanged(KoResource *item);
    void opacityChanged(qreal opacity);

private slots:
    void fillSelected();
    void strokeSelected();
    void styleButtonPressed(int buttonId);

private:

    /// Resets color related commands which are used to combine multiple color changes
    void resetColorCommands();

    static KoShapeBackground *applyFillGradientStops(KoShape *shape, const QGradientStops &stops);
    static QBrush applyStrokeGradientStops(KoShape *shape, const QGradientStops &stops);

    void updateStyleButtons(int activeStyle);

    // Widgets
    StylePreview * m_preview;
    StyleButtonBox * m_buttons;
    QStackedWidget * m_stack;
    //KoCanvasBase * m_canvas;
    QToolButton * m_colorSelector;
    KoColorPopupAction *m_actionColor;
    QSpacerItem *m_spacer;
    QGridLayout *m_layout;
    KoSliderCombo *m_opacity;

    KoColorBackground *m_lastColorFill;
};

#endif // STROKEFILLWIDGET_H
