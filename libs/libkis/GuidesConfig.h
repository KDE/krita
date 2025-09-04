/*
 *  SPDX-FileCopyrightText: 2024 Grum999 <grum999@grum.fr>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef LIBKIS_GUIDESCONFIG_H
#define LIBKIS_GUIDESCONFIG_H

#include <QObject>
#include "kis_guides_config.h"

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * The GuidesConfig class encapsulates a Krita Guides configuration.
 *
 */
class KRITALIBKIS_EXPORT GuidesConfig : public QObject
{
    Q_OBJECT

public:
    GuidesConfig(KisGuidesConfig *guidesConfig);

    /**
     * Create a new, empty GuidesConfig.
     */
    explicit GuidesConfig(QObject *parent = 0);
    ~GuidesConfig() override;

    bool operator==(const GuidesConfig &other) const;
    bool operator!=(const GuidesConfig &other) const;

public Q_SLOTS:

    /**
     * @brief Guides color
     * @return color applied for all guides
     */
    QColor color() const;

    /**
     * @brief Define guides color
     * @param color color to apply
     */
    void setColor(const QColor &color) const;

    /**
     * @brief Guides line type
     * @return line type applied for all guides
     * Can be:
     * - "solid"
     * - "dashed"
     * - "dot"
     */
    QString lineType() const;

    /**
     * @brief Define guides lines type
     * @param lineType line type to use for guides:
     * Can be:
     * - "solid"
     * - "dashed"
     * - "dot"
     */
    void setLineType(const QString &lineType);

    /**
     * @brief indicate if there's guides defined
     * @return True if at least one guide is defined, otherwise False
     */
    bool hasGuides() const;

    /**
     * @brief indicate if position from current guides configuration match positions from another guides configuration
     * @return True if positions are the same
     */
    bool hasSamePositionAs(const GuidesConfig &guideConfig) const;

    /**
     * @brief The horizontal guides.
     * @return a list of the horizontal positions of guides.
     */
    QList<qreal> horizontalGuides() const;

    /**
     * @brief Set the horizontal guides.
     * @param lines a list of the horizontal positions of guides to set
     */
    void setHorizontalGuides(const QList<qreal> &lines);

    /**
     * @brief The vertical guides.
     * @return a list of vertical positions of guides.
     */
    QList<qreal> verticalGuides() const;

    /**
     * @brief Set the vertical guides.
     * @param lines a list of the vertical positions of guides to set
     */
    void setVerticalGuides(const QList<qreal> &lines);

    /**
     * @brief Load guides definition from an XML document
     * @param xmlContent xml content provided as a string
     * @return True if xml content is valid and guides has been loaded, otherwise False
     */
    bool fromXml(const QString &xmlContent) const;

    /**
     * @brief Save guides definition as an XML document
     * @return A string with xml content
     */
    QString toXml() const;

    /**
     * @brief Remove all guides
     */
    void removeAllGuides();

    /**
     * @brief Returns guides visibility status.
     * @return True if guides are visible, otherwise False
     */
    bool visible() const;

    /**
     * @brief Set guides visibility status
     * @param value True to set guides visible, otherwise False
     */
    void setVisible(const bool value);

    /**
     * @brief Returns guide lock status
     * @return True if guides are locked, otherwise False
     */
    bool locked() const;

    /**
     * @brief Set guides lock status
     * @param value True to set guides locked, otherwise False
     */
    void setLocked(const bool value);

    /**
     * @brief Returns guide snap status
     * @return True if snap to guides is active, otherwise False
     */
    bool snap() const;

    /**
     * @brief Set guides snap status
     * @param value True to set snap to guides active, otherwise False
     */
    void setSnap(const bool value);

private:
    friend class Document;

    KisGuidesConfig guidesConfig() const;

private:
    struct Private;
    Private *d;

};

#endif // LIBKIS_GUIDESCONFIG_H
