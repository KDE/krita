/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_GRID_CONFIG_H
#define __KIS_GRID_CONFIG_H

#include <QPoint>
#include <QColor>
#include <QPen>

#include <boost/operators.hpp>
#include "kritaui_export.h"

class QDomElement;
class QDomDocument;


class KRITAUI_EXPORT KisGridConfig : boost::equality_comparable<KisGridConfig>
{
public:
    enum LineTypeInternal {
        LINE_SOLID = 0,
        LINE_DASHED,
        LINE_DOTTED,
        LINE_NONE
    };

    enum GridType {
        GRID_RECTANGULAR = 0,
        GRID_ISOMETRIC_LEGACY,
        GRID_ISOMETRIC
    };

    struct TrigoCache {
        qreal correctedAngleRightCellSize;
        qreal correctedAngleRightOffsetX;
        qreal tanAngleRight;

        qreal correctedAngleLeftCellSize;
        qreal correctedAngleLeftOffsetX;
        qreal tanAngleLeft;

        qreal verticalSpace;
    };

public:
    KisGridConfig()
        : m_showGrid(false),
          m_snapToGrid(false),
          m_spacing(16,16),
          m_xSpacingActive(true),
          m_ySpacingActive(true),
          m_offsetActive(false),
          m_offsetAspectLocked(true),
          m_spacingAspectLocked(true),
          m_angleLeft(45),
          m_angleRight(45),
          m_angleAspectLocked(true),
          m_angleLeftActive(true),
          m_angleRightActive(true),
          m_cellSpacing(30),
          m_cellSize(30),
          m_gridType(GRID_RECTANGULAR),
          m_subdivision(2),
          m_lineTypeMain(LINE_SOLID),
          m_lineTypeSubdivision(LINE_DOTTED),
          m_lineTypeIsoVertical(LINE_DASHED),
          m_colorMain(200, 200, 200, 200),
          m_colorSubdivision(200, 200, 200, 150),
          m_colorIsoVertical(200, 200, 200, 100),
          m_penMain(QPen()),
          m_penSubdivision(QPen()),
          m_penVertical(QPen())
    {
        loadStaticData();
        m_penMain.setWidth(0);
        m_penSubdivision.setWidth(0);
        m_penVertical.setWidth(0);
    }

    bool operator==(const KisGridConfig &rhs) const {
        return
            m_showGrid == rhs.m_showGrid &&
            m_snapToGrid == rhs.m_snapToGrid &&
            m_spacing == rhs.m_spacing &&
            m_xSpacingActive == rhs.m_xSpacingActive &&
            m_ySpacingActive == rhs.m_ySpacingActive &&
            m_offsetActive == rhs.m_offsetActive &&
            m_offset == rhs.m_offset &&
            m_offsetAspectLocked == rhs.m_offsetAspectLocked &&
            m_spacingAspectLocked == rhs.m_spacingAspectLocked &&
            m_angleRight == rhs.m_angleRight &&
            m_angleLeft == rhs.m_angleLeft &&
            m_angleAspectLocked == rhs.m_angleAspectLocked &&
            m_angleLeftActive == rhs.m_angleLeftActive &&
            m_angleRightActive == rhs.m_angleRightActive &&
            m_gridType == rhs.m_gridType &&
            m_cellSpacing == rhs.m_cellSpacing &&
            m_cellSize == rhs.m_cellSize &&
            m_subdivision == rhs.m_subdivision &&
            m_lineTypeMain == rhs.m_lineTypeMain &&
            m_lineTypeSubdivision == rhs.m_lineTypeSubdivision &&
            m_lineTypeIsoVertical == rhs.m_lineTypeIsoVertical &&
            m_colorMain == rhs.m_colorMain &&
            m_colorSubdivision == rhs.m_colorSubdivision &&
            m_colorIsoVertical == rhs.m_colorIsoVertical;
    }

    bool showGrid() const {
        return m_showGrid;
    }
    void setShowGrid(bool value) {
        m_showGrid = value;
    }

    bool snapToGrid() const {
        return m_snapToGrid;
    }
    void setSnapToGrid(bool value) {
        m_snapToGrid = value;
    }

    bool offsetActive() const {
        return m_offsetActive;
    }
    void setOffsetActive(bool value) {
        m_offsetActive = value;
    }

    QPoint offset() const {
        return m_offset;
    }
    void setOffset(const QPoint &value) {
        m_offset = value;
        updateTrigoCache();
    }

    QPoint spacing() const {
        return m_spacing;
    }
    void setSpacing(const QPoint &value) {
        m_spacing = value;
    }

    bool xSpacingActive() const {
        return m_xSpacingActive;
    }
    void setXSpacingActive(bool value) {
        m_xSpacingActive = value;
    }

    bool ySpacingActive() const {
        return m_ySpacingActive;
    }
    void setYSpacingActive(bool value) {
        m_ySpacingActive = value;
    }

    int subdivision() const {
        return m_subdivision;
    }
    void setSubdivision(int value) {
        m_subdivision = value;
    }

    qreal angleLeft() const {
        return m_angleLeft;
    }
    void setAngleLeft(qreal angle) {
        m_angleLeft = angle;
        updateTrigoCache();
    }

    qreal angleRight() const {
        return m_angleRight;
    }

