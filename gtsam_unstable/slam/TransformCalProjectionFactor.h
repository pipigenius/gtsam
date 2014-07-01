/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file TransformCalProjectionFactor.h
 * @brief Basic bearing factor from 2D measurement
 * @author Chris Beall
 * @author Richard Roberts
 * @author Frank Dellaert
 * @author Alex Cunningham
 */

#pragma once

#include <gtsam/nonlinear/NonlinearFactor.h>
#include <gtsam/geometry/SimpleCamera.h>
#include <boost/optional.hpp>

namespace gtsam {

  /**
   * Non-linear factor for a constraint derived from a 2D measurement. The calibration is known here.
   * i.e. the main building block for visual SLAM.
   * @addtogroup SLAM
   */
  template<class POSE, class LANDMARK, class CALIBRATION = Cal3_S2>
  class TransformCalProjectionFactor: public NoiseModelFactor4<POSE, POSE, LANDMARK, CALIBRATION> {
  protected:

    // Keep a copy of measurement and calibration for I/O
    Point2 measured_;                    ///< 2D measurement

    // verbosity handling for Cheirality Exceptions
    bool throwCheirality_; ///< If true, rethrows Cheirality exceptions (default: false)
    bool verboseCheirality_; ///< If true, prints text for Cheirality exceptions (default: false)

  public:

    /// shorthand for base class type
    typedef NoiseModelFactor4<POSE, POSE, LANDMARK, CALIBRATION> Base;

    /// shorthand for this class
    typedef TransformCalProjectionFactor<POSE, LANDMARK, CALIBRATION> This;

    /// shorthand for a smart pointer to a factor
    typedef boost::shared_ptr<This> shared_ptr;

    /// Default constructor
    TransformCalProjectionFactor() : throwCheirality_(false), verboseCheirality_(false) {}

    /**
     * Constructor
     * TODO: Mark argument order standard (keys, measurement, parameters)
     * @param measured is the 2 dimensional location of point in image (the measurement)
     * @param model is the standard deviation
     * @param poseKey is the index of the camera
     * @param pointKey is the index of the landmark
     * @param K shared pointer to the constant calibration
     */
    TransformCalProjectionFactor(const Point2& measured, const SharedNoiseModel& model,
        Key poseKey, Key transformKey,  Key pointKey, Key calibKey) :
          Base(model, poseKey, transformKey, pointKey, calibKey), measured_(measured),
          throwCheirality_(false), verboseCheirality_(false) {}

    /**
     * Constructor with exception-handling flags
     * TODO: Mark argument order standard (keys, measurement, parameters)
     * @param measured is the 2 dimensional location of point in image (the measurement)
     * @param model is the standard deviation
     * @param poseKey is the index of the camera
     * @param pointKey is the index of the landmark
     * @param K shared pointer to the constant calibration
     * @param throwCheirality determines whether Cheirality exceptions are rethrown
     * @param verboseCheirality determines whether exceptions are printed for Cheirality
     */
    TransformCalProjectionFactor(const Point2& measured, const SharedNoiseModel& model,
        Key poseKey, Key transformKey, Key pointKey, Key calibKey,
        bool throwCheirality, bool verboseCheirality) :
          Base(model, poseKey, transformKey, pointKey, calibKey), measured_(measured),
          throwCheirality_(throwCheirality), verboseCheirality_(verboseCheirality) {}

    /** Virtual destructor */
    virtual ~TransformCalProjectionFactor() {}

    /// @return a deep copy of this factor
    virtual gtsam::NonlinearFactor::shared_ptr clone() const {
      return boost::static_pointer_cast<gtsam::NonlinearFactor>(
          gtsam::NonlinearFactor::shared_ptr(new This(*this))); }

    /**
     * print
     * @param s optional string naming the factor
     * @param keyFormatter optional formatter useful for printing Symbols
     */
    void print(const std::string& s = "", const KeyFormatter& keyFormatter = DefaultKeyFormatter) const {
      std::cout << s << "TransformCalProjectionFactor, z = ";
      measured_.print();
      Base::print("", keyFormatter);
    }

    /// equals
    virtual bool equals(const NonlinearFactor& p, double tol = 1e-9) const {
      const This *e = dynamic_cast<const This*>(&p);
      return e
          && Base::equals(p, tol)
          && this->measured_.equals(e->measured_, tol);
    }

    /// Evaluate error h(x)-z and optionally derivatives
    Vector evaluateError(const Pose3& pose, const Pose3& transform, const Point3& point, const CALIBRATION& K,
        boost::optional<Matrix&> H1 = boost::none,
        boost::optional<Matrix&> H2 = boost::none,
        boost::optional<Matrix&> H3 = boost::none,
        boost::optional<Matrix&> H4 = boost::none) const {
      try {
          if(H1 || H2 || H3 || H4) {
            gtsam::Matrix H0, H02;
            PinholeCamera<CALIBRATION> camera(pose.compose(transform, H0, H02), K);
            Point2 reprojectionError(camera.project(point, H1, H3, H4) - measured_);
            *H2 = *H1 * H02;
            *H1 = *H1 * H0;
            return reprojectionError.vector();
          } else {
            PinholeCamera<CALIBRATION> camera(pose.compose(transform), K);
            Point2 reprojectionError(camera.project(point, H1, H3, H4) - measured_);
            return reprojectionError.vector();
          }
      } catch( CheiralityException& e) {
        if (H1) *H1 = zeros(2,6);
        if (H2) *H2 = zeros(2,6);
        if (H3) *H3 = zeros(2,3);
        if (H4) *H4 = zeros(2,CALIBRATION::Dim());
        if (verboseCheirality_)
          std::cout << e.what() << ": Landmark "<< DefaultKeyFormatter(this->key2()) <<
              " moved behind camera " << DefaultKeyFormatter(this->key1()) << std::endl;
        if (throwCheirality_)
          throw e;
      }
      return ones(2) * 2.0 * K.fx();
    }

    /** return the measurement */
    const Point2& measured() const {
      return measured_;
    }

    /** return verbosity */
    inline bool verboseCheirality() const { return verboseCheirality_; }

    /** return flag for throwing cheirality exceptions */
    inline bool throwCheirality() const { return throwCheirality_; }

  private:

    /// Serialization function
    friend class boost::serialization::access;
    template<class ARCHIVE>
    void serialize(ARCHIVE & ar, const unsigned int version) {
      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Base);
      ar & BOOST_SERIALIZATION_NVP(measured_);
      ar & BOOST_SERIALIZATION_NVP(throwCheirality_);
      ar & BOOST_SERIALIZATION_NVP(verboseCheirality_);
    }
  };
} // \ namespace gtsam
