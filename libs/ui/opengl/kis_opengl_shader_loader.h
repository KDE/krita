/*
 * Copyright (C) Julian Thijssen <julianthijssen@gmail.com>, (C) 2016
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

#include "canvas/kis_display_filter.h"

#include <QOpenGLShaderProgram>
#include <QByteArray>
#include <QString>

#include <unordered_map>
#include <string>
#include <stdexcept>

#include <map>

/**
 * An enum for storing all uniform names used in shaders
 */
enum Uniform { ModelViewProjection, TextureMatrix, ViewportScale,
               TexelSize, Texture0, Texture1, FixedLodLevel, FragmentColor };

/**
 * A wrapper class over Qt's QOpenGLShaderProgram to
 * provide access to uniform locations without
 * having to store them as constants next to the shader.
 */
class KisShaderProgram : public QOpenGLShaderProgram {
public:
    /**
     * Stores the mapping of uniform enums to actual shader uniform names.
     * The actual shader names are necessary for calls to uniformLocation.
     */
    static std::map<Uniform, const char *> names;

    /**
     * Stores uniform location in cache if it is called for the first time
     * and retrieves the location from the map on subsequent calls.
     */
    int location(Uniform uniform) {
        std::map<Uniform, int>::const_iterator it = locationMap.find(uniform);
        if (it != locationMap.end()) {
            return it->second;
        } else {
            int location = uniformLocation(names[uniform]);
            locationMap[uniform] = location;
            return location;
        }
    }
private:
    std::map<Uniform, int> locationMap;
};

/**
 * A wrapper class over C++ Runtime Error, specifically to record
 * failures in shader compilation. Only thrown in KisOpenGLShaderLoader.
 */
class ShaderLoaderException : public std::runtime_error {
public:
    ShaderLoaderException(QString error) : std::runtime_error(error.toStdString()) { }
};

/**
 * A utility class for loading various shaders we use in Krita. It provides
 * specific methods for shaders that pick the correct vertex and fragment files
 * depending on the availability of OpenGL3. Additionally, it provides a generic
 * shader loading method to prevent duplication.
 */
class KisOpenGLShaderLoader {
public:
    KisShaderProgram *loadDisplayShader(QSharedPointer<KisDisplayFilter> displayFilter, bool useHiQualityFiltering);
    KisShaderProgram *loadCheckerShader();
    KisShaderProgram *loadSolidColorShader();

private:
    KisShaderProgram *loadShader(QString vertPath, QString fragPath, QByteArray vertHeader, QByteArray fragHeader);
};
