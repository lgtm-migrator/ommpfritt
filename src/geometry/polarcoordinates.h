#pragma once

#include "geometry/vec2.h"
#include <Qt>

namespace omm
{
struct PolarCoordinates {
  explicit PolarCoordinates(const double argument, const double magnitude);
  explicit PolarCoordinates(const Vec2f& cartesian);
  explicit PolarCoordinates();
  [[nodiscard]] Vec2f to_cartesian() const;
  void swap(PolarCoordinates& other);
  double argument;
  double magnitude;
  bool operator==(const PolarCoordinates& point) const;
  [[nodiscard]] bool has_nan() const;
  [[nodiscard]] bool has_inf() const;

  /**
   * @brief operator < enables the use of PolarCoordinates in, e.g., std::set
   */
  bool operator<(const PolarCoordinates& other) const;
  bool operator!=(const PolarCoordinates& point) const;
  PolarCoordinates operator-() const;

  /**
   * @brief normalize_angle normalizes an angle.
   * @param rad the angle
   * @return an number between [-pi, pi)
   */
  static double normalize_angle(double rad);
};

}  // namespace omm
