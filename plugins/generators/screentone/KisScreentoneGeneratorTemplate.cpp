/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <algorithm>
#include <limits>
#include <cmath>

#include "KisScreentoneScreentoneFunctions.h"
#include "KisScreentoneGeneratorTemplate.h"

KisScreentoneGeneratorTemplate::KisScreentoneGeneratorTemplate(const KisScreentoneGeneratorConfigurationSP config)
{
    const int pattern = config->pattern();
    const int shape = config->shape();
    const int interpolation = config->interpolation();

    if (pattern == KisScreentonePatternType_Dots) {
        if (shape == KisScreentoneShapeType_RoundDots) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::DotsRoundLinear screentoneFunction;
                makeTemplate(config, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::DotsRoundSinusoidal screentoneFunction;
                makeTemplate(config, screentoneFunction);
            }
        } else if (shape == KisScreentoneShapeType_EllipseDotsLegacy) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::DotsEllipseLinear_Legacy screentoneFunction;
                makeTemplate(config, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::DotsEllipseSinusoidal_Legacy screentoneFunction;
                makeTemplate(config, screentoneFunction);
            }
        } else if (shape == KisScreentoneShapeType_EllipseDots) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::DotsEllipseLinear screentoneFunction;
                makeTemplate(config, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::DotsEllipseSinusoidal screentoneFunction;
                makeTemplate(config, screentoneFunction);
            }
        } else if (shape == KisScreentoneShapeType_DiamondDots) {
            KisScreentoneScreentoneFunctions::DotsDiamond screentoneFunction;
            makeTemplate(config, screentoneFunction);
        } else if (shape == KisScreentoneShapeType_SquareDots) {
            KisScreentoneScreentoneFunctions::DotsSquare screentoneFunction;
            makeTemplate(config, screentoneFunction);
        }
    } else if (pattern == KisScreentonePatternType_Lines) {
        if (shape == KisScreentoneShapeType_StraightLines) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::LinesStraightLinear screentoneFunction;
                makeTemplate(config, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::LinesStraightSinusoidal screentoneFunction;
                makeTemplate(config, screentoneFunction);
            }
        } else if (shape == KisScreentoneShapeType_SineWaveLines) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::LinesSineWaveLinear screentoneFunction;
                makeTemplate(config, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::LinesSineWaveSinusoidal screentoneFunction;
                makeTemplate(config, screentoneFunction);
            }
        } else if (shape == KisScreentoneShapeType_TriangularWaveLines) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::LinesTriangularWaveLinear screentoneFunction;
                makeTemplate(config, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::LinesTriangularWaveSinusoidal screentoneFunction;
                makeTemplate(config, screentoneFunction);
            }
        } else if (shape == KisScreentoneShapeType_SawtoothWaveLines) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::LinesSawToothWaveLinear screentoneFunction;
                makeTemplate(config, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::LinesSawToothWaveSinusoidal screentoneFunction;
                makeTemplate(config, screentoneFunction);
            }
        } else if (shape == KisScreentoneShapeType_CurtainsLines) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::LinesCurtainsLinear screentoneFunction;
                makeTemplate(config, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::LinesCurtainsSinusoidal screentoneFunction;
                makeTemplate(config, screentoneFunction);
            }
        }
    }
}

// The following utility classes are used to get the preferred order in which
// the pixels of a cell should be "turn on" based on the distance to the center
// of the shape (center of the dot, center of the line, etc.). For simplicity
// they just use the screentone function "as is" except for the round/ellipse
// dot sinusoidal, which use the linear counterpart (to get an ordering that
// seems to grow radially)
template <typename ScreentoneFunction>
struct LightUpOrderingFunction : public ScreentoneFunction
{};
template <>
struct LightUpOrderingFunction<KisScreentoneScreentoneFunctions::DotsRoundSinusoidal>
    : public KisScreentoneScreentoneFunctions::DotsRoundLinear
{};
template <>
struct LightUpOrderingFunction<KisScreentoneScreentoneFunctions::DotsEllipseSinusoidal>
    : public KisScreentoneScreentoneFunctions::DotsEllipseLinear
{};

