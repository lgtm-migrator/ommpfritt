#pragma once

#include "external/json_fwd.hpp"
#include "geometry/boundingbox.h"
#include "geometry/matrix.h"
#include "geometry/point.h"
#include "geometry/vec2.h"
#include <2geom/pathvector.h>
#include <QTransform>
#include <Qt>

namespace omm
{
class ObjectTransformation
{
public:
  explicit ObjectTransformation();
  explicit ObjectTransformation(const Matrix& mat);
  explicit ObjectTransformation(const Vec2f& translation,
                                const Vec2f& scale,
                                const double rotation,
                                const double shear);

  void set_translation(const Vec2f& translation_vector);
  void translate(const Vec2f& translation_vector);
  ObjectTransformation translated(const Vec2f& translation_vector) const;
  Vec2f translation() const;

  void set_rotation(const double& angle);
  void rotate(const double& angle);
  ObjectTransformation rotated(const double& angle) const;
  double rotation() const;

  void set_scaling(const Vec2f& scale_vector);
  void scale(const Vec2f& scale_vector);
  ObjectTransformation scaled(const Vec2f& scale_vector) const;
  Vec2f scaling() const;

  void set_shearing(const double& shear);
  void shear(const double& shear);
  ObjectTransformation sheared(const double& shear) const;
  double shearing() const;

  ObjectTransformation inverted() const;
  Vec2f null() const;

  ObjectTransformation transformed(const ObjectTransformation& other) const;

  Matrix to_mat() const;
  void set_mat(const Matrix& mat);

  Vec2f apply_to_position(const Vec2f& position) const;
  Vec2f apply_to_direction(const Vec2f& direction) const;
  PolarCoordinates apply_to_position(const PolarCoordinates& point) const;
  PolarCoordinates apply_to_direction(const PolarCoordinates& point) const;
  BoundingBox apply(const BoundingBox& bb) const;
  ObjectTransformation apply(const ObjectTransformation& t) const;
  Point apply(const Point& point) const;
  ObjectTransformation normalized() const;
  bool contains_nan() const;
  bool is_identity() const;

  static constexpr auto TYPE = "ObjectTransformation";

  QTransform to_qtransform() const;

  bool has_nan() const;

  Geom::PathVector apply(const Geom::PathVector& pv) const;
  Geom::Path apply(const Geom::Path& path) const;
  std::unique_ptr<Geom::Curve> apply(const Geom::Curve& curve) const;
  operator Geom::Affine() const;

private:
  Vec2f m_translation;
  Vec2f m_scaling;
  double m_shearing;
  double m_rotation;

  static Geom::PathVector transform(const Geom::PathVector& pv, const Geom::Affine& affine);
  static Geom::Path transform(const Geom::Path& path, const Geom::Affine& affine);
  static std::unique_ptr<Geom::Curve> transform(const Geom::Curve& curve,
                                                const Geom::Affine& affine);
};

std::ostream& operator<<(std::ostream& ostream, const ObjectTransformation& t);
bool operator<(const ObjectTransformation& a, const ObjectTransformation& b);
bool operator==(const ObjectTransformation& a, const ObjectTransformation& b);
bool operator!=(const ObjectTransformation& a, const ObjectTransformation& b);

}  // namespace omm
