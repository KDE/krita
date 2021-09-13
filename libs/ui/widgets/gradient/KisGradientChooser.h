/*
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_GRADIENT_CHOOSER_H_
#define KIS_GRADIENT_CHOOSER_H_

#include <QFrame>
#include <QScopedPointer>

#include <KoDialog.h>
#include <KoColor.h>
#include <KoResource.h>
#include <kritaui_export.h>
#include <KoCanvasResourcesInterface.h>
#include <KoAbstractGradient.h>

class KisResourceItemChooser;

class KRITAUI_EXPORT KisGradientChooser : public QFrame
{
    Q_OBJECT

public:
    /**
     * @brief View modes
     */
    enum ViewMode
    {
        /**
         * @brief Show the items as a grid of icons
         */
        ViewMode_Icon,
        /**
         * @brief Show the items as a list
         */
        ViewMode_List
    };

    /**
     * @brief Item sizes
     */
    enum ItemSize
    {
        /**
         * @brief Show small items
         */
        ItemSize_Small,
        /**
         * @brief Show medium size items
         */
        ItemSize_Medium,
        /**
         * @brief Show large items
         */
        ItemSize_Large,
        /**
         * @brief Show items with a custom size
         * @see itemSizeCustom()
         * @see setItemSizeCustom(int)
         */
        ItemSize_Custom
    };

    KisGradientChooser(QWidget *parent = 0, const char *name = 0, bool useGlobalViewOptions = true);
    ~KisGradientChooser() override;

    void setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface);
    KoCanvasResourcesInterfaceSP canvasResourcesInterface() const;

    /// Gets the currently selected resource
    /// @returns the selected resource, 0 is no resource is selected
    KoResourceSP currentResource();
    void setCurrentResource(KoResourceSP resource);

    void setCurrentItem(int row);

    /**
     * @brief Load the view settings from the configuration
     * @param prefix string prepended to the settings names
     * @see saveViewSettings(const QString &)
     */
    void loadViewSettings(const QString &prefix = QString());
    /**
     * @brief Save the view settings to the configuration
     * @param prefix string prepended to the settings names
     * @see loadViewSettings(const QString &)
     */
    void saveViewSettings(const QString &prefix = QString());

    /**
     * @brief Tell if the gradient preset chooser is showing the items as a
     *        grid of icons or as a list
     * @return KisGradientChooser::ViewMode_Icon if the gradient preset
     *         chooser is showing the items as a grid of icons
     * @return KisGradientChooser::ViewMode_List if the gradient preset
     *         chooser is showing the items as a list
     * @see ViewMode
     * @see setViewMode(ViewMode)
     */
    ViewMode viewMode() const;
    /**
     * @brief Tell the size of the gradient preset chooser items
     * @return KisGradientChooser::ItemSize_Small
     *         if the items are being shown with the small size
     * @return KisGradientChooser::ItemSize_Medium 
     *         if the items are being shown with the medium size
     * @return KisGradientChooser::ItemSize_Large 
     *         if the items are being shown with the large size
     * @return KisGradientChooser::ItemSize_Custom
     *         if the items are being shown with the custom size
     * @see ItemSize
     * @see setItemSize(ItemSize)
     */
    ItemSize itemSize() const;
    /**
     * @brief Tell the size (height) in pixels used when
     *        KisGradientChooser::ItemSize_Custom is
     *        used as the item size
     * 
     *        The default value is 32
     * @return the custom size in pixels
     * @see setItemSizeCustom(int)
     */
    int itemSizeCustom() const;

    /**
     * @brief Get a pointer to the item chooser being used
     * @return A pointer to the item chooser being used 
     */
    KisResourceItemChooser* resourceItemChooser() const;

    /**
     * @brief Get if the label showing the gradient name is visible
     * @return true if the label is visible, false otherwise
     * @see setNameLabelVisible(bool)
     */
    bool isNameLabelVisible() const;
    /**
     * @brief Get if the widgets with options to edit the gradient are visible
     * @return true if the edit widgets are visible, false otherwise
     * @see setEditOptionsVisible(bool)
     */
    bool areEditOptionsVisible() const;

public Q_SLOTS:
    void slotUpdateIcons();

    /**
     * @brief Set if the gradient preset chooser must show the items as a
     *        grid of icons or as a list
     * @param newViewMode ViewMode field indicating
     *        if the gradient preset chooser must show the items as a
     *        grid of icons or as a list
     * @see ViewMode
     * @see viewMode()
     */
    void setViewMode(ViewMode newViewMode);
    /**
     * @brief Set the size of the gradient preset chooser items
     * @param newItemSize ItemSize field indicating
     *        the size of the gradient preset chooser items
     * @see ItemSize
     * @see itemSize()
     */
    void setItemSize(ItemSize newItemSize);
    /**
     * @brief Set the size (height) in pixels used when
     *        KisGradientChooser::ItemSize_Custom is
     *        used as the item size
     * @param newSize the new custom size
     * @see itemSizeCustom()
     */
    void setItemSizeCustom(int newSize);

    /**
     * @brief Set if the label showing the gradient name is visible
     * @param newNameLabelVisible true if the label must be visible,
     *                            false otherwise
     * @see isNameLabelVisible() const
     */
    void setNameLabelVisible(bool newNameLabelVisible);
    /**
     * @brief Set if the widgets with options to edit the gradient are visible
     * @param newNameLabelVisible true if the edit widgets must be visible,
     *                            false otherwise
     * @see areEditOptionsVisible() const
     */
    void setEditOptionsVisible(bool newEditOptionsVisible);

Q_SIGNALS:
    /// Emitted when a resource was selected
    void resourceSelected(KoResourceSP resource);
    /// Emitted when a resource was clicked
    void resourceClicked(KoResourceSP resource);
    /// Emitted when a resource was changed or added
    void gradientEdited(KoAbstractGradientSP resource);

private:
    class Private;
    QScopedPointer<Private> m_d;

    bool event(QEvent *e) override;
};

#endif // KIS_GRADIENT_CHOOSER_H_

