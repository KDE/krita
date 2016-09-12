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


class ShaderLoaderException : public std::runtime_error {
public:
    ShaderLoaderException(QString error) : std::runtime_error(error.toLatin1().data()) { }
};

class KisShaderProgram : public QOpenGLShaderProgram {
public:
    int location(const char* name) {
        std::unordered_map<std::string, int>::const_iterator it = locationMap.find(std::string(name));
        if (it != locationMap.end()) {
            return it->second;
        } else {
            int location = uniformLocation(name);
            locationMap[std::string(name)] = location;
            return location;
        }
    }
private:
    std::unordered_map<std::string, int> locationMap;
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
