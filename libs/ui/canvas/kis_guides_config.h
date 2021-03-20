/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006 Laurent Montel <montel@kde.org>
   SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
   SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOGUIDESDATA_H
#define KOGUIDESDATA_H

#include "kritaui_export.h"
#include <QScopedPointer>
#include <QList>
#include <boost/operators.hpp>
#include <KoUnit.h>

class QDomElement;
class QDomDocument;
class QColor;
class QPen;


class KRITAUI_EXPORT KisGuidesConfig : boost::equality_comparable<KisGuidesConfig>
{
public:
    enum LineTypeInternal {
        LINE_SOLID = 0,
        LINE_DASHED,
        LINE_DOTTED
    };

public:
    KisGuidesConfig();
    ~KisGuidesConfig();

    KisGuidesConfig(const KisGuidesConfig &rhs);
    KisGuidesConfig& operator=(const KisGuidesConfig &rhs);
    bool operator==(const KisGuidesConfig &rhs) const;
    bool hasSamePositionAs(const KisGuidesConfig &rhs) const;

    /**
     * @brief Set the positions of the horizontal guide lines
     *
     * @param lines a list of positions of the horizontal guide lines
     */
    void setHorizontalGuideLines(const QList<qreal> &lines);

    /**
     * @brief Set the positions of the vertical guide lines
     *
     * @param lines a list of positions of the vertical guide lines
     */
    void setVerticalGuideLines(const QList<qreal> &lines);

    /**
     * @brief Add a guide line to the canvas.
     *
     * @param orientation the orientation of the guide line
     * @param position the position in document coordinates of the guide line
     */
    void addGuideLine(Qt::Orientation orientation, qreal position);

    /**
     * @brief Display or not guide lines
     */
    bool showGuideLines() const;

    /**
     * @param show display or not guide line
     */
    void setShowGuideLines(bool show);

    bool showGuides() const;
    void setShowGuides(bool value);
    bool lockGuides() const;
    void setLockGuides(bool value);
    bool snapToGuides() const;
    void setSnapToGuides(bool value);

    bool rulersMultiple2() const;
    void setRulersMultiple2(bool value);

    KoUnit::Type unitType() const;
    void setUnitType(KoUnit::Type type);

    LineTypeInternal guidesLineType() const;
    void setGuidesLineType(LineTypeInternal value);

    QColor guidesColor() const;
    void setGuidesColor(const QColor &value);

    QPen guidesPen() const;

    /// Returns the list of horizontal guide lines.
    const QList<qreal>& horizontalGuideLines() const;

    /// Returns the list of vertical guide lines.
    const QList<qreal>& verticalGuideLines() const;

    bool hasGuides() const;

    void loadStaticData();
    void saveStaticData() const;

    QDomElement saveToXml(QDomDocument& doc, const QString &tag) const;
    bool loadFromXml(const QDomElement &parent);

    bool isDefault() const;

    /// Transform the guides using the given \p transform. Please note that \p transform
    /// should be in 'document' coordinate system.
    /// Used with image-wide transformations.
    void transform(const QTransform &transform);

private:
    class Private;
    const QScopedPointer<Private> d;
};


#endif

