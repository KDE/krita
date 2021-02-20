/*
 * SPDX-FileCopyrightText: 2016 Julian Thijssen <julianthijssen@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    KisShaderProgram *loadOverlayInvertedShader();

private:
    KisShaderProgram *loadShader(QString vertPath, QString fragPath, QByteArray vertHeader, QByteArray fragHeader);
};
