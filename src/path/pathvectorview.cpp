#include "path/pathvectorview.h"
#include "path/dedge.h"
#include "path/edge.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include <QPainterPath>


namespace
{

std::size_t count_distinct_points(const omm::Edge& first, const omm::Edge& second)
{
  const auto* const a1 = first.a().get();
  const auto* const b1 = first.b().get();
  const auto* const a2 = second.a().get();
  const auto* const b2 = second.b().get();

  const auto n = std::set{a1, b1, a2, b2}.size();
  return n;
}

}  // namespace

namespace omm
{

PathVectorView::PathVectorView(std::deque<DEdge> edges) : m_edges(std::move(edges))
{
  assert(is_valid());
}

bool PathVectorView::is_valid() const
{
  static constexpr auto is_valid = [](const DEdge& de) { return de.edge == nullptr || !de.edge->is_valid(); };
  if (std::any_of(m_edges.begin(), m_edges.end(), is_valid)) {
    return false;
  }

  switch (m_edges.size()) {
  case 0:
    [[fallthrough]];
  case 1:
    return true;
  case 2:
    return count_distinct_points(*m_edges.front().edge, *m_edges.back().edge) <= 3;
  default:
    for (std::size_t i = 1; i < m_edges.size(); ++i) {
      const auto& current_edge = *m_edges.at(i).edge;
      const auto& previous_edge = *m_edges.at(i - 1).edge;
      const auto loop_count = static_cast<std::size_t>((current_edge.is_loop() ? 1 : 0)
                                                       + (previous_edge.is_loop() ? 1 : 0));
      if (count_distinct_points(current_edge, previous_edge) != 3 - loop_count) {
        return false;
      }
    }
    return true;
  }
}

bool PathVectorView::is_simply_closed() const
{
  switch (m_edges.size()) {
  case 0:
    return false;
  case 1:
    return m_edges.front().edge->a() == m_edges.front().edge->b();  // edge loops from point to itself
  case 2:
    // Both edges have the same points.
    // They can be part of different paths, hence any direction is possible.
    return count_distinct_points(*m_edges.front().edge, *m_edges.back().edge) == 2;
  default:
    // Assuming there are no intersections,
    // there must be only one common point between first and last edge to be closed.
    return count_distinct_points(*m_edges.front().edge, *m_edges.back().edge) == 3;
  }

  return !m_edges.empty() && m_edges.front().edge->a().get() == m_edges.back().edge->b().get();
}

const std::deque<DEdge>& PathVectorView::edges() const
{
  return m_edges;
}

QPainterPath PathVectorView::to_painter_path() const
{
  assert(is_valid());
  if (m_edges.empty()) {
    return {};
  }
  QPainterPath p;
  p.moveTo(m_edges.front().start_point().geometry().position().to_pointf());
  for (std::size_t i = 0; i < m_edges.size(); ++i) {
    Path::draw_segment(p, m_edges[i].start_point(), m_edges[i].end_point(), m_edges[i].edge->path());
  }

  return p;
}

bool PathVectorView::contains(const Vec2f& pos) const
{
  return to_painter_path().contains(pos.to_pointf());
}

QString PathVectorView::to_string() const
{
  static constexpr auto label = [](const auto& dedge) { return dedge.edge->label(); };
  return static_cast<QStringList>(util::transform<QList>(this->edges(), label)).join(", ");
}

std::vector<PathPoint*> PathVectorView::path_points() const
{
  if (m_edges.empty()) {
    return {};
  }

  std::vector<PathPoint*> ps;
  ps.reserve(m_edges.size() + 1);
  ps.emplace_back(&m_edges.front().start_point());
  for (std::size_t i = 0; i < m_edges.size(); ++i) {
    ps.emplace_back(&m_edges.at(i).end_point());
  }
  return ps;
}

QRectF PathVectorView::bounding_box() const
{
  static constexpr auto get_geometry = [](const auto* const pp) { return pp->geometry(); };
  return Point::bounding_box(util::transform<std::list>(path_points(), get_geometry));
}

void PathVectorView::normalize()
{
  if (is_simply_closed()) {
    const auto min_it = std::min_element(m_edges.begin(), m_edges.end());
    std::rotate(m_edges.begin(), min_it, m_edges.end());

    if (m_edges.size() >= 3 && m_edges.at(1) > m_edges.back()) {
      std::reverse(m_edges.begin(), m_edges.end());
      // Reversing has moved the smallest edge to the end, but it must be at front after
      // normalization .
      std::rotate(m_edges.begin(), std::prev(m_edges.end()), m_edges.end());
    }
  } else if (m_edges.size() >= 2 && m_edges.at(0) > m_edges.at(1)) {
    std::reverse(m_edges.begin(), m_edges.end());
  }
}

bool operator==(PathVectorView a, PathVectorView b)
{
  a.normalize();
  b.normalize();
  return a.edges() == b.edges();
}

bool operator<(PathVectorView a, PathVectorView b)
{
  a.normalize();
  b.normalize();
  return a.edges() < b.edges();
}

}  // namespace omm
