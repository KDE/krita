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

enum Uniform { ModelViewProjection, TextureMatrix, ViewportScale,
               TexelSize, Texture0, Texture1, FixedLodLevel };

class KisShaderProgram : public QOpenGLShaderProgram {
public:
    static std::map<Uniform, const char *> info;

    int location(Uniform uniform) {
        std::map<Uniform, int>::const_iterator it = locationMap.find(uniform);
        if (it != locationMap.end()) {
            return it->second;
        } else {
            int location = uniformLocation(info[uniform]);
            locationMap[uniform] = location;
            return location;
        }
    }
private:
    std::map<Uniform, int> locationMap;
};

class ShaderLoaderException : public std::runtime_error {
public:
    ShaderLoaderException(QString error) : std::runtime_error(error.toLatin1().data()) { }
};

class KisOpenGLShaderLoader {
public:
    KisOpenGLShaderLoader();
    KisShaderProgram *loadDisplayShader(KisDisplayFilter *displayFilter, bool useHiQualityFiltering);
    KisShaderProgram *loadCheckerShader();
    KisShaderProgram *loadCursorShader();

private:
    KisShaderProgram *loadShader(QString vertPath, QString fragPath, QByteArray vertHeader, QByteArray fragHeader);
};
