/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISENCLOSEANDFILLPAINTER_H
#define KISENCLOSEANDFILLPAINTER_H

#include <QVector>
#include <QPoint>
#include <QRect>
#include <QScopedPointer>

#include <kis_pixel_selection.h>
#include <kis_paint_device.h>
#include <kis_fill_painter.h>
#include <KoColor.h>
#include <kritaimage_export.h>

class KRITAIMAGE_EXPORT KisEncloseAndFillPainter : public KisFillPainter
{
public:
    /**
     * Different methods of selecting pixels when using flood fill related
     * functionality
     */
    enum RegionSelectionMethod
    {
        /**
         * Select all the closed regions inside the enclosing region, no matter
         * their color
         */
        SelectAllRegions,
        /**
         * Select all the closed regions inside the enclosing region, as long as
         * they are the same color as the specified region selection color
         */
        SelectRegionsFilledWithSpecificColor,
        /**
         * Select all the closed regions inside the enclosing region, as long as
         * they are transparent
         */
        SelectRegionsFilledWithTransparent,
        /**
         * Select all the closed regions inside the enclosing region, as long as
         * they are the same color as the specified region selection color
         * or transparent
         */
        SelectRegionsFilledWithSpecificColorOrTransparent,
        /**
         * Select all the closed regions inside the enclosing region, as long as
         * they are not the same color as the specified region selection color
         */
        SelectAllRegionsExceptFilledWithSpecificColor,
        /**
         * Select all the closed regions inside the enclosing region, as long as
         * they are not transparent
         */
        SelectAllRegionsExceptFilledWithTransparent,
        /**
         * Select all the closed regions inside the enclosing region, as long as
         * they are not the same color as the specified region selection color
         * or transparent
         */
        SelectAllRegionsExceptFilledWithSpecificColorOrTransparent,
        /**
         * Select all the closed regions inside the enclosing region, as long as
         * they are surrounded by the region selection color
         */
        SelectRegionsSurroundedBySpecificColor,
        /**
         * Select all the closed regions inside the enclosing region, as long as
         * they are surrounded by transparent
         */
        SelectRegionsSurroundedByTransparent,
        /**
         * Select all the closed regions inside the enclosing region, as long as
         * they are surrounded by the region selection color or transparent
         */
        SelectRegionsSurroundedBySpecificColorOrTransparent
    };

    /**
     * Construct an empty painter. Use the begin(KisPaintDeviceSP) method to attach
     * to a paint device
     */
    KisEncloseAndFillPainter();
    /**
     * Start painting on the specified paint device
     */
    KisEncloseAndFillPainter(KisPaintDeviceSP device);

    KisEncloseAndFillPainter(KisPaintDeviceSP device, KisSelectionSP selection);

    ~KisEncloseAndFillPainter() override;

    /**
     * Finds the closed areas inside the enclosing mask and fills them with the
     * set color. If there is a selection, the whole selection is filled. Note
     * that you must have set the width and height on the painter if you
     * don't have a selection.
     *
     * @param enclosingMask a mask representing where to search for the closed regions
     * @param referenceDevice the reference device that determines the areas that
     * are filled if sampleMerged is on
     */
    void encloseAndFillColor(KisPixelSelectionSP enclosingMask, KisPaintDeviceSP referenceDevice);

    /**
     * Finds the closed areas inside the enclosing mask and fills them with the
     * set pattern. If there is a selection, the whole selection is filled. Note
     * that you must have set the width and height on the painter if you
     * don't have a selection.
     *
     * @param enclosingMask a mask representing where to search for the closed regions
     * @param referenceDevice the reference device that determines the areas that
     * are filled if sampleMerged is on
     * @param patternTransform transform applied to the pattern
     */
    void encloseAndFillPattern(KisPixelSelectionSP enclosingMask,
                               KisPaintDeviceSP referenceDevice,
                               QTransform patternTransform = QTransform());

    /**
     * Returns a selection mask for the closed regions inside of the enclosing mask.
     * This variant basically creates a new selection object and passes it down
     * to the other variant of the function.
     *
     * @param enclosingMask a mask representing where to search for the closed regions
     * @param referenceDevice the reference device that determines the area that
     * is floodfilled if sampleMerged is on
     * @param existingSelection the selection used when useSelectionAsBoundary is true
     */
    KisPixelSelectionSP createEncloseAndFillSelection(KisPixelSelectionSP enclosingMask,
                                                      KisPaintDeviceSP referenceDevice,
                                                      KisPixelSelectionSP existingSelection);

    /**
     * Returns a selection mask for the closed regions inside of the enclosing mask.
     * This variant requires an empty selection object. It is used in cases where the pointer
     * to the selection must be known beforehand, for example when the selection is filled
     * in a stroke and then the pointer to the pixel selection is needed later.
     *
     * @param selection empty new selection object
     * @param enclosingMask a mask representing where to search for the closed regions
     * @param referenceDevice the reference device that determines the area that
     * is floodfilled if sampleMerged is on
     * @param existingSelection the selection used when useSelectionAsBoundary is true
     */
    KisPixelSelectionSP createEncloseAndFillSelection(KisPixelSelectionSP newSelection,
                                                      KisPixelSelectionSP enclosingMask,
                                                      KisPaintDeviceSP referenceDevice,
                                                      KisPixelSelectionSP existingSelection);

    /** Sets the region type of the closed regions in the enclose and fill */
    void setRegionSelectionMethod(RegionSelectionMethod regionSelectionMethod);

    /** Gets the region type of the closed regions in the enclose and fill */
    RegionSelectionMethod regionSelectionMethod() const;

    /** Sets the color to use when regionSelectionMethod is set to
     *  custom color in the enclose and fill
     */
    void setRegionSelectionColor(const KoColor &color);

    /** Gets the color to use when regionSelectionMethod is set to
     *  custom color in the enclose and fill
     */
    KoColor regionSelectionColor() const;

    /** Sets if the selection mask should be inverted */
    void setRegionSelectionInvert(bool invert);

    /** Gets if the selection mask should be inverted */
    bool regionSelectionInvert() const;

    /** Sets if the regions that touch the enclosing area contour should be kept or excluded */
    void setRegionSelectionIncludeContourRegions(bool include);

    /** Gets if the regions that touch the enclosing area contour should be kept or excluded */
    bool regionSelectionIncludeContourRegions() const;

    /**
     *  Sets if the surrounding regions of the specific selected color should
     *  be kept or excluded when using @ref SelectRegionsSurroundedBySpecificColor,
     *  @ref SelectRegionsSurroundedByTransparent or
     *  @ref SelectRegionsSurroundedBySpecificColorOrTransparent as the region
     *  selection method
     */
    void setRegionSelectionIncludeSurroundingRegions(bool include);

    /**
     *  Gets if the surrounding regions of the specific selected color should
     *  be kept or excluded when using @ref SelectRegionsSurroundedBySpecificColor,
     *  @ref SelectRegionsSurroundedByTransparent or
     *  @ref SelectRegionsSurroundedBySpecificColorOrTransparent as the region
     *  selection method
     */
    bool regionSelectionIncludeSurroundingRegions() const;

protected:
    void genericEncloseAndFillStart(KisPixelSelectionSP enclosingMask, KisPaintDeviceSP referenceDevice);
    void genericEncloseAndFillEnd(KisPaintDeviceSP filled);

private:
    class Private;
    QScopedPointer<Private> m_d;
};

#endif
