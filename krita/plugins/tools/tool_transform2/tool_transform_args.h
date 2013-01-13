/*
 *  tool_transform_args.h - part of Krita
 *
 *  Copyright (c) 2010 Marc Pegon <pe.marc@free.fr>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef TOOL_TRANSFORM_ARGS_H_
#define TOOL_TRANSFORM_ARGS_H_

#include <QPointF>
#include <kis_warptransform_worker.h>

/**
 * Class used to store the parameters of a transformation.
 * Some parameters are specific to free transform mode, and
 * others to warp mode : maybe add a union to save a little more
 * memory.
 */

class ToolTransformArgs
{
public:
    typedef enum TransformMode_ {FREE_TRANSFORM = 0, WARP} TransformMode;

    /**
     * Initializes the parameters for an identity transformation,
     * with mode set to free transform.
     */
    ToolTransformArgs();

    /**
     * The object return will be a copy of args.
     */
    ToolTransformArgs(const ToolTransformArgs& args);

    /**
     * If mode is warp, original and transformed vector points will be of size 0.
     * Use setPoints method to set those vectors.
     */
    ToolTransformArgs(TransformMode mode,
                      QPointF translate, QPointF rotationCenterOffset, double aX, double aY, double aZ, double scaleX, double scaleY, double shearX, double shearY,
                      KisWarpTransformWorker::WarpType warpType, double alpha, QPointF previewPos, bool defaultPoints);
    ~ToolTransformArgs();
    ToolTransformArgs& operator=(const ToolTransformArgs& args);

    inline TransformMode mode() const {
        return m_mode;
    }
    inline void setMode(TransformMode mode) {
        m_mode = mode;
    }

    //warp-related
    inline int pointsPerLine() const {
        return m_pointsPerLine;
    }
    inline QPointF &origPoint(int i) {
        return m_origPoints[i];
    }
    inline QPointF &transfPoint(int i) {
        return m_transfPoints[i];
    }
    inline const QVector<QPointF> &origPoints() const {
        return m_origPoints;
    }
    inline const QVector<QPointF> &transfPoints() const {
        return m_transfPoints;
    }
    inline KisWarpTransformWorker::WarpType warpType() const {
        return m_warpType;
    }
    inline double alpha() const {
        return m_alpha;
    }
    inline QPointF previewPos() const {
        return m_previewPos;
    }
    inline bool defaultPoints() const {
        return m_defaultPoints;
    }
    inline void setPointsPerLine(int pointsPerLine) {
        m_pointsPerLine = pointsPerLine;
    }
    inline void setPoints(QVector<QPointF> origPoints, QVector<QPointF> transfPoints) {
        m_origPoints = QVector<QPointF>(origPoints);
        m_transfPoints = QVector<QPointF>(transfPoints);
        m_pointsPerLine = m_origPoints.size();
    }
    inline void setWarpType(KisWarpTransformWorker::WarpType warpType) {
        m_warpType = warpType;
    }
    inline void setAlpha(double alpha) {
        m_alpha = alpha;
    }
    inline void setPreviewPos(QPointF previewPos) {
        m_previewPos = previewPos;
    }
    inline void setDefaultPoints(bool defaultPoints) {
        m_defaultPoints = defaultPoints;
    }

    //"free transform"-related
    inline QPointF translate() const {
        return m_translate;
    }
    inline QPointF rotationCenterOffset() const {
        return m_rotationCenterOffset;
    }
    inline double aX() const {
        return m_aX;
    }
    inline double aY() const {
        return m_aY;
    }
    inline double aZ() const {
        return m_aZ;
    }
    inline double scaleX() const {
        return m_scaleX;
    }
    inline double scaleY() const {
        return m_scaleY;
    }
    inline bool keepAspectRatio() const {
        return m_keepAspectRatio;
    }
    inline double shearX() const {
        return m_shearX;
    }
    inline double shearY() const {
        return m_shearY;
    }

    inline void setTranslate(QPointF translate) {
        m_translate = translate;
    }
    inline void setRotationCenterOffset(QPointF rotationCenterOffset) {
        m_rotationCenterOffset = rotationCenterOffset;
    }
    inline void setAX(double aX) {
        m_aX = aX;
    }
    inline void setAY(double aY) {
        m_aY = aY;
    }
    inline void setAZ(double aZ) {
        m_aZ = aZ;
    }
    inline void setScaleX(double scaleX) {
        m_scaleX = scaleX;
    }
    inline void setScaleY(double scaleY) {
        m_scaleY = scaleY;
    }
    inline void setKeepAspectRatio(bool value) {
        m_keepAspectRatio = value;
    }
    inline void setShearX(double shearX) {
        m_shearX = shearX;
    }
    inline void setShearY(double shearY) {
        m_shearY = shearY;
    }

    bool isIdentity(QPointF originalTranslate) const;

private:
    void clear();
    void init(const ToolTransformArgs& args);
    TransformMode m_mode;

    // warp-related arguments
    // these are basically the arguments taken by the warp transform worker
    bool m_defaultPoints; // true : the original points are set to make a grid
                          // which density is given by pointsPerLine
    int m_pointsPerLine; // density of the grid when defaultPoints is true
    QVector<QPointF> m_origPoints;
    QVector<QPointF> m_transfPoints;
    KisWarpTransformWorker::WarpType m_warpType;
    double m_alpha;
    QPointF m_previewPos;

    //'free transform'-related
    // basically the arguments taken by the transform worker
    QPointF m_translate;
    QPointF m_rotationCenterOffset; // the position of the rotation center relative to
                                    // the original top left corner of the selection
                                    // before any transformation
    double m_aX;
    double m_aY;
    double m_aZ;
    double m_scaleX;
    double m_scaleY;
    double m_shearX;
    double m_shearY;
    bool m_keepAspectRatio;
};

#endif // TOOL_TRANSFORM_ARGS_H_
