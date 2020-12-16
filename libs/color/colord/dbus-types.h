/***************************************************************************
 *   SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>           *
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#ifndef DBUS_TYPES_H
#define DBUS_TYPES_H

#include <QMap>
#include <QMetaType>

typedef QMap<QString, QString> CdStringMap;
Q_DECLARE_METATYPE(CdStringMap)

struct Gamma {
    double red;
    double green;
    double blue;
};
typedef QList<Gamma> CdGamaList;
Q_DECLARE_METATYPE(CdGamaList)

/* defined in org.freedesktop.ColorManager.Device.xml */
#define CD_DEVICE_PROPERTY_MODEL                "Model"
#define CD_DEVICE_PROPERTY_KIND                 "Kind"
#define CD_DEVICE_PROPERTY_VENDOR               "Vendor"
#define CD_DEVICE_PROPERTY_SERIAL               "Serial"
#define CD_DEVICE_PROPERTY_COLORSPACE           "Colorspace"
#define CD_DEVICE_PROPERTY_FORMAT               "Format"
#define CD_DEVICE_PROPERTY_MODE                 "Mode"
#define CD_DEVICE_PROPERTY_PROFILES             "Profiles"
#define CD_DEVICE_PROPERTY_CREATED              "Created"
#define CD_DEVICE_PROPERTY_MODIFIED             "Modified"
#define CD_DEVICE_PROPERTY_METADATA             "Metadata"
#define CD_DEVICE_PROPERTY_ID                   "DeviceId"
#define CD_DEVICE_PROPERTY_SCOPE                "Scope"
#define CD_DEVICE_PROPERTY_OWNER                "Owner"
#define CD_DEVICE_PROPERTY_SEAT                 "Seat"
#define CD_DEVICE_PROPERTY_PROFILING_INHIBITORS "ProfilingInhibitors"
#define CD_DEVICE_PROPERTY_ENABLED              "Enabled"
#define CD_DEVICE_PROPERTY_EMBEDDED             "Embedded"

/* defined in metadata-spec.txt */
#define CD_DEVICE_METADATA_XRANDR_NAME          "XRANDR_name"
#define CD_DEVICE_METADATA_OUTPUT_EDID_MD5      "OutputEdidMd5"
#define CD_DEVICE_METADATA_OUTPUT_PRIORITY      "OutputPriority"
#define CD_DEVICE_METADATA_OUTPUT_PRIORITY_PRIMARY      "primary"
#define CD_DEVICE_METADATA_OUTPUT_PRIORITY_SECONDARY    "secondary"
#define CD_DEVICE_METADATA_OWNER_CMDLINE        "OwnerCmdline"

#endif
