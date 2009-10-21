/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#ifndef KIS_CONFIG_H_
#define KIS_CONFIG_H_

#include <QString>
#include <QColor>

#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include "kis_global.h"
#include "krita_export.h"

class KRITAUI_EXPORT KisConfig
{
public:
    KisConfig();
    ~KisConfig();

    bool useProjections() const;
    void setUseProjections(bool useProj);

    bool undoEnabled() const;
    void setUndoEnabled(bool undo);

    qint32 defImgWidth() const;
    void defImgWidth(qint32 width);

    qint32 defImgHeight() const;
    void defImgHeight(qint32 height);

    double defImgResolution() const;
    void defImgResolution(double res);

    enumCursorStyle cursorStyle() const;
    enumCursorStyle getDefaultCursorStyle() const;
    void setCursorStyle(enumCursorStyle style);

    QString monitorProfile() const;
    void setMonitorProfile(const QString & monitorProfile);

    QString workingColorSpace() const;
    void setWorkingColorSpace(const QString & workingColorSpace);

    QString importProfile() const;
    void setImportProfile(const QString & importProfile);

    QString printerColorSpace() const;
    void setPrinterColorSpace(const QString & printerColorSpace);

    QString printerProfile() const;
    void setPrinterProfile(const QString & printerProfile);

    bool useBlackPointCompensation() const;
    void setUseBlackPointCompensation(bool useBlackPointCompensation);

    bool showRulers() const;
    void setShowRulers(bool rulers);

    qint32 pasteBehaviour() const;
    void setPasteBehaviour(qint32 behaviour);

    qint32 renderIntent() const;
    void setRenderIntent(qint32 renderIntent);

    bool useOpenGL() const;
    void setUseOpenGL(bool useOpenGL);

    bool useOpenGLShaders() const;
    void setUseOpenGLShaders(bool useOpenGLShaders);

    qint32 maxNumberOfThreads();
    void setMaxNumberOfThreads(qint32 numberOfThreads);

    /// Maximum tiles in memory (this is a guideline, not absolute)
    qint32 maxTilesInMem() const;
    void setMaxTilesInMem(qint32 tiles);

    /// Number of tiles that will be swapped at once. The higher, the more swapped, but more
    /// chance that it will become slow
    qint32 swappiness() const;
    void setSwappiness(qint32 swappiness);

    quint32 getGridMainStyle();
    void setGridMainStyle(quint32 v);
    quint32 getGridSubdivisionStyle();
    void setGridSubdivisionStyle(quint32 v);
    QColor getGridMainColor();
    void setGridMainColor(const QColor & v);
    QColor getGridSubdivisionColor();
    void setGridSubdivisionColor(const QColor & v);
    quint32 getGridHSpacing();
    void setGridHSpacing(quint32 v);
    quint32 getGridVSpacing();
    void setGridVSpacing(quint32 v);
    quint32 getGridSubdivisions();
    void setGridSubdivisions(quint32 v);
    quint32 getGridOffsetX();
    void setGridOffsetX(quint32 v);
    quint32 getGridOffsetY();
    void setGridOffsetY(quint32 v);

    qint32 checkSize();
    void setCheckSize(qint32 checkSize);

    bool scrollCheckers() const;
    void setScrollingCheckers(bool scollCheckers);

    QColor checkersColor();
    void setCheckersColor(const QColor & v);

    int numProjectionThreads();
    void setNumProjectThreads(int num);

    int projectionChunkSize();
    void setProjectionChunkSize(int num);

    bool aggregateDirtyRegionsInPainter();
    void setAggregateDirtyRegionsInPainter(bool aggregate);

    bool useBoundingRectInProjection();
    void setUseBoundingRectInProjection(bool use);

    bool useRegionOfInterestInProjection();
    void setUseRegionOfInterestInProjection(bool use);

    // Use nearest-neighbour interpolation on KisImage
    bool useNearestNeighbour();
    void setUseNearestNeighbour(bool useNearestNeigbour);

    // Use Mipmapping (KisImagePyramid) in KisPrescaledProjection
    bool useMipmapping();
    void setUseMipmapping(bool useMipmapping);

    // Use Blitz sampling on a QImage
    bool useSampling();
    void setSampling(bool sampling);

    bool threadColorSpaceConversion();
    void setThreadColorSpaceConversion(bool threadColorSpaceConversion);

    bool cacheKisImageAsQImage();
    void setCacheKisImageAsQImage(bool cacheKisImageAsQImage);

    bool drawMaskVisualisationOnUnscaledCanvasCache();
    void setDrawMaskVisualisationOnUnscaledCanvasCache(bool drawMaskVisualisationOnUnscaledCanvasCache);

    bool fastZoom() {
        return false;
    }

    // If there's no XRender use QPixmaps instead of QImage for the QPainterCanvas
    bool noXRender();
    void setNoXRender(bool noXRender);

    bool showRootLayer();
    void setShowRootLayer(bool showRootLayer);

    // in megapixels -- above 5, we will no longer use the
    // memory-guzzling qimage canvas cache
    quint32 maxCachedImageSize();
    void setMaxCachedImageSize(quint32);

    bool showFilterGallery();
    void setShowFilterGallery(bool showFilterGallery);


private:
    KisConfig(const KisConfig&);
    KisConfig& operator=(const KisConfig&);

private:
    mutable KConfigGroup m_cfg;
};

#endif // KIS_CONFIG_H_
