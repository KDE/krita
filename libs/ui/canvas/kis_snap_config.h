/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SNAP_CONFIG_H
#define __KIS_SNAP_CONFIG_H


class KisSnapConfig
{
public:
    KisSnapConfig(bool loadValues = true);
    ~KisSnapConfig();

    bool orthogonal() const {
        return m_orthogonal;
    }
    void setOrthogonal(bool value) {
        m_orthogonal = value;
    }

    bool node() const {
        return m_node;
    }
    void setNode(bool value) {
        m_node = value;
    }

    bool extension() const {
        return m_extension;
    }
    void setExtension(bool value) {
        m_extension = value;
    }

    bool intersection() const {
        return m_intersection;
    }
    void setIntersection(bool value) {
        m_intersection = value;
    }

    bool boundingBox() const {
        return m_boundingBox;
    }
    void setBoundingBox(bool value) {
        m_boundingBox = value;
    }

    bool imageBounds() const {
        return m_imageBounds;
    }
    void setImageBounds(bool value) {
        m_imageBounds = value;
    }

    bool imageCenter() const {
        return m_imageCenter;
    }
    void setImageCenter(bool value) {
        m_imageCenter = value;
    }

    bool toPixel() const {
        return m_toPixel;
    }
    void setToPixel(bool value) {
        m_toPixel = value;
    }

    void saveStaticData() const;
    void loadStaticData();

private:
    bool m_orthogonal;
    bool m_node;
    bool m_extension;
    bool m_intersection;
    bool m_boundingBox;
    bool m_imageBounds;
    bool m_imageCenter;
    bool m_toPixel;
};

#endif /* __KIS_SNAP_CONFIG_H */
