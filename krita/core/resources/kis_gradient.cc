/*
 *  kis_gradient.cc - part of Krayon
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *                2001 John Califf
 *                2004 Boudewijn Rempt <boud@valdyas.org>
 *                2004 Adrian Page <adrian@pagenet.plus.com>
 *                2004 Sven Langkamp <longamp@reallygood.de>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <cfloat>
#include <cmath>

#include <qimage.h>
#include <qtextstream.h>

#include <koColor.h>

#include <kdebug.h>

#include "kis_gradient.h"

#define PREVIEW_WIDTH 64
#define PREVIEW_HEIGHT 64

KisGradientSegment::RGBColorInterpolationStrategy *KisGradientSegment::RGBColorInterpolationStrategy::m_instance = 0;
KisGradientSegment::HSVCWColorInterpolationStrategy *KisGradientSegment::HSVCWColorInterpolationStrategy::m_instance = 0;
KisGradientSegment::HSVCCWColorInterpolationStrategy *KisGradientSegment::HSVCCWColorInterpolationStrategy::m_instance = 0;

KisGradientSegment::LinearInterpolationStrategy *KisGradientSegment::LinearInterpolationStrategy::m_instance = 0;
KisGradientSegment::CurvedInterpolationStrategy *KisGradientSegment::CurvedInterpolationStrategy::m_instance = 0;
KisGradientSegment::SineInterpolationStrategy *KisGradientSegment::SineInterpolationStrategy::m_instance = 0;
KisGradientSegment::SphereIncreasingInterpolationStrategy *KisGradientSegment::SphereIncreasingInterpolationStrategy::m_instance = 0;
KisGradientSegment::SphereDecreasingInterpolationStrategy *KisGradientSegment::SphereDecreasingInterpolationStrategy::m_instance = 0;

KisGradient::KisGradient(const QString& file) : super(file)
{
}

KisGradient::~KisGradient()
{
	for (uint i = 0; i < m_segments.count(); i++) {
		delete m_segments[i];
		m_segments[i] = 0;
	}
}

bool KisGradient::loadAsync()
{
	KIO::Job *job = KIO::get(filename(), false, false);

	connect(job, SIGNAL(data(KIO::Job*, const QByteArray&)), this, SLOT(ioData(KIO::Job*, const QByteArray&)));
	connect(job, SIGNAL(result(KIO::Job*)), SLOT(ioResult(KIO::Job*)));
	return true;
}

bool KisGradient::saveAsync()
{
	return false;
}

QImage KisGradient::img()
{
	return m_img;
}

void KisGradient::ioData(KIO::Job * /*job*/, const QByteArray& data)
{
	if (!data.isEmpty()) {
		Q_INT32 startPos = m_data.size();

		m_data.resize(m_data.size() + data.count());
		memcpy(&m_data[startPos], data.data(), data.count());
	}
}

