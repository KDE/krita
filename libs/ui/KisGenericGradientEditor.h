/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISGENERICGRADIENTEDITOR_H
#define KISGENERICGRADIENTEDITOR_H

#include <QWidget>
#include <QScopedPointer>

#include <kritaui_export.h>
#include <KoAbstractGradient.h>
#include <KoCanvasResourcesInterface.h>

/**
 * @brief This is a generic gradient editor widget
 * 
 * This widget makes use of other gradient related widgets and puts them
 * together in one place to ease the edition of gradients.
 * 
 * It supports loading/saving from/to resources and conversion between
 * the different types of gradients
 */
class KRITAUI_EXPORT KisGenericGradientEditor : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief View modes for the gradient preset chooser
     */
    enum GradientPresetChooserViewMode
    {
        /**
         * @brief Show the items in the gradient preset chooser as a grid of icons
         */
        GradientPresetChooserViewMode_Icon,
        /**
         * @brief Show the items in the gradient preset chooser as a list
         */
        GradientPresetChooserViewMode_List
    };

    /**
     * @brief Item sizes for the gradient preset chooser
     */
    enum GradientPresetChooserItemSize
    {
        /**
         * @brief Show small items in the gradient preset chooser
         */
        GradientPresetChooserItemSize_Small,
        /**
         * @brief Show medium size items in the gradient preset chooser
         */
        GradientPresetChooserItemSize_Medium,
        /**
         * @brief Show large items in the gradient preset chooser
         */
        GradientPresetChooserItemSize_Large,
        /**
         * @brief Show items in the gradient preset chooser with a custom size
         * @see gradientPresetChooserItemSizeCustom()
         * @see setGradientPresetChooserItemSizeCustom(int)
         */
        GradientPresetChooserItemSize_Custom
    };

    /**
     * @brief Construct a new KisGenericGradientEditor widget
     * @param parent the parent widget
     */
    KisGenericGradientEditor(QWidget* parent = 0);
    ~KisGenericGradientEditor() override;

    /**
     * @brief Load the ui settings from the configuration
     * @param prefix string prepended to the settings names
     * @see saveUISettings(const QString &)
     */
    void loadUISettings(const QString &prefix = QString());
    /**
     * @brief Save the ui settings to the configuration
     * @param prefix string prepended to the settings names
     * @see loadUISettings(const QString &)
     */
    void saveUISettings(const QString &prefix = QString());

    /**
     * @brief Get the current gradient
     * 
     * Aclone is returned so that any changes in the returned gradient
     * won't change the editor gradient and viceversa
     * @return A clone of the current gradient
     * @see setGradient(KoAbstractGradientSP)
     */
    KoAbstractGradientSP gradient() const;
    /**
     * @brief Get the current Canvas Resources Interface
     * @return The current Canvas Resources Interface
     * @see setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP)
     */
    KoCanvasResourcesInterfaceSP canvasResourcesInterface() const;
    /**
     * @brief Tell if the compact mode is being used
     * @return true if the compact mode is being used, false otherwise
     * @see setCompactMode(bool)
     */
    bool compactMode() const;
    /**
     * @brief Tell if the convert gradient button is being shown
     * @return true if the convert gradient button is being shown, false otherwise
     * @see setConvertGradientButtonVisible(bool)
     */
    bool isConvertGradientButtonVisible() const;
    /**
     * @brief Tell if the update gradient button is being shown
     * @return true if the update gradient button is being shown, false otherwise
     * @see setUpdateGradientButtonVisible(bool)
     */
    bool isUpdateGradientButtonVisible() const;
    /**
     * @brief Tell if the add gradient button is being shown
     * @return true if the add gradient button is being shown, false otherwise
     * @see setAddGradientButtonVisible(bool)
     */
    bool isAddGradientButtonVisible() const;
    /**
     * @brief Tell if the gradient preset chooser is being shown
     * @return true if the gradient preset chooser is being shown, false otherwise
     * @see setGradientPresetChooserVisible(bool)
     */
    bool isGradientPresetChooserVisible() const;
    /**
     * @brief Tell if the button for the gradient preset chooser options is being shown
     * @return true if the button for the gradient preset chooser options is being shown, false otherwise
     * @see setGradientPresetChooserOptionsButtonVisible(bool)
     */
    bool isGradientPresetChooserOptionsButtonVisible() const;
    /**
     * @brief Tell if the gradient preset chooser is being shown as a pop-up
     *        clicking a button or embedded in the widget ui
     * @return true if the gradient preset chooser is being shown as a pop-up, false otherwise
     * @see setGradientPresetChooserVisible(bool)
     */
    bool useGradientPresetChooserPopUp() const;
    /**
     * @brief Tell if the gradient preset chooser is being shown without any
     *        controls other than the list view
     * @return true if the gradient preset chooser is being shown without any
     *         controls other than the list view, false otherwise
     * @see setCompactGradientPresetChooserMode(bool)
     */
    bool compactGradientPresetChooserMode() const;
    /**
     * @brief Tell if the gradient preset chooser is showing the items as a
     *        grid of icons or as a list
     * @return KisGenericGradientEditor::GradientPresetChooserViewMode_Icon if the gradient preset
     *         chooser is showing the items as a grid of icons
     * @return KisGenericGradientEditor::GradientPresetChooserViewMode_List if the gradient preset
     *         chooser is showing the items as a list
     * @see GradientPresetChooserViewMode
     * @see setGradientPresetChooserViewMode(GradientPresetChooserViewMode)
     */
    GradientPresetChooserViewMode gradientPresetChooserViewMode() const;
    /**
     * @brief Tell the size of the gradient preset chooser items
     * @return KisGenericGradientEditor::GradientPresetChooserItemSize_Small
     *         if the items are being shown with the small size
     * @return KisGenericGradientEditor::GradientPresetChooserItemSize_Medium 
     *         if the items are being shown with the medium size
     * @return KisGenericGradientEditor::GradientPresetChooserItemSize_Large 
     *         if the items are being shown with the large size
     * @return KisGenericGradientEditor::GradientPresetChooserItemSize_Custom
     *         if the items are being shown with the custom size
     * @see GradientPresetChooserItemSize
     * @see setGradientPresetChooserItemSize(GradientPresetChooserItemSize)
     */
    GradientPresetChooserItemSize gradientPresetChooserItemSize() const;
    /**
     * @brief Tell the underlying size (height) in pixels used when
     *        KisGenericGradientEditor::GradientPresetChooserItemSize_Small is
     *        used as the item size
     * 
     *        The default value is 32
     * @return the underlying small size in pixels
     * @see setGradientPresetChooserItemSizeSmall(int)
     */
    int gradientPresetChooserItemSizeSmall() const;
    /**
     * @brief Tell the underlying size (height) in pixels used when
     *        KisGenericGradientEditor::GradientPresetChooserItemSize_Medium is
     *        used as the item size
     * 
     *        The default value is 48
     * @return the underlying medium size in pixels
     * @see setGradientPresetChooserItemSizeMedium(int)
     */
    int gradientPresetChooserItemSizeMedium() const;
    /**
     * @brief Tell the underlying size (height) in pixels used when
     *        KisGenericGradientEditor::GradientPresetChooserItemSize_Large is
     *        used as the item size
     * 
     *        The default value is 64
     * @return the underlying large size in pixels
     * @see setGradientPresetChooserItemSizeLarge(int)
     */
    int gradientPresetChooserItemSizeLarge() const;
    /**
     * @brief Tell the size (height) in pixels used when
     *        KisGenericGradientEditor::GradientPresetChooserItemSize_Custom is
     *        used as the item size
     * 
     *        The default value is 32
     * @return the custom size in pixels
     * @see setGradientPresetChooserItemSizeCustom(int)
     */
    int gradientPresetChooserItemSizeCustom() const;
    /**
     * @brief Tell the factor used to obtain the width of the items icons from
     *        the current item size
     * 
     *        The default value is 2.0
     * @return the factor used to obtain the width of the items
     * @see setGradientPresetChooserItemSizeWidthFactor(qreal)
     */
    qreal gradientPresetChooserItemSizeWidthFactor() const;
    /**
     * @brief Tell if the internal gradient editor is using the compact mode
     * @return true if the internal gradient editor is using the compact mode, false otherwise
     * @see setCompactGradientEditorMode(bool)
     */
    bool compactGradientEditorMode() const;