template <typename ScreentoneFunction>
void KisScreentoneGeneratorTemplate::makeTemplate(const KisScreentoneGeneratorConfigurationSP config,
                                                  ScreentoneFunction screentoneFunction)
{
    // NOTE: In the following, the word "screen" is used to mean the dot screen
    // of the screentone (the grid of dots/cells)

    // Get transformation parameters
    qreal sizeX, sizeY;
    const int alignX = config->alignToPixelGrid() ? config->alignToPixelGridX() : 1;
    const int alignY = config->alignToPixelGrid() ? config->alignToPixelGridY() : 1;
    if (config->sizeMode() == KisScreentoneSizeMode_PixelBased) {
        const bool constrainSize = config->constrainSize();
        sizeX = config->sizeX();
        // Ensure that the size y component is equal to the x component
        // if keepSizeSquare is true
        sizeY = constrainSize ? sizeX : config->sizeY();
    } else {
        const qreal resolution = config->resolution();
        const bool constrainFrequency = config->constrainFrequency();
        const qreal frequencyX = config->frequencyX();
        // Ensure that the frequency y component is equal to the x component
        // if constrainFrequency is true
        const qreal frequencyY = constrainFrequency ? frequencyX : config->frequencyY();
        sizeX = qMax(1.0, resolution / frequencyX);
        sizeY = qMax(1.0, resolution / frequencyY);
    }
    const qreal positionX = config->alignToPixelGrid() ? qRound(config->positionX()) : config->positionX();
    const qreal positionY = config->alignToPixelGrid() ? qRound(config->positionY()) : config->positionY();
    m_screenPosition = QPointF(positionX, positionY);
    const qreal shearX = config->shearX();
    const qreal shearY = config->shearY();
    const qreal rotation = config->rotation();
    // Construct image<->screen transforms
    m_imageToScreenTransform.shear(shearX, shearY);
    m_imageToScreenTransform.scale(qFuzzyIsNull(sizeX) ? 0.0 : 1.0 / sizeX, qFuzzyIsNull(sizeY) ? 0.0 : 1.0 / sizeY);
    m_imageToScreenTransform.rotate(rotation);
    m_imageToScreenTransform.translate(positionX, positionY);
    QTransform screenToImage;
    screenToImage.rotate(-rotation);
    screenToImage.scale(sizeX, sizeY);
    screenToImage.shear(-shearX, -shearY);
    // u1 is the unaligned vector that goes from the origin to the top-right
    // corner of the macrocell. v1 is the aligned version
    // u2 is the unaligned vector that goes from the origin to the bottom-left
    // corner of the macrocell. v2 is the aligned version.
    // At this point we asume the macrocell will have a minimum size given by
    // the alignment
    const QPointF u1 = screenToImage.map(QPointF(static_cast<qreal>(alignX), 0.0));
    const QPointF u2 = screenToImage.map(QPointF(0.0, static_cast<qreal>(alignY)));
    QPointF v1(qRound(u1.x()), qRound(u1.y()));
    QPointF v2(qRound(u2.x()), qRound(u2.y()));
    // If the following condition is met, that means that the screen is
    // transformed in such a way that the cell corners are colinear so we move
    // v1 or v2 to a neighbor position and give the cell some area
    if (qFuzzyCompare(v1.y() * v2.x(), v2.y() * v1.x()) &&
        !qFuzzyIsNull(v1.x() * v2.x() + v1.y() * v2.y())) {
        // Choose point to move based on distance from non aligned point to
        // aligned point
        const qreal dist1 = kisSquareDistance(u1, v1);
        const qreal dist2 = kisSquareDistance(u2, v2);
        const QPointF *p_u = dist1 > dist2 ? &u1 : &u2;
        QPointF *p_v = dist1 > dist2 ? &v1 : &v2;
        // Then we get the closest pixel aligned point to the current, colinear,
        // point
        QPair<int, qreal> dists[4]{
            {1, kisSquareDistance(*p_u, *p_v + QPointF(0.0, -1.0))},
            {2, kisSquareDistance(*p_u, *p_v + QPointF(1.0, 0.0))},
            {3, kisSquareDistance(*p_u, *p_v + QPointF(0.0, 1.0))},
            {4, kisSquareDistance(*p_u, *p_v + QPointF(-1.0, 0.0))}
        };
        std::sort(
            std::begin(dists), std::end(dists),
            [](const QPair<int, qreal> &a, const QPair<int, qreal> &b)
            {
                return a.second < b.second;
            }
        );
        // Move the point
        if (dists[0].first == 1) {
            p_v->setY(p_v->y() - 1.0);
        } else if (dists[0].first == 2) {
            p_v->setX(p_v->x() + 1.0);
        } else if (dists[0].first == 3) {
            p_v->setY(p_v->y() + 1.0);
        } else {
            p_v->setX(p_v->x() - 1.0);
        }
    }
    qreal v1Length = std::sqrt(v1.x() * v1.x() + v1.y() * v1.y());
    qreal v2Length = std::sqrt(v2.x() * v2.x() + v2.y() * v2.y());
    // The macrocell will have a minimum size derived from the alignment but if
    // its size doesn't contain enough pixels to cover all the range of
    // intensities we expand it.
    QSize macrocellTiles(1, 1);
    while (macrocellTiles.width() * v1Length * macrocellTiles.height() * v2Length < 256) {
        if (macrocellTiles.width() * v1Length > macrocellTiles.height() * v2Length) {
            macrocellTiles.setHeight(macrocellTiles.height() + 1);
        } else {
            macrocellTiles.setWidth(macrocellTiles.width() + 1);
        }
    }
    // Get size in cells of the macrocell
    m_macrocellSize = QSize(alignX * macrocellTiles.width(), alignY * macrocellTiles.height());
    const QSizeF macrocellSizeF = m_macrocellSize;
    // Scale the top and left vectors and lengths to the final macrocell size
    v1 *= macrocellTiles.width();
    v2 *= macrocellTiles.height();
    v1Length *= macrocellTiles.width();
    v2Length *= macrocellTiles.height();
    // Compute other useful quantities
    const QPointF v3 = v1 + v2;
    const QPointF l1 = v1;
    const QPointF l2 = v2;
    const QPointF l3 = -v1;
    const QPointF l4 = -v2;
    const qreal v1MicrocellLength = v1Length / macrocellSizeF.width();
    const qreal v2MicrocellLength = v2Length / macrocellSizeF.height();
    m_v1 = v1;
    m_v2 = v2;
    // Construct template<->screen transforms
    const QPolygonF quad(
        {
            QPointF(0.0, 0.0),
            v1 / macrocellSizeF.width(),
            v1 / macrocellSizeF.width() + v2 / macrocellSizeF.height(),
            v2 / macrocellSizeF.height()
        }
    );
    QTransform::quadToSquare(quad, m_templateToScreenTransform);
    m_screenToTemplateTransform = m_templateToScreenTransform.inverted();

    // Construct the template

    // Compute template dimensions
    const QPoint topLeft(
        static_cast<int>(qMin(0.0, qMin(v1.x(), qMin(v2.x(), v1.x() + v2.x())))),
        static_cast<int>(qMin(0.0, qMin(v1.y(), qMin(v2.y(), v1.y() + v2.y()))))
    );
    const QPoint bottomRight(
        static_cast<int>(qMax(0.0, qMax(v1.x(), qMax(v2.x(), v1.x() + v2.x())))),
        static_cast<int>(qMax(0.0, qMax(v1.y(), qMax(v2.y(), v1.y() + v2.y()))))
    );
    // Add an 1 pixel border around the template, useful for bilinear interpolation
    m_templateSize = QSize(bottomRight.x() - topLeft.x() + 2, bottomRight.y() - topLeft.y() + 2);
    m_originOffset = -topLeft + QPoint(1, 1);

    m_templateData = QVector<qreal>(m_templateSize.width() * m_templateSize.height(), -1.0);

    // Convenience struct to store some info for a single pixel during the
    // construction of the template
    struct AuxiliaryPoint
    {
        // Pixel index with respect to the template image
        int templatePixelIndex;
        // An pseudo index that orders the pixel with respect to a cell from
        // top-left to bottom right
        qreal microcellPixelIndex;
        // Spot function value at the top-left corner of the pixel
        qreal valueAtCorner;
        // Spot function value at the center of the pixel
        qreal valueAtCenter;
        // Distance from the "center" of the shape to the top-left corner of the
        // pixel
        qreal distanceToCorner;
        // Distance from the "center" of the shape to the center of the pixel
        qreal distanceToCenter;
    };
    // Convenience struct to store some info for a single microcell during the
    // construction of the template
    struct AuxiliaryMicrocell
    {
        // The order in which this cell should light up pixels with respect to
        // the other cells
        int index;
        // List of points in this cell
        QVector<AuxiliaryPoint> auxiliaryPoints;
    };

    QVector<AuxiliaryMicrocell> auxiliaryMicrocells(m_macrocellSize.width() * m_macrocellSize.height());

    // Set the ordered index for each auxiliary microcell
    // Use makeCellOrderList to shuffle the microcell activation order so that
    // they follow bayer matrix like patters. This allows to grow the
    // microcells in a way that they don't visually cluster
    QVector<int> microcellIndices = makeCellOrderList(m_macrocellSize.width(), m_macrocellSize.height());
    for (int i = 0; i < auxiliaryMicrocells.size(); ++i) {
        auxiliaryMicrocells[i].index = microcellIndices[i];
    }

    // "Rasterize" the macrocell: get all the points of the template that lie
    // inside the macrocell and store them as auxiliary points that contain
    // some info useful for sorting later.
    // The template will have a representation of the complete transformed
    // macrocell, but also some other areas outside (parts of adjacent
    // macrocells) if, for example, the screen is rotated. To get later a value
    // from the template we only use the pixels inside the macrocell, but the
    // pixels outside are usefull to perform bilinear interpolation

    // Get the horizontal and vertical points at cell corners
    QVector<QPointF> horizontalCellPositions;
    for (int i = -1; i <= m_macrocellSize.width() + 1; ++i) {
        horizontalCellPositions.append(v1 / static_cast<qreal>(m_macrocellSize.width()) * static_cast<qreal>(i));
    }
    QVector<QPointF> verticalCellPositions;
    for (int i = -1; i <= m_macrocellSize.height() + 1; ++i) {
        verticalCellPositions.append(v2 / static_cast<qreal>(m_macrocellSize.height()) * static_cast<qreal>(i));
    }

    int totalNumberOfAuxiliaryPoints = 0;

    // Convenience function that tells if a pixel should belong to the left or
    // to the right side of an edge. Used only when the point lies exactly in
    // the edge
    auto pointBelongsToLeftSideOfEdge = [](const QPointF &point, const QPointF &edgeStart, const QPointF &edgeDirection) -> bool
    {
        const qreal r = (point.x() - edgeStart.x()) * edgeDirection.y() - (point.y() - edgeStart.y()) * edgeDirection.x();
        return (r == 0.0) && ((edgeDirection.y() == 0.0 && edgeDirection.x() > 0.0) || (edgeDirection.y() > 0.0));
    };

    // This is used to get an estimate of how far a given point is from the
    // "center" of the shape (center of dot, center of line, etc.)
    LightUpOrderingFunction<ScreentoneFunction> lightUpOrderingFunction;

    // Visit all the pixels on the template and add an auxiliary point for each
    // one that falls inside the macrocell
    for (int i = 0; i < m_templateData.size(); ++i) {
        // Transform the pixel position from template to screen coordinates.
        // We transform both the point at the pixel corner (used for sampling
        // the screentone function) and the point at pixel center (used to
        // compute to which cell the pixel belongs)
        const int templateY = i / m_templateSize.width();
        const int templateX = i - m_templateSize.width() * templateY;
        const QPointF pixelCornerPoint(
            static_cast<qreal>(templateX - m_originOffset.x()),
            static_cast<qreal>(templateY - m_originOffset.y())
        );
        const QPointF pixelCenterPoint(pixelCornerPoint + QPointF(0.5, 0.5));
        const QPointF pixelCenterScreenPoint = m_templateToScreenTransform.map(pixelCenterPoint);
        const QPointF macrocellPos(pixelCenterScreenPoint.x() * v1MicrocellLength, pixelCenterScreenPoint.y() * v2MicrocellLength);
        // Initial estimate for the microcell position
        int microcellX = static_cast<int>(std::floor(macrocellPos.x() * static_cast<qreal>(m_macrocellSize.width()) / v1Length));
        int microcellY = static_cast<int>(std::floor(macrocellPos.y() * static_cast<qreal>(m_macrocellSize.height()) / v2Length));
        // Discard this pixel if it is clearly outside the macrocell
        if (microcellX < -1 || microcellX > m_macrocellSize.width() ||
            microcellY < -1 || microcellY > m_macrocellSize.height()) {
            continue;
        }
        // If the pixel center lies exactly in a border between cells we must
        // decide if it should belong to a neighbor cell. This is an usual
        // problem in rasterization. Here we treat the pixel as if it belonged
        const QPointF topLeftCorner(horizontalCellPositions[microcellX + 1] + verticalCellPositions[microcellY + 1]);
        const QPointF topRightCorner(horizontalCellPositions[microcellX + 1 + 1] + verticalCellPositions[microcellY + 1]);
        const QPointF bottomRightCorner(horizontalCellPositions[microcellX + 1 + 1] + verticalCellPositions[microcellY + 1 + 1]);
        const QPointF bottomLeftCorner(horizontalCellPositions[microcellX + 1] + verticalCellPositions[microcellY + 1 + 1]);
        if (pointBelongsToLeftSideOfEdge(pixelCenterPoint, topLeftCorner, l1)) {
            --microcellY;
        }
        if (pointBelongsToLeftSideOfEdge(pixelCenterPoint, topRightCorner, l2)) {
            ++microcellX;
        }
        if (pointBelongsToLeftSideOfEdge(pixelCenterPoint, bottomRightCorner, l3)) {
            ++microcellY;
        }
        if (pointBelongsToLeftSideOfEdge(pixelCenterPoint, bottomLeftCorner, l4)) {
            --microcellX;
        }
        // Discard the pixel since the modified microcell position is for a cell
        // that belongs to another macrocell
        if (microcellX < 0 || microcellX >= m_macrocellSize.width() ||
            microcellY < 0 || microcellY >= m_macrocellSize.height()) {
            continue;
        }
        // Add the pixel to the auxiliary points collection
        const int microcellIndex = microcellY * m_macrocellSize.width() + microcellX;
        const qreal microcellPixelIndexX = macrocellPos.x() - static_cast<qreal>(microcellX) * v1MicrocellLength;
        const qreal microcellPixelIndexY = macrocellPos.y() - static_cast<qreal>(microcellY) * v2MicrocellLength;
        const qreal microcellPixelIndex = microcellPixelIndexY * v1MicrocellLength + microcellPixelIndexX;
        const QPointF pixelCornerScreenPoint = m_templateToScreenTransform.map(pixelCornerPoint);
        auxiliaryMicrocells[microcellIndex].auxiliaryPoints.push_back(
            {
                i,
                microcellPixelIndex,
                screentoneFunction(pixelCornerScreenPoint.x(), pixelCornerScreenPoint.y()),
                screentoneFunction(pixelCenterScreenPoint.x(), pixelCenterScreenPoint.y()),
                lightUpOrderingFunction(pixelCornerScreenPoint.x(), pixelCornerScreenPoint.y()),
                lightUpOrderingFunction(pixelCenterScreenPoint.x(), pixelCenterScreenPoint.y()),
            }
        );
        ++totalNumberOfAuxiliaryPoints;
    }

    // Sort the auxiliary points of each auxiliary microcell with respect to
    // different criteria
    for (int i = 0; i < auxiliaryMicrocells.size(); ++i) {
        std::sort(auxiliaryMicrocells[i].auxiliaryPoints.begin(), auxiliaryMicrocells[i].auxiliaryPoints.end(),
            [](const AuxiliaryPoint &a, const AuxiliaryPoint &b) {
                if (qFuzzyCompare(a.valueAtCorner, b.valueAtCorner)) {
                    if (qFuzzyCompare(a.valueAtCenter, b.valueAtCenter)) {
                        if (qFuzzyCompare(a.distanceToCenter, b.distanceToCenter)) {
                            if (qFuzzyCompare(a.distanceToCorner, b.distanceToCorner)) {
                                return a.microcellPixelIndex < b.microcellPixelIndex;
                            }
                            return a.distanceToCorner < b.distanceToCorner;
                        }
                        return a.distanceToCenter < b.distanceToCenter;
                    }
                    return a.valueAtCenter < b.valueAtCenter;
                }
                return a.valueAtCorner < b.valueAtCorner;
            }
        );
    }

    // Sort the auxiliary microcells themselves
    std::sort(auxiliaryMicrocells.begin(), auxiliaryMicrocells.end(),
        [](const AuxiliaryMicrocell &a, const AuxiliaryMicrocell &b) {
            return a.index < b.index;
        }
    );

    // Fill the template pixels inside the macrocell with the sorted auxiliary
    // points
    {
        int macrocellPixelIndex = 0;
        QVector<int> microcellPixelIndides(auxiliaryMicrocells.size());
        while (macrocellPixelIndex < totalNumberOfAuxiliaryPoints) {
            for (int i = 0; i < auxiliaryMicrocells.size(); ++i) {
                if (microcellPixelIndides[i] == auxiliaryMicrocells[i].auxiliaryPoints.size()) {
                    continue;
                }
                m_templateData[auxiliaryMicrocells[i].auxiliaryPoints[microcellPixelIndides[i]].templatePixelIndex] =
                    static_cast<qreal>(macrocellPixelIndex) / static_cast<qreal>(totalNumberOfAuxiliaryPoints - 1);
                ++microcellPixelIndides[i];
                ++macrocellPixelIndex;
            }
        }
    }

    // Fill the rest of the template pixels by copying the values from the
    // already set ones
    for (int i = 0; i < m_templateData.size(); ++i) {
        if (m_templateData[i] < 0.0) {
            int templateY = i / m_templateSize.width();
            int templateX = i - m_templateSize.width() * templateY;
            QPointF pixelCenterPoint(
                static_cast<qreal>(templateX - m_originOffset.x()) + 0.5,
                static_cast<qreal>(templateY - m_originOffset.y()) + 0.5
            );
            const QPointF pixelCenterScreenPoint = m_templateToScreenTransform.map(pixelCenterPoint);
            const qreal a = -std::floor(pixelCenterScreenPoint.x() / macrocellSizeF.width());
            const qreal b = -std::floor(pixelCenterScreenPoint.y() / macrocellSizeF.height());
            pixelCenterPoint += QPointF(a * v1.x() + b * v2.x(), a * v1.y() + b * v2.y());

            int x = static_cast<int>(std::floor(pixelCenterPoint.x())) + m_originOffset.x();
            int y = static_cast<int>(std::floor(pixelCenterPoint.y())) + m_originOffset.y();
            int macrocellPointIndex = y * m_templateSize.width() + x;

            // if for some reason the target reference pixel is not set, we try
            // find another one just by wrapping around the macrocell
            if (m_templateData[macrocellPointIndex] < 0.0) {
                if (pointBelongsToLeftSideOfEdge(pixelCenterPoint, QPointF(0.0, 0.0), l1)) {
                    pixelCenterPoint += v2;
                }
                if (pointBelongsToLeftSideOfEdge(pixelCenterPoint, v1, l2)) {
                    pixelCenterPoint -= v1;
                }
                if (pointBelongsToLeftSideOfEdge(pixelCenterPoint, v3, l3)) {
                    pixelCenterPoint -= v2;
                }
                if (pointBelongsToLeftSideOfEdge(pixelCenterPoint, v2, l4)) {
                    pixelCenterPoint += v1;
                }
                templateX = static_cast<int>(std::floor(pixelCenterPoint.x())) + m_originOffset.x();
                templateY = static_cast<int>(std::floor(pixelCenterPoint.y())) + m_originOffset.y();
                macrocellPointIndex = templateY * m_templateSize.width() + templateX;
            }

            m_templateData[i] = m_templateData[macrocellPointIndex];
        }
    }
}