void KisGradient::ioResult(KIO::Job * /*job*/)
{
	// Gimp gradient files are UTF-8 text files.
	QTextIStream fileContent(m_data);
	fileContent.setEncoding(QTextStream::UnicodeUTF8);

	QString header = fileContent.readLine();

	if (header != "GIMP Gradient") {
		emit ioFailed(this);
		return;
	}

	QString nameDefinition = fileContent.readLine();
	QString numSegmentsText;

	if (nameDefinition.startsWith("Name: ")) {
		QString nameText = nameDefinition.right(nameDefinition.length() - 6);
		setName(nameText);

		numSegmentsText = fileContent.readLine();
	}
	else {
		// Older format without name.

		numSegmentsText = nameDefinition;
	}

	//kdDebug() << "Loading gradient: " << name() << endl;

	int numSegments;
	bool ok;

	numSegments = numSegmentsText.toInt(&ok);

	if (!ok || numSegments < 1) {
		emit ioFailed(this);
		return;
	}

	//kdDebug() << "Number of segments = " << numSegments << endl;

	for (int i = 0; i < numSegments; i++) {

		QString segmentText = fileContent.readLine();
		QTextIStream segmentFields(&segmentText);

		double leftOffset;
		double middleOffset;
		double rightOffset;

		segmentFields >> leftOffset >> middleOffset >> rightOffset;

		double leftRed;
		double leftGreen;
		double leftBlue;
		double leftAlpha;

		segmentFields >> leftRed >> leftGreen >> leftBlue >> leftAlpha;

		double rightRed;
		double rightGreen;
		double rightBlue;
		double rightAlpha;

		segmentFields >> rightRed >> rightGreen >> rightBlue >> rightAlpha;

		int interpolationType;
		int colorInterpolationType;

		segmentFields >> interpolationType >> colorInterpolationType;

		KoColor leftRgb((int)(leftRed * 255 + 0.5), (int)(leftGreen * 255 + 0.5), (int)(leftBlue * 255 + 0.5));
		KoColor rightRgb((int)(rightRed * 255 + 0.5), (int)(rightGreen * 255 + 0.5), (int)(rightBlue * 255 + 0.5));

		Color leftColor(leftRgb.color(), leftAlpha);
		Color rightColor(rightRgb.color(), rightAlpha);

		KisGradientSegment *segment = new KisGradientSegment(interpolationType, colorInterpolationType, leftOffset, middleOffset, rightOffset, leftColor, rightColor);

		if ( !segment -> isValid() ) {
			emit ioFailed(this);
			delete segment;
			return;
		}

		m_segments.push_back(segment);
	}

	if (!m_segments.isEmpty()) {
		m_img = generatePreview(PREVIEW_WIDTH, PREVIEW_HEIGHT);
		setValid(true);
		emit loadComplete(this);
	}
	else {
		emit ioFailed(this);
	}
}

KisGradientSegment *KisGradient::segmentAt(double t) const
{
	if (t < DBL_EPSILON) {
		t = 0;
	}
	else
	if (t > 1 - DBL_EPSILON) {
		t = 1;
	}

	Q_ASSERT(m_segments.count() != 0);

	KisGradientSegment *segment = 0;

	for (uint i = 0; i < m_segments.count(); i++) {
		if (t > m_segments[i] -> startOffset() - DBL_EPSILON && t < m_segments[i] -> endOffset() + DBL_EPSILON) {
			segment = m_segments[i];
			break;
		}
	}

	return segment;
}

void KisGradient::colorAt(double t, QColor *color, QUANTUM *opacity) const
{
	const KisGradientSegment *segment = segmentAt(t);
	Q_ASSERT(segment != 0);

	if (segment) {
		Color col = segment -> colorAt(t);
		*color = col.color();
		*opacity = static_cast<QUANTUM>(col.alpha() * OPACITY_OPAQUE + 0.5);
	}
}

QImage KisGradient::generatePreview(int width, int height) const
{
	QImage img(width, height, 32);

	for (int y = 0; y < img.height(); y++) {
		for (int x = 0; x < img.width(); x++) {

			int backgroundRed = 128 + 63 * ((x / 4 + y / 4) % 2);
			int backgroundGreen = backgroundRed;
			int backgroundBlue = backgroundRed;

			QColor color;
			QUANTUM opacity;
			double t = static_cast<double>(x) / (img.width() - 1);

			colorAt(t,  &color, &opacity);

			double alpha = static_cast<double>(opacity) / OPACITY_OPAQUE;

			int red = static_cast<int>((1 - alpha) * backgroundRed + alpha * color.red() + 0.5);
			int green = static_cast<int>((1 - alpha) * backgroundGreen + alpha * color.green() + 0.5);
			int blue = static_cast<int>((1 - alpha) * backgroundBlue + alpha * color.blue() + 0.5);

			img.setPixel(x, y, qRgb(red, green, blue));
		}
	}

	return img;
}