public Q_SLOTS:
    /**
     * @brief Set the gradient
     * 
     * This editor makes a clone os the gradient passed so that any changes
     * made in the editor won't change the passed gradient and viceversa
     * @param newGradient The new gradient
     * @see gradient()
     */
    void setGradient(KoAbstractGradientSP newGradient);
    /**
     * @brief Set the canvas resources interface
     * 
     * the canvas resources interface is used to get the
     * current foreground and background colors
     * 
     * @param newCanvasResourcesInterface the new canvas resources interface
     * @see canvasResourcesInterface()
     */
    void setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP newCanvasResourcesInterface);
    /**
     * @brief Set if the editor must show a reduced ui
     * 
     * If the compact mode is set, only the gradient editors will be visible
     * 
     * @param compact true if the compact mode must be used, false otherwise
     * @see compactMode()
     */
    void setCompactMode(bool compact);
    /**
     * @brief Set if the convert gradient button must be shown
     * @param visible true if the convert gradient button must be shown, false otherwise
     * @see isConvertGradientButtonVisible()
     */
    void setConvertGradientButtonVisible(bool visible);
    /**
     * @brief Set if the update gradient button must be shown
     * @param visible true if the update gradient button must be shown, false otherwise
     * @see isUpdateGradientButtonVisible()
     */
    void setUpdateGradientButtonVisible(bool visible);
    /**
     * @brief Set if the add gradient button must be shown
     * @param visible true if the add gradient button must be shown, false otherwise
     * @see isAddGradientButtonVisible()
     */
    void setAddGradientButtonVisible(bool visible);
    /**
     * @brief Set if the gradient preset chooser must be shown
     * @param visible true if the gradient preset chooser must be shown, false otherwise
     * @see isSaveGradientButtonVisible()
     */
    void setGradientPresetChooserVisible(bool visible);
    /**
     * @brief Set if the button for the gradient preset chooser options must be shown
     * @param visible true if the button for the gradient preset chooser options must be shown, false otherwise
     * @see isGradientPresetChooserOptionsButtonVisible()
     */
    void setGradientPresetChooserOptionsButtonVisible(bool visible);
    /**
     * @brief Set if the gradient preset chooser must be shown as a pop-up
     *        clicking a button or embedded in the widget ui
     * @param use true if the gradient preset chooser must be shown as a pop-up, false otherwise
     * @see useGradientPresetChooserPopUp()
     */
    void setUseGradientPresetChooserPopUp(bool use);
    /**
     * @brief Set if the gradient preset chooser must be shown without any
     *        controls other than the list view
     * @param compact true if the gradient preset chooser must be shown without any
     *        controls other than the list view, false otherwise
     * @see compactGradientPresetChooserMode()
     */
    void setCompactGradientPresetChooserMode(bool compact);
    /**
     * @brief Set if the gradient preset chooser must show the items as a
     *        grid of icons or as a list
     * @param newViewMode GradientPresetChooserViewMode field indicating
     *        if the gradient preset chooser must show the items as a
     *        grid of icons or as a list
     * @see GradientPresetChooserViewMode
     * @see gradientPresetChooserViewMode()
     */
    void setGradientPresetChooserViewMode(GradientPresetChooserViewMode newViewMode);
    /**
     * @brief Set the size of the gradient preset chooser items
     * @param newItemSize GradientPresetChooserItemSize field indicating
     *        the size of the gradient preset chooser items
     * @see GradientPresetChooserItemSize
     * @see gradientPresetChooserItemSize()
     */
    void setGradientPresetChooserItemSize(GradientPresetChooserItemSize newItemSize);
    /**
     * @brief Set the underlying size (height) in pixels used when
     *        KisGenericGradientEditor::GradientPresetChooserItemSize_Small is
     *        used as the item size
     * @param newSize the new underlying small size
     * @see gradientPresetChooserItemSizeSmall()
     */
    void setGradientPresetChooserItemSizeSmall(int newSize);
    /**
     * @brief Set the underlying size (height) in pixels used when
     *        KisGenericGradientEditor::GradientPresetChooserItemSize_Medium is
     *        used as the item size
     * @param newSize the new underlying medium size
     * @see gradientPresetChooserItemSizeMedium()
     */
    void setGradientPresetChooserItemSizeMedium(int newSize);
    /**
     * @brief Set the underlying size (height) in pixels used when
     *        KisGenericGradientEditor::GradientPresetChooserItemSize_Large is
     *        used as the item size
     * @param newSize the new underlying large size
     * @see gradientPresetChooserItemSizeLarge()
     */
    void setGradientPresetChooserItemSizeLarge(int newSize);
    /**
     * @brief Set the size (height) in pixels used when
     *        KisGenericGradientEditor::GradientPresetChooserItemSize_Custom is
     *        used as the item size
     * @param newSize the new custom size
     * @see gradientPresetChooserItemSizeCustom()
     */
    void setGradientPresetChooserItemSizeCustom(int newSize);
    /**
     * @brief Set the factor used to obtain the width of the items icons from
     *        the current item size
     * @param newFactor the factor used to obtain the width of the items
     * @see gradientPresetChooserItemSizeWidthFactor()
     */
    void setGradientPresetChooserItemSizeWidthFactor(qreal newFactor);
    /**
     * @brief Set if the internal gradient editor must use the compact mode
     * @param compact true if the internal gradient editor must use the compact mode, false otherwise
     * @see compactGradientEditorMode()
     */
    void setCompactGradientEditorMode(bool compact);

Q_SIGNALS:
    /**
     * @brief signal emitted when the gradient changes
     */
    void sigGradientChanged();

private:
    class Private;
    QScopedPointer<Private> m_d;

    bool event(QEvent *e) override;

    void updateConvertGradientButton();
    void updateUpdateGradientButton();
    void updateAddGradientButton();
    void updateGradientPresetChooser();
    void updateGradientEditor();
    void updateWidgetSliderGradientPresetChooserItemSizeCustom();

private Q_SLOTS:
    void on_buttonConvertGradient_clicked();
    void on_buttonUpdateGradient_clicked();
    void on_buttonAddGradient_clicked();
    void on_widgetGradientPresetChooser_resourceClicked(KoResourceSP resoure);
    void on_actionGroupGradientPresetChooserViewMode_triggered(QAction *triggeredAction);
    void on_actionGroupGradientPresetChooserItemSize_triggered(QAction *triggeredAction);
    void on_sliderGradientPresetChooserItemSizeCustom_valueChanged(int newValue);
    void on_widgetGradientEditor_sigGradientChanged();
};

#endif