    void setAngleRight(qreal angle) {
        m_angleRight = angle;
        updateTrigoCache();
    }

    bool angleLeftActive() const {
        return m_angleLeftActive;
    }

    void setAngleLeftActive(bool value) {
        m_angleLeftActive = value;
    }

    bool angleRightActive() const {
        return m_angleRightActive;
    }

    void setAngleRightActive(bool value) {
        m_angleRightActive = value;
    }

    int cellSpacing() const {
        return m_cellSpacing;
    }
    void setCellSpacing(int spacing) {
        m_cellSpacing = spacing;
    }

    int cellSize() const {
        return m_cellSize;
    }
    void setCellSize(int spacing) {
        m_cellSize = spacing;
        updateTrigoCache();
    }

    GridType gridType() const {
        return m_gridType;
    }
    void setGridType(GridType type) {
        m_gridType = type;
    }

    bool offsetAspectLocked() const {
        return m_offsetAspectLocked;
    }
    void setOffsetAspectLocked(bool value) {
        m_offsetAspectLocked = value;
    }

    bool spacingAspectLocked() const {
        return m_spacingAspectLocked;
    }
    void setSpacingAspectLocked(bool value) {
        m_spacingAspectLocked = value;
    }

    bool angleAspectLocked() const {
        return m_angleAspectLocked;
    }
    void setAngleAspectLocked(bool value) {
        m_angleAspectLocked = value;
    }

    LineTypeInternal lineTypeMain() const {
        return m_lineTypeMain;
    }
    void setLineTypeMain(LineTypeInternal value) {
        m_lineTypeMain = value;
        updatePenStyle(&m_penMain, m_colorMain, m_lineTypeMain);
    }

    LineTypeInternal lineTypeSubdivision() const {
        return m_lineTypeSubdivision;
    }

    void setLineTypeSubdivision(LineTypeInternal value) {
        m_lineTypeSubdivision = value;
        updatePenStyle(&m_penSubdivision, m_colorSubdivision, m_lineTypeSubdivision);
    }

    LineTypeInternal lineTypeVertical() const {
        return m_lineTypeIsoVertical;
    }

    void setLineTypeVertical(LineTypeInternal value) {
        m_lineTypeIsoVertical= value;
        updatePenStyle(&m_penVertical, m_colorIsoVertical, m_lineTypeIsoVertical);
        updateTrigoCache();
    }

    QColor colorMain() const {
        return m_colorMain;
    }
    void setColorMain(const QColor &value) {
        m_colorMain = value;
        updatePenStyle(&m_penMain, m_colorMain, m_lineTypeMain);
    }

    QColor colorSubdivision() const {
        return m_colorSubdivision;
    }
    void setColorSubdivision(const QColor &value) {
        m_colorSubdivision = value;
        updatePenStyle(&m_penSubdivision, m_colorSubdivision, m_lineTypeSubdivision);
    }

    QColor colorVertical() const {
        return m_colorIsoVertical;
    }
    void setColorVertical(const QColor &value) {
        m_colorIsoVertical = value;
        updatePenStyle(&m_penVertical, m_colorIsoVertical, m_lineTypeIsoVertical);
    }

    QPen penMain() const {
        return m_penMain;
    }

    QPen penSubdivision() const {
        return m_penSubdivision;
    }

    QPen penVertical() const {
        return m_penVertical;
    }

    TrigoCache trigoCache() const {
        return m_trigoCache;
    }

    void loadStaticData();
    void saveStaticData() const;

    QDomElement saveDynamicDataToXml(QDomDocument& doc, const QString &tag) const;
    bool loadDynamicDataFromXml(const QDomElement &parent);

    static const KisGridConfig& defaultGrid();

    bool isDefault() const {
        return *this == defaultGrid();
    }

    /// Transform the grids using the given \p transform. Please note that \p transform
    /// should be in 'image' coordinate system.
    /// Used with image-wide transformations.
    void transform(const QTransform &transform);

private:
    void updatePenStyle(QPen *pen, QColor color, LineTypeInternal type);
    void updateTrigoCache();

private:
    // Dynamic data. Stored in KisDocument.

    bool m_showGrid;
    bool m_snapToGrid;
    QPoint m_spacing;
    bool m_xSpacingActive;
    bool m_ySpacingActive;
    bool m_offsetActive;
    bool m_offsetAspectLocked;
    bool m_spacingAspectLocked;
    qreal m_angleLeft;
    qreal m_angleRight;
    bool m_angleAspectLocked;
    bool m_angleLeftActive;
    bool m_angleRightActive;
    int m_cellSpacing;
    int m_cellSize;

    GridType m_gridType;
    int m_subdivision;

    QPoint m_offset;

    // Static data. Stored in the Krita config.

    LineTypeInternal m_lineTypeMain;
    LineTypeInternal m_lineTypeSubdivision;
    LineTypeInternal m_lineTypeIsoVertical;

    QColor m_colorMain;
    QColor m_colorSubdivision;
    QColor m_colorIsoVertical;

    QPen m_penMain;
    QPen m_penSubdivision;
    QPen m_penVertical;

    TrigoCache m_trigoCache;
};

Q_DECLARE_METATYPE(KisGridConfig)

#endif /* __KIS_GRID_CONFIG_H */
