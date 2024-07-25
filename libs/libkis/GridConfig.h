/*
 *  SPDX-FileCopyrightText: 2024 Grum999 <grum999@grum.fr>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef LIBKIS_GRIDCONFIG_H
#define LIBKIS_GRIDCONFIG_H

#include <QObject>
#include "kis_grid_config.h"

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * The GridConfig class encapsulates a Krita Guides configuration.
 *
 */
class KRITALIBKIS_EXPORT GridConfig : public QObject
{
    Q_OBJECT

public:
    GridConfig(KisGridConfig *guidesConfig);

    /**
     * Create a new, empty GridConfig.
     */
    explicit GridConfig(QObject *parent = 0);
    ~GridConfig() override;

    bool operator==(const GridConfig &other) const;
    bool operator!=(const GridConfig &other) const;

public Q_SLOTS:

    /**
     * @brief Returns grid visibility for document.
     * @return If grid is visible, return True.
     */
    bool visible() const;

    /**
     * @brief Set grid visibility for document.
     * @param snap Set to True to get grid visible.
     */
    void setVisible(bool visible);

    /**
     * @brief Returns snap to grid status for document.
     * @return If snap to grid is active on document, return True.
     */
    bool snap() const;

    /**
     * @brief Activate or deactivate snap to grid for document
     * @param snap Set to True to activate snap to grid.
     */
    void setSnap(bool snap);

    /**
     * @brief Returns grid offset (in pixels, from origin) for document.
     * @return A QPoint that define X and Y offset.
     */
    QPoint offset() const;

    /**
     * @brief Define grid offset (in pixels, from origin) for document.
     * @param offset A QPoint that define X and Y offset (X and Y in range [0 - 500])
     */
    void setOffset(QPoint offset);

    /**
     * @brief Returns grid spacing (in pixels) for document.
     * Spacing value is used for grid type "rectangular".
     * @return A QPoint that define X and Y spacing.
     */
    QPoint spacing() const;

    /**
     * @brief Set grid spacing (in pixels) for document.
     * Spacing value is used for grid type "rectangular".
     * @param spacing A QPoint that define X and Y spacing  (minimum value for X and Y is 1)
     */
    void setSpacing(QPoint spacing);

    /**
     * @brief Returns if horizontal grid spacing is active.
     * Spacing value is used for grid type "rectangular".
     *
     * @returns a boolean which indicate if horizontal grid is active or not
     */
    bool spacingActiveHorizontal() const;

    /**
     * @brief Set horizontal grid spacing active.
     * Spacing value is used for grid type "rectangular".
     *
     * @param active True to activate horizontal spacing, False to deactivate it.
     */
    void setSpacingActiveHorizontal(bool active);

    /**
     * @brief Returns if vertical grid spacing is active.
     * Spacing value is used for grid type "rectangular".
     *
     * @returns a boolean which indicate if vertical grid is active or not
     */
    bool spacingActiveVertical() const;

    /**
     * @brief Set vertical grid spacing active.
     * Spacing value is used for grid type "rectangular".
     *
     * @param active True to activate vertical spacing, False to deactivate it.
     */
    void setSpacingActiveVertical(bool active);

    /**
     * @brief Returns number of grid subdivision for document.
     * Subdivision value is used for grid type "rectangular".
     * @return A positive integer value, starting from 1
     */
    int subdivision() const;

    /**
     * @brief Set number of grid subdivision for document.
     * Subdivision value is used for grid type "rectangular".
     * @param subdivision A positive integer value, in range [1 - 10]
     */
    void setSubdivision(int subdivision);

    /**
     * @brief Returns left angle (in degrees) of isometric grid for document.
     * AngleLeft value is used for grid type "isometric".
     * @return A positive decimal value, in range [0.00 - 89.00]
     */
    qreal angleLeft() const;