KisGradientSegment::KisGradientSegment(int interpolationType, int colorInterpolationType, double startOffset, double middleOffset, double endOffset, const Color& startColor, const Color& endColor)
{
	m_interpolator = 0;

	switch (interpolationType) {
	case INTERP_LINEAR:
		m_interpolator = LinearInterpolationStrategy::instance();
		break;
	case INTERP_CURVED:
		m_interpolator = CurvedInterpolationStrategy::instance();
		break;
	case INTERP_SINE:
		m_interpolator = SineInterpolationStrategy::instance();
		break;
	case INTERP_SPHERE_INCREASING:
		m_interpolator = SphereIncreasingInterpolationStrategy::instance();
		break;
	case INTERP_SPHERE_DECREASING:
		m_interpolator = SphereDecreasingInterpolationStrategy::instance();
		break;
	}

	m_colorInterpolator = 0;

	switch (colorInterpolationType) {
	case COLOR_INTERP_RGB:
		m_colorInterpolator = RGBColorInterpolationStrategy::instance();
		break;
	case COLOR_INTERP_HSV_CCW:
		m_colorInterpolator = HSVCCWColorInterpolationStrategy::instance();
		break;
	case COLOR_INTERP_HSV_CW:
		m_colorInterpolator = HSVCWColorInterpolationStrategy::instance();
		break;
	}

	if (startOffset < DBL_EPSILON) {
		m_startOffset = 0;
	}
	else
	if (startOffset > 1 - DBL_EPSILON) {
		m_startOffset = 1;
	}
	else {
		m_startOffset = startOffset;
	}

	if (middleOffset < m_startOffset + DBL_EPSILON) {
		m_middleOffset = m_startOffset;
	}
	else
	if (middleOffset > 1 - DBL_EPSILON) {
		m_middleOffset = 1;
	}
	else {
		m_middleOffset = middleOffset;
	}

	if (endOffset < m_middleOffset + DBL_EPSILON) {
		m_endOffset = m_middleOffset;
	}
	else
	if (endOffset > 1 - DBL_EPSILON) {
		m_endOffset = 1;
	}
	else {
		m_endOffset = endOffset;
	}

	m_length = m_endOffset - m_startOffset;

	if (m_length < DBL_EPSILON) {
		m_middleT = 0.5;
	}
	else {
		m_middleT = (m_middleOffset - m_startOffset) / m_length;
	}

	m_startColor = startColor;
	m_endColor = endColor;
}

const Color& KisGradientSegment::startColor() const
{
	return m_startColor;
}

const Color& KisGradientSegment::endColor() const
{
	return m_endColor;
}

double KisGradientSegment::startOffset() const
{
	return m_startOffset;
}

double KisGradientSegment::middleOffset() const
{
	return m_middleOffset;
}

double KisGradientSegment::endOffset() const
{
	return m_endOffset;
}

void KisGradientSegment::setStartOffset(double t)
{
	m_startOffset = t;
	m_length = m_endOffset - m_startOffset;

	if (m_length < DBL_EPSILON) {
		m_middleT = 0.5;
	}
	else {
		m_middleT = (m_middleOffset - m_startOffset) / m_length;
	}
}
void KisGradientSegment::setMiddleOffset(double t)
{
	m_middleOffset = t;

	if (m_length < DBL_EPSILON) {
		m_middleT = 0.5;
	}
	else {
		m_middleT = (m_middleOffset - m_startOffset) / m_length;
	}
}

void KisGradientSegment::setEndOffset(double t)
{
	m_endOffset = t;
	m_length = m_endOffset - m_startOffset;

	if (m_length < DBL_EPSILON) {
		m_middleT = 0.5;
	}
	else {
		m_middleT = (m_middleOffset - m_startOffset) / m_length;
	}
}

int KisGradientSegment::interpolation() const
{
	return m_interpolator -> type();
}

void KisGradientSegment::setInterpolation(int interpolationType)
{
	switch (interpolationType) {
	case INTERP_LINEAR:
		m_interpolator = LinearInterpolationStrategy::instance();
		break;
	case INTERP_CURVED:
		m_interpolator = CurvedInterpolationStrategy::instance();
		break;
	case INTERP_SINE:
		m_interpolator = SineInterpolationStrategy::instance();
		break;
	case INTERP_SPHERE_INCREASING:
		m_interpolator = SphereIncreasingInterpolationStrategy::instance();
		break;
	case INTERP_SPHERE_DECREASING:
		m_interpolator = SphereDecreasingInterpolationStrategy::instance();
		break;
	}
}

