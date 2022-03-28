/*
 *  SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSPRAYRANDOMDISTRIBUTIONS_H
#define KISSPRAYRANDOMDISTRIBUTIONS_H

#include <QScopedPointer>

#include <kis_random_source.h>
#include <kis_cubic_curve.h>

/**
 * @brief Class that can generate randomly distributed values in the range
 *        [a..b] following an arbitrary pdf
 */
class KisSprayFunctionBasedDistribution
{
public:
    /**
     * @brief Construct an invalid KisSprayFunctionBasedDistribution
     */
    KisSprayFunctionBasedDistribution();

    /**
     * @brief Construct a new distribution
     * @tparam Function Type of the functor to sample from
     * @param numberOfSamples Number of points to sample from the function. They
     *                        are sampled evenly through the range [a..b].
     *                        The first point will be sampled at @p a and the
     *                        last one at @p b.
     * @param a The lower bound of the domain of the function. The sampling will
     *          start here
     * @param b The upper bound of the domain of the function. The sampling will
     *          start here
     * @param f The functor that will be used to get the samples
     */
    template <typename Function>
    KisSprayFunctionBasedDistribution(int numberOfSamples, double a, double b, Function f);

    ~KisSprayFunctionBasedDistribution();
    KisSprayFunctionBasedDistribution(const KisSprayFunctionBasedDistribution &other);
    KisSprayFunctionBasedDistribution& operator=(const KisSprayFunctionBasedDistribution &rhs);

    /**
     * @brief Get a random value between @ref min and @ref max that follows the distribution
     * @param rs The random source object that will be used to get a uniform value
     * @return A random value between @ref min and @ref max that follows the
     *         distribution
     */
    double operator()(KisRandomSourceSP rs) const;

    /**
     * @brief Return the minimum value that this distribution can produce
     */
    double min() const;

    /**
     * @brief Return the maximum value that this distribution can produce
     */
    double max() const;

    /**
     * @brief Return if this object is correctly initialiced and can be used to
     *        generate values
     */
    bool isValid() const;

protected:
    /**
     * @brief Function used to setup the distribution and put it in a valid
     *        state. See the constructor for the explanation of the parameters
     */
    template <typename Function>
    void initialize(size_t numberOfSamples, double a, double b, Function f);

private:
    class Private;
    QScopedPointer<Private> m_d;
};

/**
 * @brief Class that can generate uniformly distributed values in
 *        the [0..1) range
 */
class KisSprayUniformDistribution
{
public:
    /**
     * @brief Get a random value between @ref min and @ref max that follows a uniform distribution
     * @param rs The random source object that will be used to get a uniform value
     * @return A random value between @ref min and @ref max that follows the
     *         distribution
     */
    double operator()(KisRandomSourceSP rs) const;

    /**
     * @brief Return the minimum value that this distribution can produce
     */
    double min() const { return 0.0; }

    /**
     * @brief Return the maximum value that this distribution can produce
     */
    double max() const { return 1.0; }

    /**
     * @brief Return if this object is correctly initialiced and can be used to
     *        generate values
     */
    bool isValid() const { return true; }
};

/**
 * @brief Class that can generate uniformly distributed values in
 *        the [0..1) range, for polar coordinates distance
 */
class KisSprayUniformDistributionPolarDistance : public KisSprayUniformDistribution
{
public:
    double operator()(KisRandomSourceSP rs) const;
};

/**
 * @brief Class that can generate normally distributed values. For efficiency,
 *        the values will be in the range [0..standardDeviation*5]
 */
class KisSprayNormalDistribution : public KisSprayFunctionBasedDistribution
{
public:
    /**
     * @brief Construct an invalid KisSprayNormalDistribution
     */
    KisSprayNormalDistribution();

    /**
     * @brief Construct a new normal distribution
     * @param mean Where the "peak" of the distribution should lie or how much
     *             the distribution is shifted left or right
     * @param standardDeviation How spread should the distribution be
     */
    KisSprayNormalDistribution(double mean, double standardDeviation);
};

/**
 * @brief Class that can generate normally distributed values. For efficiency,
 *        the values will be in the range [0..standardDeviation*5], for polar
 *        coordinates distance
 * @see KisSprayNormalDistribution
 */
class KisSprayNormalDistributionPolarDistance : public KisSprayNormalDistribution
{
public:
    KisSprayNormalDistributionPolarDistance();
    KisSprayNormalDistributionPolarDistance(double mean, double standardDeviation);
};

/**
 * @brief Class that can generate randomly distributed values in the range
 *        [0..1] that follow a distribution that clusters the values
 *        towards 0 or 1
 */
class KisSprayClusterBasedDistribution : public KisSprayFunctionBasedDistribution
{
public:
    /**
     * @brief Construct an invalid KisSprayClusterBasedDistribution
     */
    KisSprayClusterBasedDistribution();

    /**
     * @brief Construct a new cluster based distribution
     * @param clusteringAmount A value in the range [-100..100] that indicates
     *                         how and how much the generated values should
     *                         cluster. A positive clustering amount will make
     *                         generated values cluster towards 0 whereas a
     *                         negative amount will cluster them towards 1. The
     *                         bigger the clustering amount, the more pronounced
     *                         will be the clustering, and the smaller it is,
     *                         the more evenly distributed the values will be
     */
    explicit KisSprayClusterBasedDistribution(double clusteringAmount);
};

/**
 * @brief Class that can generate randomly distributed values in the range
 *        [0..1] that follow a distribution that clusters the values
 *        towards 0 or 1, for polar coordinates distance
 * @see KisSprayClusterBasedDistribution
 */
class KisSprayClusterBasedDistributionPolarDistance : public KisSprayFunctionBasedDistribution
{
public:
    KisSprayClusterBasedDistributionPolarDistance();
    explicit KisSprayClusterBasedDistributionPolarDistance(double clusteringAmount);
};

/**
 * @brief Class that can generate randomly distributed values in the range
 *        [0..1] that follow a distribution given by a user defined cubic curve
 */
class KisSprayCurveBasedDistribution : public KisSprayFunctionBasedDistribution
{
public:
    /**
     * @brief Construct an invalid KisSprayCurveBasedDistribution
     */
    KisSprayCurveBasedDistribution();

    /**
     * @brief Construct a new curve based distribution
     * @param curve A cubic curve that will be used as pdf
     * @param repeat The number of times the given curve should repeat in the
     *               range [0..1] 
     */
    explicit KisSprayCurveBasedDistribution(const KisCubicCurve &curve, size_t repeat = 1);
};

/**
 * @brief Class that can generate randomly distributed values in the range
 *        [0..1] that follow a distribution given by a user defined cubic curve,
 *        for polar coordinates distance
 * @see KisSprayCurveBasedDistribution
 */
class KisSprayCurveBasedDistributionPolarDistance : public KisSprayFunctionBasedDistribution
{
public:
    KisSprayCurveBasedDistributionPolarDistance();
    explicit KisSprayCurveBasedDistributionPolarDistance(const KisCubicCurve &curve, size_t repeat = 1);
};

#endif