    /**
     * @brief Set left angle (in degrees) of isometric grid for document.
     * AngleLeft value is used for grid type "isometric".
     * @param angleLeft A positive decimal value, in range [0.00 - 89.00]
     */
    void setAngleLeft(qreal angleLeft);

    /**
     * @brief Returns right angle (in degrees) of isometric grid for document.
     * AngleRight value is used for grid type "isometric".
     * @return A positive decimal value, in range [0.00 - 89.00]
     */
    qreal angleRight() const;

    /**
     * @brief Set right angle (in degrees) of isometric grid for document.
     * AngleRight value is used for grid type "isometric".
     * @param angleRight A positive decimal value, in range [0.00 - 89.00]
     */
    void setAngleRight(qreal angleRight);

    /**
     * @brief Returns if left angle grid is active.
     * Spacing value is used for grid type "isometric".
     *
     * @returns a boolean which indicate if left angle grid is active or not
     */
    bool angleLeftActive() const;

    /**
     * @brief Set left angle grid active.
     * Spacing value is used for grid type "isometric".
     *
     * @param active True to activate left angle grid, False to deactivate it.
     */
    void setAngleLeftActive(bool active);

    /**
     * @brief Returns if right angle grid is active.
     * Spacing value is used for grid type "isometric".
     *
     * @returns a boolean which indicate if right angle grid is active or not
     */
    bool angleRightActive() const;

    /**
     * @brief Set right angle grid active.
     * Spacing value is used for grid type "isometric".
     *
     * @param active True to activate right angle grid, False to deactivate it.
     */
    void setAngleRightActive(bool active);

    /**
     * @brief Returns grid cell spacing (in pixels) for document.
     * Cell spacing value is used for grid type "isometric_legacy".
     * @return A positive integer value, minimum value is 10
     */
    int cellSpacing() const;

    /**
     * @brief Set grid cell spacing for document.
     * Cell spacing value is used for grid type "isometric_legacy".
     * @param cellSpacing A integer that define spacing, in range [10 - 1000]
     */
    void setCellSpacing(int cellSpacing);

    /**
     * @brief Returns grid cell border size (in pixels) for document.
     * Cell spacing value is used for grid type "isometric".
     * @return A positive integer value, in range [10 - 1000]
     */
    int cellSize() const;

    /**
     * @brief Set grid cell size (in pixels) for document.
     * Cell spacing value is used for grid type "isometric".
     * @param cellSize An integer that define cell border size.
     */
    void setCellSize(int cellSize);

    /**
     * @brief Returns current grid type applied for document.
     * @return The grid type can be:
     * - "rectangular"
     * - "isometric"
     * - "isometric_legacy"
     */
    QString type() const;

    /**
     * @brief Set current grid type applied for document.
     * @param gridType The grid type can be:
     * - "rectangular"
     * - "isometric"
     * - "isometric_legacy"
     */
    void setType(const QString &gridType);

    /**
     * @brief Returns status of "Aspect locked" property for offset values
     * (X and Y values are linked to keep ratio)
     * @return If locked, return True.
     */
    bool offsetAspectLocked() const;

    /**
     * @brief Set status of "Aspect locked" property for offset values
     * (X and Y values are linked to keep ratio)
     * @param offsetAspectLocked Set to True lock aspect.
     */
    void setOffsetAspectLocked(bool offsetAspectLocked);

    /**
     * @brief Returns status of "Aspect locked" property for spacing values
     * (mean, X and Y values are linked to keep ratio)
     * SpacingAspectLocked value is used for grid type "rectangular".
     * @return If locked, return True.
     */
    bool spacingAspectLocked() const;

    /**
     * @brief Set status of "Aspect locked" property for spacing values
     * (X and Y values are linked to keep ratio)
     * SpacingAspectLocked value is used for grid type "rectangular".
     * @param spacingAspectLocked Set to True lock aspect.
     */
    void setSpacingAspectLocked(bool spacingAspectLocked);

