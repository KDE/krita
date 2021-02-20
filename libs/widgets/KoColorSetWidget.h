/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 C. Boemann <cbo@boemann.dk>
   SPDX-FileCopyrightText: 2007 Fredy Yanardi <fyanardi@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOCOLORSETWIDGET_H_
#define KOCOLORSETWIDGET_H_

#include <QWidgetAction>
#include <QFrame>

#include <KisSwatch.h>
#include <KoColorSet.h>

#include <KoColorDisplayRendererInterface.h>

#include "kritawidgets_export.h"

class KoColor;
class KoColorPatch;

/**
 * @short A colormanaged widget for choosing a color from a colorset
 *
 * KoColorSetWidget is a widget for choosing a color (colormanaged via pigment). It shows a color
 * set plus optionally a checkbox to filter away bad matching colors.
 */
class KRITAWIDGETS_EXPORT KoColorSetWidget : public QFrame
{

    Q_OBJECT

public:

    /**
     * Constructor for the widget, where color is initially blackpoint of sRGB
     *
     * @param parent parent QWidget
     */
    explicit KoColorSetWidget(QWidget *parent=0);

    /**
     * Destructor
     */
    ~KoColorSetWidget() override;

    /**
     * Sets the color set that this widget shows.
     * @param colorSet pointer to the color set
     */
    void setColorSet(KoColorSetSP colorSet);

    /**
     * @brief setDisplayRenderer
     * Set the display renderer of this object.
     * @param displayRenderer
     */
    void setDisplayRenderer(const KoColorDisplayRendererInterface *displayRenderer);

    /**
     * Gets the current color set
     * @returns current color set,, 0 if none set
     */
    KoColorSetSP colorSet();

protected:
    void resizeEvent(QResizeEvent *event) override; ///< reimplemented from QFrame

Q_SIGNALS:

    /**
     * Emitted every time the color changes (by calling setColor() or
     * by user interaction.
     * @param color the new color
     * @param final if the value is final (ie not produced by the pointer moving over around)
     */
    void colorChanged(const KoColor &color, bool final);

    /**
     * Emitted every time the size of this widget changes because of new colorset with
     * different number of colors is loaded. This is useful for KoColorSetAction to update
     * correct size of the menu showing this widget.
     * @param size the new size
     */
    void widgetSizeChanged(const QSize &size);

private Q_SLOTS:
    /**
     * @brief slotPatchTriggered
     * Triggered when a recent patch is triggered
     */
    void slotPatchTriggered(KoColorPatch *);
    /**
     * @brief slotEntrySelected
     * Triggered when a color is choose from the palette view
     */
    void slotColorSelectedByPalette(const KoColor &color);
    void slotPaletteChoosen(KoColorSetSP );
    void slotNameListSelection(const KoColor &);

private:
    class KoColorSetWidgetPrivate;
    KoColorSetWidgetPrivate * const d;
};

#endif
