#include <CalligraVersionWrapper.h>

#include <calligraversion.h>
#include <calligragitversion.h>

QString CalligraVersionWrapper::versionYear()
{
    return QLatin1Literal(CALLIGRA_YEAR);
}

QString CalligraVersionWrapper::versionString(bool checkGit)
{
    QString calligraVersion(CALLIGRA_VERSION_STRING);
    QString version = calligraVersion;

    if (checkGit) {
#ifdef CALLIGRA_GIT_SHA1_STRING
        QString gitVersion(CALLIGRA_GIT_SHA1_STRING);
        version = QString("%1 (git %2)").arg(calligraVersion).arg(gitVersion).toLatin1();
#endif
    }
    return version;
}

