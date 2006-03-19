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

#include "kis_global.h"
#include "koffice_export.h"

class KRITACORE_EXPORT KisConfig {
public:
    KisConfig();
    ~KisConfig();

    bool fixDockerWidth() const;
    void setFixedDockerWidth(bool fix);
    
    bool undoEnabled() const;
    void setUndoEnabled(bool undo);
    
    Q_INT32 defUndoLimit() const;
    void defUndoLimit(Q_INT32 limit);

    Q_INT32 defImgWidth() const;
    void defImgWidth(Q_INT32 width);

    Q_INT32 defImgHeight() const;
    void defImgHeight(Q_INT32 height);

    double defImgResolution() const;
    void defImgResolution(double res);

    enumCursorStyle cursorStyle() const;
    enumCursorStyle getDefaultCursorStyle() const;
    void setCursorStyle(enumCursorStyle style);

    QString monitorProfile() const;
    void setMonitorProfile(QString monitorProfile);

    QString workingColorSpace() const;
    void setWorkingColorSpace(QString workingColorSpace);

    QString importProfile() const;
    void setImportProfile(QString importProfile);

    QString printerColorSpace() const;
    void setPrinterColorSpace(QString printerColorSpace);

    QString printerProfile() const;
    void setPrinterProfile(QString printerProfile);

    bool useBlackPointCompensation() const;
    void setUseBlackPointCompensation(bool useBlackPointCompensation);

    bool showRulers() const;
    void setShowRulers(bool rulers);

    Q_INT32 pasteBehaviour() const;
    void setPasteBehaviour(Q_INT32 behaviour);

    Q_INT32 renderIntent() const;
    void setRenderIntent(Q_INT32 renderIntent);

    bool useOpenGL() const;
    void setUseOpenGL(bool useOpenGL);

    bool useOpenGLShaders() const;
    void setUseOpenGLShaders(bool useOpenGLShaders);

    Q_INT32 maxNumberOfThreads();
    void setMaxNumberOfThreads(Q_INT32 numberOfThreads);

    /// Maximum tiles in memory (this is a guideline, not absolute)
    Q_INT32 maxTilesInMem() const;
    void setMaxTilesInMem(Q_INT32 tiles);

    /// Number of tiles that will be swapped at once. The higher, the more swapped, but more
    /// chance that it will become slow
    Q_INT32 swappiness() const;
    void setSwappiness(Q_INT32 swappiness);

    Q_INT32 getPressureCorrection();
    void setPressureCorrection( Q_INT32 correction);
    Q_INT32 getDefaultPressureCorrection();

    bool tabletDeviceEnabled(const QString& tabletDeviceName) const;
    void setTabletDeviceEnabled(const QString& tabletDeviceName, bool enabled);

    Q_INT32 tabletDeviceAxis(const QString& tabletDeviceName, const QString& axisName, Q_INT32 defaultAxis) const;
    void setTabletDeviceAxis(const QString& tabletDeviceName, const QString& axisName, Q_INT32 axis) const;

    Q_INT32 dockability();
    Q_INT32 getDefaultDockability();
    void setDockability( Q_INT32 dockability);

    float dockerFontSize();
    float getDefaultDockerFontSize();
    void setDockerFontSize(float);

    
    Q_UINT32 getGridMainStyle();
    void setGridMainStyle(Q_UINT32 v);
    Q_UINT32 getGridSubdivisionStyle();
    void setGridSubdivisionStyle(Q_UINT32 v);
    QColor getGridMainColor();
    void setGridMainColor(QColor v);
    QColor getGridSubdivisionColor();
    void setGridSubdivisionColor(QColor v);
    Q_UINT32 getGridHSpacing();
    void setGridHSpacing(Q_UINT32 v);
    Q_UINT32 getGridVSpacing();
    void setGridVSpacing(Q_UINT32 v);
    Q_UINT32 getGridSubdivisions();
    void setGridSubdivisions(Q_UINT32 v);
    Q_UINT32 getGridOffsetX();
    void setGridOffsetX(Q_UINT32 v);
    Q_UINT32 getGridOffsetY();
    void setGridOffsetY(Q_UINT32 v);

    
private:
    KisConfig(const KisConfig&);
    KisConfig& operator=(const KisConfig&);

private:
    mutable KConfig *m_cfg;
};

#endif // KIS_CONFIG_H_
