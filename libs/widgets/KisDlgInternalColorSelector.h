/*
 * SPDX-FileCopyrightText: 2016 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISDLGINTERNALCOLORSELECTOR_H
#define KISDLGINTERNALCOLORSELECTOR_H

#include "kritawidgets_export.h"
#include "KoColor.h"
#include "KoColorSpace.h"
#include "KoColorDisplayRendererInterface.h"
#include "KoColorSet.h"

#include <QScopedPointer>
#include <QDialog>

#include "KisScreenColorSamplerBase.h"

class Ui_WdgDlgInternalColorSelector;
class KoColorPatch;

/**
 * @brief The KisInternalColorSelector class
 *
 * A non-modal color selector dialog that is not a plugin and can thus be used for filters.
 */
class KRITAWIDGETS_EXPORT KisDlgInternalColorSelector : public QDialog
{
    Q_OBJECT

    static std::function<KisScreenColorSamplerBase *(QWidget *)> s_screenColorSamplerFactory;

public:

    static void setScreenColorSamplerFactory(std::function<KisScreenColorSamplerBase *(QWidget *)> f) {
        s_screenColorSamplerFactory = f;
    }

    struct Config
    {
        Config() :
#ifdef Q_OS_ANDROID
            modal(false),
#else
            modal(true),
#endif
            visualColorSelector(true),
            paletteBox(true),
            screenColorSampler(true),
            prevNextButtons(true),
            hexInput(true),
            useAlpha(false){}
        bool modal;
        bool visualColorSelector;
        bool paletteBox;
        bool screenColorSampler;
        bool prevNextButtons;
        bool hexInput;
        bool useAlpha;
    };

    KisDlgInternalColorSelector(QWidget* parent, KoColor color, Config config, const QString &caption, const KoColorDisplayRendererInterface *displayRenderer = KoDumbColorDisplayRenderer::instance());
    ~KisDlgInternalColorSelector() override;

    /**
     * @brief slotColorSpaceChanged
     * Color space has changed, use this dialog to change the colorspace.
     */
    void colorSpaceChanged(const KoColorSpace *cs);

    /**
     * @brief lockUsedColorSpace
     * Lock the used colorspace of this selector.
     * @param cs
     */
    void lockUsedColorSpace(const KoColorSpace *cs);

    /**
     * @brief setDisplayRenderer
     * Set the display renderer. This is necessary for HDR color manage support.
     * @param displayRenderer
     */
    void setDisplayRenderer(const KoColorDisplayRendererInterface *displayRenderer);

    /**
     * @brief getModalColorDialog
     * Execute this dialog modally. The function returns
     * the KoColor you want.
     * @param color - The current color. Make sure this is in the color space you want your
     * end color to be in.
     * @param parent parent widget.
     * @param caption the dialog caption.
     */
    static KoColor getModalColorDialog(const KoColor color, QWidget* parent = 0, QString caption = QString());

    /**
     * @brief getCurrentColor
     * @return gives currently active color;
     */
    KoColor getCurrentColor();

    void chooseAlpha(bool chooseAlpha);

Q_SIGNALS:
    /**
     * @brief signalForegroundColorChosen
     * The most important signal. This will sent out when a color has been chosen from the selector.
     * There will be a small delay to make sure that the selector causes too many updates.
     *
     * Do not connect this to slotColorUpdated.
     * @param color The new color chosen
     */

    void signalForegroundColorChosen(KoColor color);

public Q_SLOTS:
    /**
     * @brief slotColorUpdated
     * Very important slot. Is connected to krita's resources to make sure it has
     * the currently active color. It's very important that this function is able to understand
     * when the signal came from itself.
     * @param newColor This is the new color.
     */
    void slotColorUpdated(KoColor newColor);

    /**
    * @brief slotSetColorFromPatch
    * update current color from kocolorpatch.
    * @param patch
    */
    void slotSetColorFromPatch(KoColorPatch* patch);

    /**
     * @brief setPreviousColor
     * set the previous color.
     */
    void setPreviousColor(KoColor c);

    void reject() override;

private Q_SLOTS:

    void slotSelectorModelChanged();
    void endUpdateWithNewColor();

    /**
     * @brief slotFinishUp
     * This is called when the selector is closed, for saving the current palette.
     */
    void slotFinishUp();

    /**
     * @brief slotSetColorFromHex
     * Update from the hex color input.
     */
    void slotSetColorFromHex();

    void slotChangePalette(KoColorSetSP set);

protected:
    void showEvent(QShowEvent *event) override;

private:
    void focusInEvent(QFocusEvent *) override;
    /**
     * @brief updateAllElements
     * Updates each widget with the new element, and if it's responsible for the update sents
     * a signal out that there's a new color.
     */
    void updateAllElements(QObject *source);

private:
    Ui_WdgDlgInternalColorSelector *m_ui;
    struct Private; //The private struct
    const QScopedPointer<Private> m_d; //the private pointer
};

#endif // KISDLGINTERNALCOLORSELECTOR_H
