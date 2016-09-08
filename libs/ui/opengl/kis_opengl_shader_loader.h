#include "canvas/kis_display_filter.h"

#include <QOpenGLShaderProgram>
#include <QByteArray>
#include <QString>

#include <unordered_map>
#include <string>

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
    void reportShaderLinkFailedAndExit(bool result, const QString &context, const QString &log);
};
