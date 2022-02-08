#include "path/path.h"
#include "geometry/point.h"
#include "path/pathpoint.h"
#include "serializers/abstractserializer.h"
#include <2geom/pathvector.h>

namespace
{

using namespace omm;

auto copy(const std::deque<std::unique_ptr<PathPoint>>& vs, Path& path)
{
  std::decay_t<decltype(vs)> copy;
  for (auto&& v : vs) {
    copy.emplace_back(std::make_unique<PathPoint>(v->geometry(), path));
  }
  return copy;
}

auto to_path_points(std::vector<Point>&& points, Path& path)
{
  return util::transform<std::deque>(std::move(points), [&path](auto&& point) {
    return std::make_unique<PathPoint>(point, path);
  });
}

auto to_path_points(std::deque<Point>&& points, Path& path)
{
  return util::transform(std::move(points), [&path](auto&& point) {
    return std::make_unique<PathPoint>(point, path);
  });
}

}  // namespace

namespace omm
{

Path::Path(PathVector* path_vector)
  : m_path_vector(path_vector)
{
}

Path::Path(std::deque<Point>&& points, PathVector* path_vector)
  : m_points(to_path_points(std::move(points), *this))
  , m_path_vector(path_vector)
{
}

Path::Path(std::vector<Point>&& points, PathVector* path_vector)
  : m_points(to_path_points(std::move(points), *this))
  , m_path_vector(path_vector)
{
}

Path::Path(const Path& other, PathVector* path_vector)
  : m_points(copy(other.m_points, *this))
  , m_path_vector(path_vector)
{
}

Path::~Path() = default;

std::size_t Path::size() const
{
  return m_points.size();
}

PathPoint& Path::at(std::size_t i) const
{
  return *m_points.at(i);
}

bool Path::contains(const PathPoint& point) const
{
  return  std::any_of(m_points.begin(), m_points.end(), [&point](const auto& candidate) {
    return &point == candidate.get();
  });
}

std::size_t Path::find(const PathPoint& point) const
{
  const auto it = std::find_if(m_points.begin(), m_points.end(), [&point](const auto& candidate) {
    return &point == candidate.get();
  });
  if (it == m_points.end()) {
    throw std::out_of_range("No such point in path.");
  } else {
    return std::distance(m_points.begin(), it);
  }
}

PathPoint& Path::add_point(const Vec2f& pos)
{
  return *m_points.emplace_back(std::make_unique<PathPoint>(Point{pos}, *this));
}

void Path::make_linear() const
{
  for (const auto& point : m_points) {
    point->set_geometry(point->geometry().nibbed());
  }
}

void Path::set_interpolation(InterpolationMode interpolation) const
{
  switch (interpolation) {
  case InterpolationMode::Bezier:
    return;
  case InterpolationMode::Smooth:
    smoothen();
    return;
  case InterpolationMode::Linear:
    make_linear();
    return;
  }
  Q_UNREACHABLE();
}

std::vector<Vec2f> Path::compute_control_points(const Point& a, const Point& b, InterpolationMode interpolation)
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

PathVector* Path::path_vector() const
{
  return m_path_vector;
}

void Path::set_path_vector(PathVector* path_vector)
{
  m_path_vector = path_vector;
}

void Path::smoothen() const
{
  for (std::size_t i = 0; i < m_points.size(); ++i) {
    m_points[i]->set_geometry(smoothen_point(i));
  }
}

Point Path::smoothen_point(std::size_t i) const
{
  const std::size_t n = m_points.size();
  PathPoint* left = nullptr;
  PathPoint* right = nullptr;
  if (i == 0) {
    left = m_points.at(0).get();
    right = m_points.at(1).get();
  } else if (i == n - 1) {
    left = m_points.at(n - 2).get();
    right = m_points.at(n - 1).get();
  } else {
    left = m_points.at(i - 1).get();
    right = m_points.at(i + 1).get();
  }
  Point copy = m_points[i]->geometry();
  const Vec2f d = (left->geometry().position() - right->geometry().position()) / 6.0;
  copy.set_right_tangent(PolarCoordinates(-d));
  copy.set_left_tangent(PolarCoordinates(d));
  return copy;
}

std::deque<PathPoint*> Path::points() const
{
  return util::transform(m_points, [](const auto& pt) { return pt.get(); });
}

void Path::insert_points(std::size_t i, std::deque<std::unique_ptr<PathPoint>>&& points)
{
  m_points.insert(std::next(m_points.begin(), static_cast<std::ptrdiff_t>(i)),
                  std::make_move_iterator(points.begin()),
                  std::make_move_iterator(points.end()));
}

std::deque<std::unique_ptr<PathPoint> > Path::extract(std::size_t start, std::size_t size)
{
  std::deque<std::unique_ptr<PathPoint>> points(size);
  for (std::size_t i = 0; i < size; ++i) {
    std::swap(points[i], m_points[i + start]);
  }
  const auto begin = std::next(m_points.begin(), static_cast<std::ptrdiff_t>(start));
  const auto end = std::next(begin, static_cast<std::ptrdiff_t>(size));
  m_points.erase(begin, end);
  return points;
}

void Path::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  const auto points_ptr = make_pointer(root, POINTS_POINTER);
  serializer.start_array(m_points.size(), points_ptr);
  for (std::size_t i = 0; i < m_points.size(); ++i) {
    m_points.at(i)->geometry().serialize(serializer, make_pointer(points_ptr, i));
  }
  serializer.end_array();
}

void Path::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  const auto points_ptr = make_pointer(root, POINTS_POINTER);
  const auto size = deserializer.array_size(points_ptr);
  for (std::size_t i = 0; i < size; ++i) {
    Point geometry;
    geometry.deserialize(deserializer, make_pointer(points_ptr, i));
    m_points.emplace_back(std::make_unique<PathPoint>(geometry, *this));
  }
}

PathView::PathView(Path& path, std::size_t index, std::size_t size)
  : path(&path), index(index), size(size)
{
}

bool operator<(const PathView& a, const PathView& b)
{
  static constexpr auto as_tuple = [](const PathView& a) {
    return std::tuple{a.path, a.index};
  };
  return as_tuple(a) < as_tuple(b);
}

std::ostream& operator<<(std::ostream& ostream, const PathView& path_view)
{
  ostream << "Path[" << path_view.path << " " << path_view.index << " " << path_view.size << "]";
  return ostream;
}

}  // namespace omm
