/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef LIBKIS_FILTER_H
#define LIBKIS_FILTER_H

#include <QObject>

#include "kritalibkis_export.h"
#include "libkis.h"
#include <kis_filter_configuration.h>

/**
 * Filter: represents a filter and its configuration. A filter is identified by
 * an internal name. The configuration for each filter is defined as an InfoObject:
 * a map of name and value pairs.
 *
 * Currently available filters are:
 *
 * 'autocontrast', 'blur', 'bottom edge detections', 'brightnesscontrast', 'burn', 'colorbalance', 'colortoalpha', 'colortransfer',
 * 'desaturate', 'dodge', 'emboss', 'emboss all directions', 'emboss horizontal and vertical', 'emboss horizontal only',
 * 'emboss laplascian', 'emboss vertical only', 'gaussian blur', 'gaussiannoisereducer', 'gradientmap', 'halftone', 'hsvadjustment',
 * 'indexcolors', 'invert', 'left edge detections', 'lens blur', 'levels', 'maximize', 'mean removal', 'minimize', 'motion blur',
 * 'noise', 'normalize', 'oilpaint', 'perchannel', 'phongbumpmap', 'pixelize', 'posterize', 'raindrops', 'randompick',
 * 'right edge detections', 'roundcorners', 'sharpen', 'smalltiles', 'sobel', 'threshold', 'top edge detections', 'unsharp',
 * 'wave', 'waveletnoisereducer']
 */
class KRITALIBKIS_EXPORT Filter : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Filter)

public:
    /**
     * @brief Filter: create an empty filter object. Until a name is set, the filter cannot
     * be applied.
     */
    explicit Filter();
    ~Filter() override;

    bool operator==(const Filter &other) const;
    bool operator!=(const Filter &other) const;

public Q_SLOTS:

    /**
     * @brief name the internal name of this filter.
     * @return the name.
     */
    QString name() const;

    /**
     * @brief setName set the filter's name to the given name.
     */
    void setName(const QString &name);

    /**
     * @return the configuration object for the filter
     */
    InfoObject* configuration() const;

    /**
     * @brief setConfiguration set the configuration object for the filter
     */
    void setConfiguration(InfoObject* value);

    /**
     * @brief Apply the filter to the given node.
     * @param node the node to apply the filter to
     * @param x
     * @param y
     * @param w
     * @param h describe the rectangle the filter should be apply.
     * This is always in image pixel coordinates and not relative to the x, y
     * of the node.
     * @return @c true if the filter was applied successfully, or
     * @c false if the filter could not be applied because the node is locked or
     * does not have an editable paint device.
     */
    bool apply(Node *node, int x, int y, int w, int h);

    /**
     * @brief startFilter starts the given filter on the given node.
     *
     * @param node the node to apply the filter to
     * @param x
     * @param y
     * @param w
     * @param h describe the rectangle the filter should be apply.
     * This is always in image pixel coordinates and not relative to the x, y
     * of the node.
     */
    bool startFilter(Node *node, int x, int y, int w, int h);

private:
    friend class FilterLayer;
    friend class FilterMask;

    struct Private;
    Private *const d;

    KisFilterConfigurationSP filterConfig();

};

#endif // LIBKIS_FILTER_H
