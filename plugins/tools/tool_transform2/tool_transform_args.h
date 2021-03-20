/*
 *  tool_transform_args.h - part of Krita
 *
 *  SPDX-FileCopyrightText: 2010 Marc Pegon <pe.marc@free.fr>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TOOL_TRANSFORM_ARGS_H_
#define TOOL_TRANSFORM_ARGS_H_

#include <QPointF>
#include <QVector3D>
#include <kis_warptransform_worker.h>
#include <kis_filter_strategy.h>
#include "kis_liquify_properties.h"
#include "kritatooltransform_export.h"
#include "kis_global.h"
#include "KisToolChangesTrackerData.h"
#include "KisBezierTransformMesh.h"
#include "kis_paint_device.h"

#include <QScopedPointer>
class KisLiquifyTransformWorker;
class QDomElement;

/**
 * Class used to store the parameters of a transformation.
 * Some parameters are specific to free transform mode, and
 * others to warp mode : maybe add a union to save a little more
 * memory.
 */

class KRITATOOLTRANSFORM_EXPORT ToolTransformArgs : public KisToolChangesTrackerData
{
public:
    enum TransformMode {FREE_TRANSFORM = 0,
                        WARP,
                        CAGE,
                        LIQUIFY,
                        PERSPECTIVE_4POINT,
                        MESH,
                        N_MODES};

    /**
     * Initializes the parameters for an identity transformation,
     * with mode set to free transform.
     */
    ToolTransformArgs();

    /**
     * The object return will be a copy of args.
     */
    ToolTransformArgs(const ToolTransformArgs& args);

    KisToolChangesTrackerData *clone() const;

    /**
     * If mode is warp, original and transformed vector points will be of size 0.
     * Use setPoints method to set those vectors.
     */
    ToolTransformArgs(TransformMode mode,
                      QPointF transformedCenter,
                      QPointF originalCenter,
                      QPointF rotationCenterOffset, bool transformAroundRotationCenter,
                      double aX, double aY, double aZ,
                      double scaleX, double scaleY,
                      double shearX, double shearY,
                      KisWarpTransformWorker::WarpType warpType,
                      double alpha,
                      bool defaultPoints,
                      const QString &filterId,
                      int pixelPrecision, int previewPixelPrecision,
                      KisPaintDeviceSP externalSource);
    ~ToolTransformArgs();
    ToolTransformArgs& operator=(const ToolTransformArgs& args);

    bool operator==(const ToolTransformArgs& other) const;
    bool isSameMode(const ToolTransformArgs& other) const;

    inline TransformMode mode() const {
        return m_mode;
    }
    inline void setMode(TransformMode mode) {
        m_mode = mode;
    }

    inline int pixelPrecision() const {
        return m_pixelPrecision;
    }

    inline void setPixelPrecision(int precision) {
        m_pixelPrecision = precision;
    }

    inline int previewPixelPrecision() const {
        return m_previewPixelPrecision;
    }

    inline void setPreviewPixelPrecision(int precision) {
        m_previewPixelPrecision = precision;
    }

    inline KisPaintDeviceSP externalSource() const {
        return m_externalSource;
    }

    inline void setExternalSource(KisPaintDeviceSP externalSource) {
        m_externalSource = externalSource;
    }