    /**
     * @brief Returns status of "Aspect locked" property for angles values
     * (mean, left and right angles values are linked to keep ratio)
     * AngleAspectLocked value is used for grid type "isometric" and "isometric_legacy".
     * @return If locked, return True.
     */
    bool angleAspectLocked() const;

    /**
     * @brief Set status of "Aspect locked" property for angles values
     * (left and right angles values are linked to keep ratio)
     * AngleAspectLocked value is used for grid type "isometric" and "isometric_legacy".
     * @param angleAspectLocked Set to True lock aspect.
     */
    void setAngleAspectLocked(bool angleAspectLocked);

    /**
     * @brief Returns grid main line type
     * @return The main line type for grid in current document
     * Can be:
     * - "solid"
     * - "dashed"
     * - "dotted"
     */
    QString lineTypeMain() const;

    /**
     * @brief Set grid main line type
     * @param lineType The main line type to apply for grid
     * Can be:
     * - "solid"
     * - "dashed"
     * - "dotted"
     */
    void setLineTypeMain(const QString &lineType);

    /**
     * @brief Returns grid subdivision line type
     * @return The subdivision line type for grid in current document
     * Can be:
     * - "solid"
     * - "dashed"
     * - "dotted"
     *
     * LineTypeSubdivision value is used for grid type "rectangular".
     */
    QString lineTypeSubdivision() const;

    /**
     * @brief Set grid subdivision line type
     * @param lineType The subdivision line type to apply for grid
     * Can be:
     * - "solid"
     * - "dashed"
     * - "dotted"
     *
     * LineTypeSubdivision value is used for grid type "rectangular".
     */
    void setLineTypeSubdivision(const QString &lineType);

    /**
     * @brief Returns grid vertical line type
     * @return The vertical line type for grid in current document
     * Can be:
     * - "solid"
     * - "dashed"
     * - "dotted"
     * - "none"
     *
     * LineTypeVertical value is used for grid type "isometric".
     */
    QString lineTypeVertical() const;

    /**
     * @brief Set grid vertical line type
     * @param lineType The vertical line type to apply for grid
     * Can be:
     * - "solid"
     * - "dashed"
     * - "dotted"
     * - "none"
     *
     * LineTypeVertical value is used for grid type "isometric".
     */
    void setLineTypeVertical(const QString &lineType);

    /**
     * @brief Returns grid main line color
     * @return The color for grid main line
     */
    QColor colorMain() const;

    /**
     * @brief Set grid main line color
     * @param color The color to apply for grid main line
     */
    void setColorMain(QColor colorMain);

    /**
     * @brief Returns grid subdivision line color
     * ColorSubdivision value is used for grid type "rectangular".
     * @return The color for grid subdivision line
     */
    QColor colorSubdivision() const;

    /**
     * @brief Set grid subdivision line color
     * ColorSubdivision value is used for grid type "rectangular".
     * @param color The color to apply for grid subdivision line
     */
    void setColorSubdivision(QColor colorSubdivision);

    /**
     * @brief Returns grid vertical line color
     * ColorSubdivision value is used for grid type "isometric".
     * @return The color for grid vertical line
     */
    QColor colorVertical() const;

    /**
     * @brief Set grid vertical line color
     * ColorSubdivision value is used for grid type "isometric".
     * @param color The color to apply for grid vertical line
     */
    void setColorVertical(QColor colorVertical);

    /**
     * @brief Load grid definition from an XML document
     * @param xmlContent xml content provided as a string
     * @return True if xml content is valid and grid has been loaded, otherwise False
     */
    bool fromXml(const QString &xmlContent) const;

    /**
     * @brief Save grid definition as an XML document
     * @return A string with xml content
     */
    QString toXml() const;


private:
    friend class Document;

    KisGridConfig gridConfig() const;

private:
    struct Private;
    Private *d;

};

#endif // LIBKIS_GRIDCONFIG_H
