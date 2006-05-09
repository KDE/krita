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
#include "krita_export.h"

class KRITAUI_EXPORT KisConfig {
public:
    KisConfig();
    ~KisConfig();

    bool fixDockerWidth() const;
    void setFixedDockerWidth(bool fix);

    bool undoEnabled() const;
    void setUndoEnabled(bool undo);

    qint32 defUndoLimit() const;
    void defUndoLimit(qint32 limit);

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

    qint32 getPressureCorrection();
    void setPressureCorrection( qint32 correction);
    qint32 getDefaultPressureCorrection();

    bool tabletDeviceEnabled(const QString& tabletDeviceName) const;
    void setTabletDeviceEnabled(const QString& tabletDeviceName, bool enabled);

    qint32 tabletDeviceAxis(const QString& tabletDeviceName, const QString& axisName, qint32 defaultAxis) const;
    void setTabletDeviceAxis(const QString& tabletDeviceName, const QString& axisName, qint32 axis) const;

    qint32 dockability();
    qint32 getDefaultDockability();
    void setDockability( qint32 dockability);

    float dockerFontSize();
    float getDefaultDockerFontSize();
    void setDockerFontSize(float);


    quint32 getGridMainStyle();
    void setGridMainStyle(quint32 v);
    quint32 getGridSubdivisionStyle();
    void setGridSubdivisionStyle(quint32 v);
    QColor getGridMainColor();
    void setGridMainColor(QColor v);
    QColor getGridSubdivisionColor();
    void setGridSubdivisionColor(QColor v);
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


private:
    KisConfig(const KisConfig&);
    KisConfig& operator=(const KisConfig&);

private:
    mutable KConfig *m_cfg;
};

#endif // KIS_CONFIG_H_