    //warp-related
    inline int numPoints() const {
        KIS_ASSERT_RECOVER_NOOP(m_origPoints.size() == m_transfPoints.size());
        return m_origPoints.size();
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

    inline QVector<QPointF> &refOriginalPoints() {
        return m_origPoints;
    }
    inline QVector<QPointF> &refTransformedPoints() {
        return m_transfPoints;
    }

    inline KisWarpTransformWorker::WarpType warpType() const {
        return m_warpType;
    }
    inline double alpha() const {
        return m_alpha;
    }
    inline bool defaultPoints() const {
        return m_defaultPoints;
    }
    inline void setPoints(QVector<QPointF> origPoints, QVector<QPointF> transfPoints) {
        m_origPoints = QVector<QPointF>(origPoints);
        m_transfPoints = QVector<QPointF>(transfPoints);
    }
    inline void setWarpType(KisWarpTransformWorker::WarpType warpType) {
        m_warpType = warpType;
    }
    inline void setWarpCalculation(KisWarpTransformWorker::WarpCalculation warpCalc) {
        m_warpCalculation = warpCalc;
    }
    inline KisWarpTransformWorker::WarpCalculation warpCalculation() {
        return m_warpCalculation;
    }

    inline void setAlpha(double alpha) {
        m_alpha = alpha;
    }
    inline void setDefaultPoints(bool defaultPoints) {
        m_defaultPoints = defaultPoints;
    }

    //"free transform"-related
    inline QPointF transformedCenter() const {
        return m_transformedCenter;
    }
    inline QPointF originalCenter() const {
        return m_originalCenter;
    }
    inline QPointF rotationCenterOffset() const {
        return m_rotationCenterOffset;
    }
    inline bool transformAroundRotationCenter() const {
        return m_transformAroundRotationCenter;
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
    inline QVector3D cameraPos() const {
        return m_cameraPos;
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

    inline void setTransformedCenter(QPointF transformedCenter) {
        m_transformedCenter = transformedCenter;
    }
    inline void setOriginalCenter(QPointF originalCenter) {
        m_originalCenter = originalCenter;
    }
    inline void setRotationCenterOffset(QPointF rotationCenterOffset) {
        m_rotationCenterOffset = rotationCenterOffset;
    }
    void setTransformAroundRotationCenter(bool value);

    inline void setAX(double aX) {
        if(qFuzzyCompare(aX, normalizeAngle(aX))) {
            aX = normalizeAngle(aX);
        }

        m_aX = aX;
    }
    inline void setAY(double aY) {
        if(qFuzzyCompare(aY, normalizeAngle(aY))) {
            aY = normalizeAngle(aY);
        }

        m_aY = aY;
    }
    inline void setAZ(double aZ) {
        if(qFuzzyCompare(aZ, normalizeAngle(aZ))) {
            aZ = normalizeAngle(aZ);
        }

        m_aZ = aZ;
    }
    inline void setCameraPos(const QVector3D &pos) {
        m_cameraPos = pos;
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

    inline QString filterId() const {
        return m_filter->id();
    }

    void setFilterId(const QString &id);

    inline KisFilterStrategy* filter() const {
        return m_filter;
    }

    // True if the transformation does not differ from the initial one. The
    // target device may still need changing if we are placing an external source.
    bool isIdentity() const;

    // True if the target device does not need changing as a result of this
    // transformation, because the tranformation does not differ from the initial
    // one and the source image is not external.
    bool isUnchanging() const;

    inline QTransform flattenedPerspectiveTransform() const {
        return m_flattenedPerspectiveTransform;
    }

    inline void setFlattenedPerspectiveTransform(const QTransform &value) {
        m_flattenedPerspectiveTransform = value;
    }

    bool isEditingTransformPoints() const {
        return m_editTransformPoints;
    }

    void setEditingTransformPoints(bool value) {
        m_editTransformPoints = value;
    }

    const KisLiquifyProperties* liquifyProperties() const {
        return m_liquifyProperties.data();
    }

    KisLiquifyProperties* liquifyProperties() {
        return m_liquifyProperties.data();
    }

    void initLiquifyTransformMode(const QRect &srcRect);
    void saveLiquifyTransformMode() const;

    KisLiquifyTransformWorker* liquifyWorker() const {
        return m_liquifyWorker.data();
    }

    void toXML(QDomElement *e) const;
    static ToolTransformArgs fromXML(const QDomElement &e);

    void translate(const QPointF &offset);

    void saveContinuedState();
    void restoreContinuedState();
    const ToolTransformArgs* continuedTransform() const;

    const KisBezierTransformMesh* meshTransform() const;
    KisBezierTransformMesh* meshTransform();

    bool meshShowHandles() const;
    void setMeshShowHandles(bool value);

    bool meshSymmetricalHandles() const;
    void setMeshSymmetricalHandles(bool meshSymmetricalHandles);

    bool meshScaleHandles() const;
    void setMeshScaleHandles(bool meshScaleHandles);

    void scaleSrcAndDst(qreal scale);

private:
    void clear();
    void init(const ToolTransformArgs& args);
    TransformMode m_mode {ToolTransformArgs::TransformMode::FREE_TRANSFORM};

    // warp-related arguments
    // these are basically the arguments taken by the warp transform worker
    bool m_defaultPoints {true}; // true : the original points are set to make a grid
                          // which density is given by numPoints()
    QVector<QPointF> m_origPoints;
    QVector<QPointF> m_transfPoints;
    KisWarpTransformWorker::WarpType m_warpType {KisWarpTransformWorker::WarpType_::RIGID_TRANSFORM};
    KisWarpTransformWorker::WarpCalculation m_warpCalculation {KisWarpTransformWorker::WarpCalculation::DRAW}; // DRAW or GRID
    double m_alpha {1.0};

    //'free transform'-related
    // basically the arguments taken by the transform worker
    QPointF m_transformedCenter;
    QPointF m_originalCenter;
    QPointF m_rotationCenterOffset; // the position of the rotation center relative to
                                    // the original top left corner of the selection
                                    // before any transformation
    bool m_transformAroundRotationCenter {false}; // In freehand mode makes the scaling and other transformations
                                          // be anchored to the rotation center point.

    double m_aX {0};
    double m_aY {0};
    double m_aZ {0};
    QVector3D m_cameraPos {QVector3D(0,0,1024)};
    double m_scaleX {1.0};
    double m_scaleY {1.0};
    double m_shearX {0.0};
    double m_shearY {0.0};
    bool m_keepAspectRatio {false};

    // perspective trasform related
    QTransform m_flattenedPerspectiveTransform;

    KisFilterStrategy *m_filter {0};
    bool m_editTransformPoints {false};
    QSharedPointer<KisLiquifyProperties> m_liquifyProperties;
    QScopedPointer<KisLiquifyTransformWorker> m_liquifyWorker;

    KisBezierTransformMesh m_meshTransform;
    bool m_meshShowHandles = true;
    bool m_meshSymmetricalHandles = true;
    bool m_meshScaleHandles = false;

    /**
     * When we continue a transformation, m_continuedTransformation
     * stores the initial step of our transform. All cancel and revert
     * operations should revert to it.
     */
    QScopedPointer<ToolTransformArgs> m_continuedTransformation;

    //PixelPrecision should always be in powers of 2
    int m_pixelPrecision {8};
    int m_previewPixelPrecision {16};

    /**
     * Optional external image, for example from the clipboard, that
     * can be transformed directly over an existing paint layer or mask.
     */
    KisPaintDeviceSP m_externalSource;
};

#endif // TOOL_TRANSFORM_ARGS_H_