QVector<int> KisScreentoneGeneratorTemplate::makeCellOrderList(int macrocellColumns, int macrocellRows) const
{
    if (macrocellColumns == 1 && macrocellRows == 1) {
        return {0};
    }

    struct Candidate
    {
        int availableCellIndex;
        int index;
        int distanceSquared;
        int averageDistance;
    };

    const int numberOfCells = macrocellColumns * macrocellRows;
    QVector<int> processedCells;
    QVector<int> availableCells;

    // Insert the cell with index 0 in the first position
    processedCells.push_back(0);
    for (int i = 1; i < numberOfCells; ++i) {
        availableCells.push_back(i);
    }

    while (availableCells.size() > 0) {
        QVector<Candidate> candidates;
        for (int i = 0; i < availableCells.size(); ++i) {
            const int availableCellY = availableCells[i] / macrocellColumns;
            const int availableCellX = availableCells[i] - availableCellY * macrocellColumns;
            int averageDistanceToProcessedCells = 0;
            int minimumDistanceToProcessedCells = std::numeric_limits<int>::max();
            for (int j = 0; j < processedCells.size(); ++j) {
                const int processedCellY = processedCells[j] / macrocellColumns;
                const int processedCellX = processedCells[j] - processedCellY * macrocellColumns;

                int distanceToCurrentProcessedCell = std::numeric_limits<int>::max();
                for (int y = -1; y < 2; ++y) {
                    for (int x = -1; x < 2; ++x) {
                        const int xx = processedCellX + macrocellColumns * x;
                        const int yy = processedCellY + macrocellRows * y;
                        const int deltaX = availableCellX - xx;
                        const int deltaY = availableCellY - yy;
                        const int distanceSquared = deltaX * deltaX + deltaY * deltaY;
                        if (distanceSquared < distanceToCurrentProcessedCell) {
                            distanceToCurrentProcessedCell = distanceSquared;
                        }
                    }
                }
                if (distanceToCurrentProcessedCell < minimumDistanceToProcessedCells) {
                    minimumDistanceToProcessedCells = distanceToCurrentProcessedCell;
                }
                averageDistanceToProcessedCells += distanceToCurrentProcessedCell;
            }
            candidates.push_back({i, availableCells[i], minimumDistanceToProcessedCells, averageDistanceToProcessedCells});
        }

        // Find the best candidate
        int bestCandidateIndex = 0;
        for (int i = 0; i < candidates.size(); ++i) {
            if (candidates[i].distanceSquared > candidates[bestCandidateIndex].distanceSquared ||
                (candidates[i].distanceSquared == candidates[bestCandidateIndex].distanceSquared &&
                candidates[i].averageDistance > candidates[bestCandidateIndex].averageDistance)) {
                bestCandidateIndex = i;
            }
        }
        
        processedCells.push_back(candidates[bestCandidateIndex].index);
        availableCells.remove(candidates[bestCandidateIndex].availableCellIndex);
    }

    QVector<int> cellOrderList(numberOfCells);
    for (int i = 1; i < numberOfCells; ++i) {
        cellOrderList[processedCells[i]] = i;
    }

    return cellOrderList;
}
