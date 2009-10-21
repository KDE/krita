/*
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
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

#include "kis_illuminant_profile.h"

#include <KoColorProfile.h>
#include <QtXml/QtXml>

#include <QDataStream>
#include <QFile>
#include <kis_debug.h>
#include <cmath>

double **allocateMatrix(int m, int n)
{
    double **matrix;
    matrix = new double*[m];
    for (int i = 0; i < m; i++)
        matrix[i] = new double[n];
    return matrix;
}

void freeMatrix(int m, double **matrix)
{
    for (int i = 0; i < m; i++)
        delete [] matrix[i];
    delete [] matrix;
}

void applyMatrix(int m, int n, double **T, double *b, double *d)
{
    for (int i = 0; i < m; i++) {
        d[i] = 0;
        for (int j = 0; j < n; j++)
            d[i] += T[i][j] * b[j];
    }
}

double polyval(int n, const double *P, double x)
{
    double y = 0;
    for (int i = 0; i < n; i++)
        y += P[n-1-i] * pow(x, i);
    return y;
}

double phi(double r)
{
    return 2.0*r / pow(1.0 - r, 2);
}

double psi(double r)
{
    return pow(1.0 - r, 2) / (2.0*r);
}

double invphi(double y)
{
    return (1.0 + y - sqrt(2.0 * y + 1.0)) / y;
}

//--------------------------
// Helper functions for saving and loading
void writeTransformations(QDomDocument & doc, QDomElement & transformations, double ** T, int numWavelengths)
{
    for (int i = 0; i < numWavelengths; i++) {
        for (int j = 0; j < 3; j++) {
            QDomElement transformation = doc.createElement("transformation");
            transformation.setAttribute("value", T[j][i]);
            transformations.appendChild(transformation);
        }
    }
}

void writePrimary(QDomDocument & doc, QDomElement & node, double * primary, int numWavelengths)
{
    for (int i = 0; i < numWavelengths; ++i) {
        QDomElement wavelength = doc.createElement("wavelength");
        wavelength.setAttribute("value", primary[i]);
        node.appendChild(wavelength);
    }
}

bool readPrimaries(double *wavelengths, QDomElement primary, int numWavelengths)
{
    QDomElement wvl = primary.firstChildElement("wavelength");
    for (int i = 0; i < numWavelengths; i++) {
        if (wvl.isNull()) {
            dbgFile << "Not enough wavelengths";
            return false;

        }
        QString v = wvl.attribute("value");
        if (v.isEmpty()) {
            dbgFile << "No wavelength value";
            return false;
        }
        bool ok = true;
        double d = v.toDouble(&ok);
        if (!ok) {
            dbgFile << "Could not convert" << v << " to double.";
            return false;
        }
        wavelengths[i] = d;
        wvl = wvl.nextSiblingElement("wavelength");
    }
    return true;
}

bool verifyCount(const QString & elementName, QDomElement & e, int num)
{
    // Check whether there are enough coeffs
    int count = 0;
    QDomElement c = e.firstChildElement(elementName);
    while (! c.isNull()) {
        ++count;
        c = c.nextSiblingElement(elementName);
    }
    return (count == num);

}
//--------------- end helper functions for loading/saving


KisIlluminantProfile::KisIlluminantProfile(const QString &fileName)
        : KoColorProfile(fileName),
        m_wl(-1), m_T(0), m_red(0), m_green(0), m_blue(0), m_refvec(0),
        coeffs(0), m_illuminant(""), m_valid(false)
{

}

KisIlluminantProfile::KisIlluminantProfile(const KisIlluminantProfile &copy)
        : KoColorProfile(copy.fileName()),
        m_wl(-1), m_T(0), m_red(0), m_green(0), m_blue(0), m_refvec(0),
        coeffs(0), m_illuminant(""), m_valid(false)
{
    if (copy.valid()) {
        m_valid = copy.m_valid;
        m_wl = copy.m_wl;
        m_illuminant = copy.m_illuminant;
        setName(copy.name());

        m_T = allocateMatrix(3, m_wl);
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < m_wl; j++)
                m_T[i][j] = copy.m_T[i][j];

        m_red   = new double[m_wl];
        m_green = new double[m_wl];
        m_blue  = new double[m_wl];
        for (int i = 0; i < m_wl; i++)
            m_red[i] = copy.m_red[i];
        for (int i = 0; i < m_wl; i++)
            m_green[i] = copy.m_green[i];
        for (int i = 0; i < m_wl; i++)
            m_blue[i] = copy.m_blue[i];

        nc = copy.nc;
        coeffs = new double[nc*m_wl];
        for (qint8 i = 0; i < nc*m_wl; i++)
            coeffs[i] = copy.coeffs[i];

        m_refvec = new double[m_wl];
    }
}

KisIlluminantProfile::~KisIlluminantProfile()
{
    reset();
}

bool KisIlluminantProfile::load()
{
    reset();

    QFile inFile(fileName());
    if (! inFile.open(QIODevice::ReadOnly)) {
        dbgFile << "Could not open file for reading; " << fileName();
        return false;
    }

    QDomDocument doc("illuminant");
    if (!doc.setContent(&inFile)) {
        dbgFile << "Not an XML file; " << fileName();
        inFile.close();
        return false;
    }

    QDomElement root = doc.documentElement();
    if (root.nodeName() != "illuminant" || root.attribute("version") != "1") {
        dbgFile << "Not an illuminant file or wrong version; " << fileName();
        return false;
    }

    m_wl = root.attribute("wavelengths").toInt();
    if (m_wl == 0) {
        dbgFile << "No wavelengths; " << fileName();
        return false;
    }
    m_illuminant = root.attribute("name");
    if (m_illuminant.isEmpty()) {
        dbgFile << "No name; " << fileName();
        return false;
    }
    m_T = allocateMatrix(3, m_wl);
    QDomElement transformations = root.firstChildElement("transformations");
    if (transformations.isNull()) {
        dbgFile << "No transformations element; " << fileName();
        return false;
    }
    if (!verifyCount("transformation", transformations, 3 * m_wl)) {
        dbgFile << "Wrong number of transformations; " << fileName();
        return false;
    }

    QDomElement transformation = transformations.firstChildElement("transformation");
    for (int i = 0; i < m_wl; i++) {
        for (int j = 0; j < 3; j++) {
            if (transformation.isNull()) {
                dbgFile << "Not enough transformations: " << fileName();
                return false;

            }
            QString v = transformation.attribute("value");
            if (v.isEmpty()) {
                dbgFile << "No value in transformation element " << fileName();
                return 0;
            }
            bool ok = true;
            double d = v.toDouble(&ok);
            if (!ok) {
                dbgFile << "Could not convert" << v << " to double: " << fileName();
                return 0;
            }
            m_T[j][i] = d;
            transformation = transformation.nextSiblingElement("transformation");
        }
    }

    QDomElement primaries = root.firstChildElement("primaries");
    if (primaries.isNull()) {
        dbgFile << "No primaries; " << fileName();
        return false;
    }

    m_red   = new double[m_wl];
    QDomElement red = primaries.firstChildElement("red");
    if (red.isNull()) {
        dbgFile << "No red; " << fileName();
        return false;
    }

    if (!verifyCount("wavelength", red, m_wl)) {
        dbgFile << "Wrong number of red wavelengths; " << m_wl << ": " << fileName();
        return false;
    }

    readPrimaries(m_red, red, m_wl);

    m_green = new double[m_wl];
    QDomElement green = primaries.firstChildElement("green");
    if (green.isNull()) {
        dbgFile << "No green; " << fileName();
        return false;
    }
    if (!verifyCount("wavelength", green, m_wl)) {
        dbgFile << "Wrong number of green wavelengths; " << fileName();
        return false;
    }
    readPrimaries(m_green, green, m_wl);

    m_blue  = new double[m_wl];
    QDomElement blue = primaries.firstChildElement("blue");
    if (blue.isNull()) {
        dbgFile << "No blue: " << fileName();
        return false;
    }
    if (!verifyCount("wavelength", blue, m_wl)) {
        dbgFile << "Wrong number of blue wavelengths; " << fileName();
        return false;
    }
    readPrimaries(m_blue, blue, m_wl);

    QDomElement X = root.firstChildElement("X");
    if (X.isNull()) {
        dbgFile << "No X: " << fileName();
        return false;
    }

    nc = X.attribute("nc").toInt();
    if (nc == 0) {
        dbgFile << "No number of coefficients " << fileName();
        return false;
    }

    if (!verifyCount("coefficient", X, m_wl * nc)) {
        dbgFile << "Got wrong number coefficients (" << nc * m_wl << "): " << fileName();
        return false;
    }

    coeffs = new double[nc*m_wl];


    QDomElement coefficient = X.firstChildElement("coefficient");
    for (int i = 0; i < nc * m_wl; ++i) {
        if (coefficient.isNull()) {
            dbgFile << "Got wrong number of coefficients";
            return false;
        }
        QString v = coefficient.attribute("value");
        if (v.isEmpty()) {
            dbgFile << "No coefficient value " << fileName();
            return false;
        }
        bool ok = true;
        double d = v.toDouble(&ok);
        if (!ok) {
            dbgFile << "Could not convert" << v << " to double: " << fileName();
            return false;
        }

        coeffs[i] = d;

        coefficient = coefficient.nextSiblingElement("coefficient");
    }

    // Initialize the reflectance vector and channel converter
    m_refvec = new double[m_wl];
    setName(QString("%1").arg(m_illuminant));
    m_valid = true;

    return true;
}

bool KisIlluminantProfile::save(const QString &fileName)
{
    if (!valid())
        return false;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    // Create a dom document & save it
    QDomDocument doc("illuminant");
    QDomElement root = doc.createElement("illuminant");
    doc.appendChild(root);

    root.setAttribute("version", 1);
    root.setAttribute("name", m_illuminant);
    root.setAttribute("wavelengths", m_wl);

    QDomElement transformations = doc.createElement("transformations");
    root.appendChild(transformations);
    writeTransformations(doc, transformations, m_T, m_wl);

    QDomElement primaries = doc.createElement("primaries");
    root.appendChild(primaries);

    QDomElement red = doc.createElement("red");
    primaries.appendChild(red);
    writePrimary(doc, red, m_red, m_wl);

    QDomElement green = doc.createElement("green");
    primaries.appendChild(green);
    writePrimary(doc, green, m_green, m_wl);

    QDomElement blue = doc.createElement("blue");
    primaries.appendChild(blue);
    writePrimary(doc, blue, m_blue, m_wl);

    QDomElement coefficients = doc.createElement("X");
    coefficients.setAttribute("nc", nc);
    root.appendChild(coefficients);

    for (int i = 0; i < m_wl * nc; ++i) {
        QDomElement coeff = doc.createElement("coefficient");
        coeff.setAttribute("value", coeffs[i]);
        coefficients.appendChild(coeff);
    }

    QTextStream(&file) << doc.toString();
    return true;
}

void KisIlluminantProfile::fromRgb(const double *rgbvec, double *ksvec) const
{
    // TODO: add cache!

    rgbToReflectance(rgbvec);
    reflectanceToKS(ksvec);
}

void KisIlluminantProfile::toRgb(const double *ksvec, double *rgbvec) const
{
    KSToReflectance(ksvec);
    reflectanceToRgb(rgbvec);
}

double KisIlluminantProfile::fgen(double R, int i) const
{
    return fabs(polyval(nc, coeffs + i*nc, R));
}

void KisIlluminantProfile::reflectanceToKS(double *ksvec) const
{
    for (int i = 0; i < m_wl; i++) {
        ksvec[2*i+0] = fgen(m_refvec[i], i) * pow(1 - m_refvec[i], 2);
        ksvec[2*i+1] = fgen(m_refvec[i], i) * 2.0 * m_refvec[i];
    }
}

void KisIlluminantProfile::KSToReflectance(const double *ksvec) const
{
    for (int i = 0; i < m_wl; i++) {
        if (ksvec[2*i+0] <= 0)
            m_refvec[i] = 1;
        else if (ksvec[2*i+1] <= 0)
            m_refvec[i] = 0;
        else
            m_refvec[i] = invphi(ksvec[2*i+1] / ksvec[2*i+0]);
    }
}

void KisIlluminantProfile::rgbToReflectance(const double *rgbvec) const
{
    // Each reflectance is a linear combination of three base colors.
    for (int i = 0; i < m_wl; i++) {
        m_refvec[i] = rgbvec[0] * m_red[i] + rgbvec[1] * m_green[i] + rgbvec[2] * m_blue[i];
    }
}

void KisIlluminantProfile::reflectanceToRgb(double *rgbvec) const
{
    // Avoid calculation of black and white
    double sum = 0;
    for (int i = 0; i < m_wl; i++)
        sum += m_refvec[i];
    if (sum <= 0) {
        rgbvec[0] = rgbvec[1] = rgbvec[2] = 0;
        return;
    }
    if (sum >= m_wl) {
        rgbvec[0] = rgbvec[1] = rgbvec[2] = 1;
        return;
    }

    applyMatrix(3, m_wl, m_T, m_refvec, rgbvec);
    for (int i = 0; i < 3; i++) {
        if (rgbvec[i] < 0) rgbvec[i] = 0;
        if (rgbvec[i] > 1) rgbvec[i] = 1;
    }
}

void KisIlluminantProfile::reset()
{
    if (m_T)
        freeMatrix(3, m_T);
    if (m_refvec)
        delete [] m_refvec;
    if (m_red)
        delete [] m_red;
    if (m_green)
        delete [] m_green;
    if (m_blue)
        delete [] m_blue;
    if (coeffs)
        delete [] coeffs;

    m_T = 0;
    m_refvec = m_red = m_green = m_blue = 0;

    m_illuminant = "";
    m_wl = -1;

    m_valid = false;
}
