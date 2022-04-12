#include "path/pathgeometry.h"
#include <QPainterPath>
#include "geometry/point.h"

namespace omm
{

PathGeometry::PathGeometry(std::vector<Point> points)
    : m_points(std::move(points))
{

}

std::vector<Vec2f> PathGeometry::compute_control_points(const Point& a, const Point& b, InterpolationMode interpolation)
{
  static constexpr double t = 1.0 / 3.0;
  switch (interpolation) {
  case InterpolationMode::Bezier:
    [[fallthrough]];
  case InterpolationMode::Smooth:
    return {a.position(),
            a.right_position(),
            b.left_position(),
            b.position()};
    break;
  case InterpolationMode::Linear:
    return {a.position(),
            ((1.0 - t) * a.position() + t * b.position()),
            ((1.0 - t) * b.position() + t * a.position()),
            b.position()};
    break;
  }
  Q_UNREACHABLE();
  return {};
}

QPainterPath omm::PathGeometry::to_painter_path() const
{
    if (m_points.empty()) {
      return {};
    }
    QPainterPath path;
    path.moveTo(m_points.front().position().to_pointf());
    for (auto it = m_points.begin(); next(it) != m_points.end(); ++it) {
      path.cubicTo(it->right_position().to_pointf(),
                   next(it)->left_position().to_pointf(),
                   next(it)->position().to_pointf());
    }
    return path;
}

Point PathGeometry::smoothen_point(std::size_t i) const
{
  Q_UNUSED(i)
  return {};  // TODO
}

}  // namespace omm
