/*
 *  SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <cmath>

#include <kis_assert.h>

#include "KisSprayRandomDistributions.h"

class KisSprayFunctionBasedDistribution::Private
{
public:
    struct SampleInfo
    {
        double x;
        double cdfAtX;
        double oneOverCdfDy;
    };

    std::vector<SampleInfo> samples;

    template <typename Function>
    void initialize(size_t numberOfSamples, double a, double b, Function f)
    {
        KIS_SAFE_ASSERT_RECOVER_RETURN(numberOfSamples > 0);
        KIS_SAFE_ASSERT_RECOVER_RETURN(b > a);

        samples.clear();

        if (numberOfSamples < 3) {
            samples.push_back({a, 0.0, 0.0});
            samples.push_back({b, 1.0, 1.0});
            return;
        }

        // Create the CDF
        const double domainSize = b - a;
        const double intervalSize = domainSize / static_cast<double>(numberOfSamples - 1);
        double sum = 0.0;
        double lastX, lastY, lastSum;
        size_t effectiveNumberOfSamples = numberOfSamples;

        // Adjust the limits of the pdf if it has a 0 probability segment at
        // the start or at the end
        {
            for (size_t i = 0; i < numberOfSamples; ++i) {
                const double x = a + intervalSize * static_cast<double>(i);
                const double y = f(x);
                if (y > 0.0) {
                    if (i > 0) {
                        a += intervalSize * static_cast<double>(i - 1);
                        effectiveNumberOfSamples -= i - 1;
                    }
                    break;
                }
                if (i == numberOfSamples - 1) {
                    // The whole pdf must have 0 probability
                    return;
                }
            }

            for (size_t i = 0; i < numberOfSamples; ++i) {
                const double x = b - intervalSize * static_cast<double>(i);
                const double y = f(x);
                if (y > 0.0) {
                    if (i > 0) {
                        b -= intervalSize * static_cast<double>(i - 1);
                        effectiveNumberOfSamples -= i - 1;
                    }
                    break;
                }
            }
        }
        // Insert first point
        {
            samples.push_back({a, 0.0, 0.0});
            lastX = a;
            lastY = f(a);
            lastSum = 0.0;
        }
        // Insert the rest of the points
        {
            // This angle serves as a reference to see if the cdf curve is
            // roughly straight, and remove some points
            constexpr double maximumAngleDeviationToSkip{M_PI / 1000.0};
            double referenceAngle = 0.0;
            bool mustCheckAngle = false;
            int skippedPoints = 0;

            for (size_t i = 1; i < effectiveNumberOfSamples; ++i) {
                const double x = a + intervalSize * static_cast<double>(i);
                const double y = f(x);
                // Accumulate the area under the curve between the two points
                sum += (x - lastX) * (y + lastY) / 2.0;
                //
                if (y == 0.0) {
                    if (lastY == 0.0) {
                        lastX = x;
                        lastY = y;
                        lastSum = sum;
                        ++skippedPoints;
                        continue;
                    } else {
                        mustCheckAngle = false;
                    }
                } else {
                    if (lastY == 0.0) {
                        if (skippedPoints > 0) {
                            samples.push_back({lastX, lastSum, 0.0});
                        }
                        mustCheckAngle = false;
                    }
                }

                // Compute if the current point is nearly colinear to the last two points
                if (i > 1 && mustCheckAngle) {
                    const int nPoints = samples.size();
                    const double angle = std::atan2(sum - samples[nPoints - 2].cdfAtX, x - samples[nPoints - 2].x);
                    if (std::abs(angle - referenceAngle) <= maximumAngleDeviationToSkip) {
                        samples.back().x = x;
                        samples.back().cdfAtX = sum;
                        continue;
                    }
                }

                samples.push_back({x, sum, 0.0});
                referenceAngle = std::atan2(sum - lastSum, x - lastX);
                mustCheckAngle = true;
                skippedPoints = 0;

                lastX = x;
                lastY = y;
                lastSum = sum;
            }
        }
        // Scale the cdf to the [0..1] range and compute deltas
        {
            for (size_t i = 1; i < samples.size() - 1; ++i) {
                samples[i].cdfAtX /= sum;
                samples[i].oneOverCdfDy = 1.0 / (samples[i].cdfAtX - samples[i - 1].cdfAtX);
            }
            samples.back().cdfAtX = 1.0;
            samples.back().oneOverCdfDy = 1.0 / (1.0 - samples[samples.size() - 2].cdfAtX);
        }
    }

    double generate(double randomValue) const
    {
        // Find the first sample that has cdf greater than the passed value
        auto sampleIterator =
            std::upper_bound(samples.begin(), samples.end(), SampleInfo{0.0, randomValue, 0.0},
                [](const SampleInfo &a, const SampleInfo &b) -> bool {  return a.cdfAtX < b.cdfAtX; }
            );
        const double t = (randomValue - (sampleIterator - 1)->cdfAtX) * sampleIterator->oneOverCdfDy;
        return (sampleIterator - 1)->x + t * (sampleIterator->x - (sampleIterator - 1)->x);
    }
};

KisSprayFunctionBasedDistribution::KisSprayFunctionBasedDistribution()
    : m_d(new Private)
{}

template <typename Function>
KisSprayFunctionBasedDistribution::KisSprayFunctionBasedDistribution(int numberOfSamples, double a, double b, Function f)
    : m_d(new Private)
{
    m_d->initialize(numberOfSamples, a, b, f);
}

KisSprayFunctionBasedDistribution::~KisSprayFunctionBasedDistribution()
{}

KisSprayFunctionBasedDistribution::KisSprayFunctionBasedDistribution(const KisSprayFunctionBasedDistribution &other)
    : m_d(new Private)
{
    m_d->samples = other.m_d->samples;
}

KisSprayFunctionBasedDistribution& KisSprayFunctionBasedDistribution::KisSprayFunctionBasedDistribution::operator=(const KisSprayFunctionBasedDistribution &rhs)
{
    if (this != &rhs) {
        m_d->samples = rhs.m_d->samples;
    }
    return *this;
}

double KisSprayFunctionBasedDistribution::operator()(KisRandomSourceSP rs) const
{
    return m_d->generate(rs->generateNormalized());
}

double KisSprayFunctionBasedDistribution::min() const
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(isValid(), NAN);

    return m_d->samples.front().x;
}

double KisSprayFunctionBasedDistribution::max() const
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(isValid(), NAN);

    return m_d->samples.back().x;
}

bool KisSprayFunctionBasedDistribution::isValid() const
{
    return m_d->samples.size() > 1;
}

template <typename Function>
void KisSprayFunctionBasedDistribution::initialize(size_t numberOfSamples, double a, double b, Function f)
{
    m_d->initialize(numberOfSamples, a, b, f);
}

double KisSprayUniformDistribution::operator()(KisRandomSourceSP rs) const
{
    return rs->generateNormalized();
}

double KisSprayUniformDistributionPolarDistance::operator()(KisRandomSourceSP rs) const
{
    return std::sqrt(rs->generateNormalized());
}

KisSprayNormalDistribution::KisSprayNormalDistribution()
{}

KisSprayNormalDistribution::KisSprayNormalDistribution(double mean, double standardDeviation)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(standardDeviation > 0.0);

    const double m_c1 = 1.0 / (standardDeviation * std::sqrt(2.0 * M_PI));
    const double m_c2 = 2.0 * standardDeviation * standardDeviation;
    KisSprayFunctionBasedDistribution::initialize(1000, 0.0, standardDeviation * 5.0,
        [mean, m_c1, m_c2](double x)
        {
            const double shiftedX = x - mean;
            return m_c1 * std::exp(-(shiftedX * shiftedX / m_c2));
        }
    );
}

KisSprayNormalDistributionPolarDistance::KisSprayNormalDistributionPolarDistance()
{}

KisSprayNormalDistributionPolarDistance::KisSprayNormalDistributionPolarDistance(double mean, double standardDeviation)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(standardDeviation > 0.0);

    const double m_c1 = 1.0 / (standardDeviation * std::sqrt(2.0 * M_PI));
    const double m_c2 = 2.0 * standardDeviation * standardDeviation;
    KisSprayFunctionBasedDistribution::initialize(1000, 0.0, standardDeviation * 5.0,
        [mean, m_c1, m_c2](double x)
        {
            const double shiftedX = x - mean;
            return 2.0 * x * m_c1 * std::exp(-(shiftedX * shiftedX / m_c2));
        }
    );
}

KisSprayClusterBasedDistribution::KisSprayClusterBasedDistribution()
{}

KisSprayClusterBasedDistribution::KisSprayClusterBasedDistribution(double clusteringAmount)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(clusteringAmount >= -100.0 && clusteringAmount <= 100.0);

    if (clusteringAmount == 0.0) {
        // Uniform distribution basically
        KisSprayFunctionBasedDistribution::initialize(2, 0.0, 1.0,
            [](double)
            {
                return 1.0;
            }
        );
        return;
    }
    const double sigma = 1.0 / std::abs(clusteringAmount);
    const double m_c1 = 2.0 * sigma * sigma;
    if (clusteringAmount < 0.0) {
        KisSprayFunctionBasedDistribution::initialize(1000, 1.0 - std::min(1.0, sigma * 4.0), 1.0,
            [m_c1](double x)
            {
                const double xx = 1.0 - x;
                return std::exp(-(xx * xx / m_c1));
            }
        );
    } else {
        KisSprayFunctionBasedDistribution::initialize(1000, 0.0, std::min(1.0, sigma * 4.0),
            [m_c1](double x)
            {
                return std::exp(-(x * x / m_c1));
            }
        );
    }
}

KisSprayClusterBasedDistributionPolarDistance::KisSprayClusterBasedDistributionPolarDistance()
{}

KisSprayClusterBasedDistributionPolarDistance::KisSprayClusterBasedDistributionPolarDistance(double clusteringAmount)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(clusteringAmount >= -100.0 && clusteringAmount <= 100.0);

    if (clusteringAmount == 0.0) {
        // Uniform distribution basically
        KisSprayFunctionBasedDistribution::initialize(1000, 0.0, 1.0,
            [](double x)
            {
                return 2.0 * x;
            }
        );
        return;
    }
    const double sigma = 1.0 / std::abs(clusteringAmount);
    const double m_c1 = 2.0 * sigma * sigma;
    if (clusteringAmount < 0.0) {
        KisSprayFunctionBasedDistribution::initialize(1000, 1.0 - std::min(1.0, sigma * 4.0), 1.0,
            [m_c1](double x)
            {
                const double xx = 1.0 - x;
                return 2.0 * x * std::exp(-(xx * xx / m_c1));
            }
        );
    } else {
        KisSprayFunctionBasedDistribution::initialize(1000, 0.0, std::min(1.0, sigma * 4.0),
            [m_c1](double x)
            {
                return 2.0 * x * std::exp(-(x * x / m_c1));
            }
        );
    }
}

KisSprayCurveBasedDistribution::KisSprayCurveBasedDistribution()
{}

KisSprayCurveBasedDistribution::KisSprayCurveBasedDistribution(const KisCubicCurve &curve, size_t repeat)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(repeat > 0);

    KisSprayFunctionBasedDistribution::initialize(((curve.points().size() % 4) + 1) * 1000 * repeat, 0.0, 1.0,
        [curve, repeat](double x)
        { 
            const double sx = x * static_cast<double>(repeat);
            return curve.value(sx - std::floor(sx));
        }
    );
}

KisSprayCurveBasedDistributionPolarDistance::KisSprayCurveBasedDistributionPolarDistance()
{}

KisSprayCurveBasedDistributionPolarDistance::KisSprayCurveBasedDistributionPolarDistance(const KisCubicCurve &curve, size_t repeat)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(repeat > 0);

    KisSprayFunctionBasedDistribution::initialize(((curve.points().size() % 4) + 1) * 1000 * repeat, 0.0, 1.0,
        [curve, repeat](double x)
        { 
            const double sx = x * static_cast<double>(repeat);
            return 2.0 * x * curve.value(sx - std::floor(sx));
        }
    );
}
