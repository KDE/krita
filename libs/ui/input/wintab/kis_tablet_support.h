/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_TABLET_SUPPORT_H
#define __KIS_TABLET_SUPPORT_H

#include <QtGlobal>
#include <QLibrary>
#include <QPointer>
#include <QPointF>
#include <QVector>
#include <QList>
#include <QMap>
#include <QTabletEvent>

#ifdef HAVE_X11
#include "kis_config.h"
#include <algorithm>
#include <X11/extensions/XInput.h>
#include <kis_global.h>
#include "kis_incremental_average.h"
#endif


struct QTabletDeviceData
{
#ifndef Q_OS_OSX
    int minPressure;
    int maxPressure;
    int minTanPressure;
    int maxTanPressure;
    int minX, maxX, minY, maxY, minZ, maxZ;
    int sysOrgX, sysOrgY, sysExtX, sysExtY;
#ifdef HAVE_X11 // on windows the scale is fixed (and hardcoded)
    int minRotation;
    int maxRotation;
#endif /* HAVE_X11 */
    inline QPointF scaleCoord(int coordX, int coordY, int outOriginX, int outExtentX,
                              int outOriginY, int outExtentY) const;
#endif

#if defined(HAVE_X11) || (defined(Q_OS_OSX) && !defined(QT_MAC_USE_COCOA))
    QPointer<QWidget> widgetToGetPress;
#endif

#ifdef HAVE_X11
    int deviceType;
    enum {
        TOTAL_XINPUT_EVENTS = 64
    };
    void *device;
    int eventCount;
    long unsigned int eventList[TOTAL_XINPUT_EVENTS]; // XEventClass is in fact a long unsigned int

    int xinput_motion;
    int xinput_key_press;
    int xinput_key_release;
    int xinput_button_press;
    int xinput_button_release;
    int xinput_proximity_in;
    int xinput_proximity_out;
#elif defined(Q_OS_OSX)
    quint64 tabletUniqueID;
    int tabletDeviceType;
    int tabletPointerType;
    int capabilityMask;
#endif

    // Added by Krita
#if defined(Q_OS_OSX) || defined(Q_OS_WIN32)
  QMap<quint8, quint8> buttonsMap;
  qint64 llId;
  int currentPointerType;
  int currentDevice;
#endif

#ifdef HAVE_X11
    bool isTouchWacomTablet;

    /**
     * Different tablets have different assignment of axes reported by
     * the XInput subsystem. More than that some of the drivers demand
     * local storage of the tablet axes' state, because in the events
     * they report only recently changed axes.  SavedAxesData was
     * created to handle all these complexities.
     */
    class SavedAxesData {
    public:
        enum AxesIndexes {
            XCoord = 0,
            YCoord,
            Pressure,
            XTilt,
            YTilt,
            Rotation,
            Unused,

            NAxes
        };

    public:
        SavedAxesData()
            : m_workaroundX11SmoothPressureSteps(KisConfig(true).workaroundX11SmoothPressureSteps()),
              m_pressureAverage(m_workaroundX11SmoothPressureSteps)
        {
            for (int i = 0; i < NAxes; i++) {
                m_x11_to_local_axis_mapping.append((AxesIndexes)i);
            }

            if (m_workaroundX11SmoothPressureSteps) {
                warnKrita << "WARNING: Workaround for broken tablet"
                           << "pressure reports is activated. Number"
                           << "of smooth steps:"
                           << m_workaroundX11SmoothPressureSteps;
            }
        }

        void tryFetchAxesMapping(XDevice *dev);

        void setAxesMap(const QVector<AxesIndexes> &axesMap) {
            // the size of \p axesMap can be smaller/equal/bigger
            // than m_axes_data. Everything depends on the driver

            m_x11_to_local_axis_mapping = axesMap;
        }

        inline QPointF position(const QTabletDeviceData *tablet, const QRect &screenArea) const {
            return tablet->scaleCoord(m_axis_data[XCoord], m_axis_data[YCoord],
                              screenArea.x(), screenArea.width(),
                              screenArea.y(), screenArea.height());
        }

        inline int pressure() const {
            return m_axis_data[Pressure];
        }

        inline int xTilt() const {
            return m_axis_data[XTilt];
        }

        inline int yTilt() const {
            return m_axis_data[YTilt];
        }

        inline int rotation() const {
            return m_axis_data[Rotation];
        }

        bool updateAxesData(int firstAxis, int axesCount, const int *axes) {
            for (int srcIt = 0, dstIt = firstAxis;
                 srcIt < axesCount;
                 srcIt++, dstIt++) {

                int index = m_x11_to_local_axis_mapping[dstIt];
                int newValue = axes[srcIt];

                if (m_workaroundX11SmoothPressureSteps > 0 &&
                    index == Pressure) {
                    newValue = m_pressureAverage.pushThrough(newValue);
                }

                m_axis_data[index] = newValue;
            }

            return true;
        }
    private:
        int m_axis_data[NAxes];
        QVector<AxesIndexes> m_x11_to_local_axis_mapping;
        int m_workaroundX11SmoothPressureSteps;
        KisIncrementalAverage m_pressureAverage;
    };

    SavedAxesData savedAxesData;
#endif /* HAVE_X11 */
};

static inline int sign(int x)
{
    return x >= 0 ? 1 : -1;
}

#ifndef Q_OS_OSX
inline QPointF QTabletDeviceData::scaleCoord(int coordX, int coordY,
                                            int outOriginX, int outExtentX,
                                            int outOriginY, int outExtentY) const
{
    QPointF ret;

    if (sign(outExtentX) == sign(maxX))
        ret.setX(((coordX - minX) * qAbs(outExtentX) / qAbs(qreal(maxX - minX))) + outOriginX);
    else
        ret.setX(((qAbs(maxX) - (coordX - minX)) * qAbs(outExtentX) / qAbs(qreal(maxX - minX)))
                 + outOriginX);

    if (sign(outExtentY) == sign(maxY))
        ret.setY(((coordY - minY) * qAbs(outExtentY) / qAbs(qreal(maxY - minY))) + outOriginY);
    else
        ret.setY(((qAbs(maxY) - (coordY - minY)) * qAbs(outExtentY) / qAbs(qreal(maxY - minY)))
                 + outOriginY);

    return ret;
}
#endif

typedef QList<QTabletDeviceData> QTabletDeviceDataList;
QTabletDeviceDataList *qt_tablet_devices();


#endif /* __KIS_TABLET_SUPPORT_H */