int KisGradientSegment::colorInterpolation() const
{
	return m_colorInterpolator -> type();
}

void KisGradientSegment::setColorInterpolation(int colorInterpolationType)
{
	switch (colorInterpolationType) {
	case COLOR_INTERP_RGB:
		m_colorInterpolator = RGBColorInterpolationStrategy::instance();
		break;
	case COLOR_INTERP_HSV_CCW:
		m_colorInterpolator = HSVCCWColorInterpolationStrategy::instance();
		break;
	case COLOR_INTERP_HSV_CW:
		m_colorInterpolator = HSVCWColorInterpolationStrategy::instance();
		break;
	}
}

Color KisGradientSegment::colorAt(double t) const
{
	Q_ASSERT(t > m_startOffset - DBL_EPSILON && t < m_endOffset + DBL_EPSILON);

	double segmentT;

	if (m_length < DBL_EPSILON) {
		segmentT = 0.5;
	}
	else {
		segmentT = (t - m_startOffset) / m_length;
	}

	double colorT = m_interpolator -> valueAt(segmentT, m_middleT);

	Color color = m_colorInterpolator -> colorAt(colorT, m_startColor, m_endColor);

	return color;
}

bool KisGradientSegment::isValid() const
{
	if (m_interpolator == 0 || m_colorInterpolator ==0)
		return false;
	return true;
}

KisGradientSegment::RGBColorInterpolationStrategy *KisGradientSegment::RGBColorInterpolationStrategy::instance()
{
	if (m_instance == 0) {
		m_instance = new RGBColorInterpolationStrategy();
	}

	return m_instance;
}

Color KisGradientSegment::RGBColorInterpolationStrategy::colorAt(double t, Color start, Color end) const
{
	int red = static_cast<int>(start.color().red() + t * (end.color().red() - start.color().red()) + 0.5);
	int green = static_cast<int>(start.color().green() + t * (end.color().green() - start.color().green()) + 0.5);
	int blue = static_cast<int>(start.color().blue() + t * (end.color().blue() - start.color().blue()) + 0.5);
	double alpha = start.alpha() + t * (end.alpha() - start.alpha());

	return Color(QColor(red, green, blue), alpha);
}

KisGradientSegment::HSVCWColorInterpolationStrategy *KisGradientSegment::HSVCWColorInterpolationStrategy::instance()
{
	if (m_instance == 0) {
		m_instance = new HSVCWColorInterpolationStrategy();
	}

	return m_instance;
}

Color KisGradientSegment::HSVCWColorInterpolationStrategy::colorAt(double t, Color start, Color end) const
{
	KoColor sc = KoColor(start.color());
	KoColor ec = KoColor(end.color());
	
	int s = static_cast<int>(sc.S() + t * (ec.S() - sc.S()) + 0.5);
	int v = static_cast<int>(sc.V() + t * (ec.V() - sc.V()) + 0.5);
	int h;
	
	if (ec.H() < sc.H()) {
		h = static_cast<int>(ec.H() + (1 - t) * (sc.H() - ec.H()) + 0.5);
	}
	else {
		h = static_cast<int>(ec.H() + (1 - t) * (360 - ec.H() + sc.H()) + 0.5);
		
		if (h > 359) {
			h -= 360;
		}
	}
	
	double alpha = start.alpha() + t * (end.alpha() - start.alpha());

	return Color(KoColor(h, s, v, KoColor::csHSV).color(), alpha);
}

KisGradientSegment::HSVCCWColorInterpolationStrategy *KisGradientSegment::HSVCCWColorInterpolationStrategy::instance()
{
	if (m_instance == 0) {
		m_instance = new HSVCCWColorInterpolationStrategy();
	}

	return m_instance;
}

