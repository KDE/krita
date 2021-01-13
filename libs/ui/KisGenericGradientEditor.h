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

    void updateConvertGradientButton();
    void updateUpdateGradientButton();
    void updateAddGradientButton();
    void updateGradientPresetChooser();
    void updateGradientEditor();

private Q_SLOTS:
    void on_buttonConvertGradient_clicked();
    void on_buttonUpdateGradient_clicked();
    void on_buttonAddGradient_clicked();
    void on_widgetGradientPresetChooser_resourceClicked(KoResourceSP resource);
    void on_widgetGradientEditor_sigGradientChanged();
};

#endif
