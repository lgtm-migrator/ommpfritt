#pragma once

#include "common.h"
#include "geometry/vec2.h"
#include <set>

namespace omm
{

class Path;
class Point;
class Scene;

class PointSelection
{
public:
  PointSelection(Scene& scene);
  [[nodiscard]] std::set<Point> points(Space space) const;
  [[nodiscard]] Vec2f center(Space space) const;

private:
  Scene& m_scene;
};

}  // namespace omm