Color KisGradientSegment::HSVCCWColorInterpolationStrategy::colorAt(double t, Color start, Color end) const
{
	KoColor sc = KoColor(start.color());
	KoColor se = KoColor(end.color());

	int s = static_cast<int>(sc.S() + t * (se.S() - sc.S()) + 0.5);
	int v = static_cast<int>(sc.V() + t * (se.V() - sc.V()) + 0.5);
	int h;

	if (sc.H() < se.H()) {
		h = static_cast<int>(sc.H() + t * (se.H() - sc.H()) + 0.5);
	}
	else {
		h = static_cast<int>(sc.H() + t * (360 - sc.H() + se.H()) + 0.5);

		if (h > 359) {
			h -= 360;
		}
	}

	double alpha = start.alpha() + t * (end.alpha() - start.alpha());

	return Color(KoColor(h, s, v, KoColor::csHSV).color(), alpha);
}

KisGradientSegment::LinearInterpolationStrategy *KisGradientSegment::LinearInterpolationStrategy::instance()
{
	if (m_instance == 0) {
		m_instance = new LinearInterpolationStrategy();
	}

	return m_instance;
}

double KisGradientSegment::LinearInterpolationStrategy::calcValueAt(double t, double middle)
{
	Q_ASSERT(t > -DBL_EPSILON && t < 1 + DBL_EPSILON);
	Q_ASSERT(middle > -DBL_EPSILON && middle < 1 + DBL_EPSILON);

	double value = 0;

	if (t <= middle) {
		if (middle < DBL_EPSILON) {
			value = 0;
		}
		else {
			value = (t / middle) * 0.5;
		}
	}
	else {
		if (middle > 1 - DBL_EPSILON) {
			value = 1;
		}
		else {
			value = ((t - middle) / (1 - middle)) * 0.5 + 0.5;
		}
	}

	return value;
}

double KisGradientSegment::LinearInterpolationStrategy::valueAt(double t, double middle) const
{
	return calcValueAt(t, middle);
}

KisGradientSegment::CurvedInterpolationStrategy::CurvedInterpolationStrategy()
{
	m_logHalf = log(0.5);
}

KisGradientSegment::CurvedInterpolationStrategy *KisGradientSegment::CurvedInterpolationStrategy::instance()
{
	if (m_instance == 0) {
		m_instance = new CurvedInterpolationStrategy();
	}

	return m_instance;
}

double KisGradientSegment::CurvedInterpolationStrategy::valueAt(double t, double middle) const
{
	Q_ASSERT(t > -DBL_EPSILON && t < 1 + DBL_EPSILON);
	Q_ASSERT(middle > -DBL_EPSILON && middle < 1 + DBL_EPSILON);

	double value = 0;

	if (middle < DBL_EPSILON) {
		middle = DBL_EPSILON;
	}

	value = pow(t, m_logHalf / log(middle));

	return value;
}

KisGradientSegment::SineInterpolationStrategy *KisGradientSegment::SineInterpolationStrategy::instance()
{
	if (m_instance == 0) {
		m_instance = new SineInterpolationStrategy();
	}

	return m_instance;
}

double KisGradientSegment::SineInterpolationStrategy::valueAt(double t, double middle) const
{
	double lt = LinearInterpolationStrategy::calcValueAt(t, middle);
	double value = (sin(-M_PI_2 + M_PI * lt) + 1.0) / 2.0;

	return value;
}

KisGradientSegment::SphereIncreasingInterpolationStrategy *KisGradientSegment::SphereIncreasingInterpolationStrategy::instance()
{
	if (m_instance == 0) {
		m_instance = new SphereIncreasingInterpolationStrategy();
	}

	return m_instance;
}

double KisGradientSegment::SphereIncreasingInterpolationStrategy::valueAt(double t, double middle) const
{
	double lt = LinearInterpolationStrategy::calcValueAt(t, middle) - 1;
	double value = sqrt(1 - lt * lt);

	return value;
}

KisGradientSegment::SphereDecreasingInterpolationStrategy *KisGradientSegment::SphereDecreasingInterpolationStrategy::instance()
{
	if (m_instance == 0) {
		m_instance = new SphereDecreasingInterpolationStrategy();
	}

	return m_instance;
}

double KisGradientSegment::SphereDecreasingInterpolationStrategy::valueAt(double t, double middle) const
{
	double lt = LinearInterpolationStrategy::calcValueAt(t, middle);
	double value = 1 - sqrt(1 - lt * lt);

	return value;
}

#include "kis_gradient.moc"

