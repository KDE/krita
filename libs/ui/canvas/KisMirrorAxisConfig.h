/*
 *  Copyright (c) 2019 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#ifndef KISMIRRORAXISCONFIG_H
#define KISMIRRORAXISCONFIG_H

#include <QScopedPointer>

#include "kritaui_export.h"
#include <boost/operators.hpp>

class QDomElement;
class QDomDocument;

/**
 * @brief The KisMirrorAxisConfig class stores configuration for the KisMirrorAxis
 * canvas decoration. Contents are saved to/loaded from KRA documents.
 */

class KRITAUI_EXPORT KisMirrorAxisConfig : public QObject, boost::equality_comparable<KisMirrorAxisConfig>
{
    Q_OBJECT

public:
    KisMirrorAxisConfig();
    ~KisMirrorAxisConfig();

    KisMirrorAxisConfig(const KisMirrorAxisConfig &rhs);
    KisMirrorAxisConfig& operator=(const KisMirrorAxisConfig& rhs);
    bool operator==(const KisMirrorAxisConfig& rhs) const;

    bool mirrorHorizontal();
    void setMirrorHorizontal(bool state);

    bool mirrorVertical();
    void setMirrorVertical(bool state);

    bool lockHorizontal();
    void setLockHorizontal(bool state);

    bool lockVertical();
    void setLockVertical(bool state);

    bool hideVerticalDecoration();
    void setHideVerticalDecoration(bool state);

    bool hideHorizontalDecoration();
    void setHideHorizontalDecoration(bool state);

    float handleSize();
    void setHandleSize(float size);

    float horizontalHandlePosition();
    void setHorizontalHandlePosition(float position);

    float verticalHandlePosition();
    void setVerticalHandlePosition(float position);

    QPointF axisPosition();
    void setAxisPosition(QPointF position);

    /**
     * @brief saveToXml() function for KisKraSaver
     * @param doc
     * @param tag
     * @return
     */
    QDomElement saveToXml(QDomDocument& doc, const QString &tag) const;

    /**
     * @brief loadFromXml() function for KisKraLoader
     * @param parent element
     * @return
     */
    bool loadFromXml(const QDomElement &parent);

    /**
     * @brief Check whether the config object was changed, or is the class default.
     * @return true, if the object is default; false, if the config was changed
     */
    bool isDefault() const;

private:
    class Private;
    const QScopedPointer<Private> d;
};

#endif // KISMIRRORAXISCONFIG_H
