#pragma once

#include <string>
#include <stack>

#include "geometry/objecttransformation.h"
#include "geometry/boundingbox.h"
#include "geometry/point.h"
#include "common.h"

class QFont;
class QTextOption;

namespace omm
{

class Style;
class Scene;
class Rectangle;

class AbstractRenderer
{
public:
  struct TextOptions
  {
    TextOptions( const QFont& font, const QTextOption& option,
                 const Style& style, const double width )
      : font(font), option(option), style(style), width(width) {}
    const QFont& font;
    const QTextOption& option;
    const Style& style;
    const double width;
  };

  enum class Category { None = 0x0, Objects = 0x1, Handles = 0x2, BoundingBox = 0x4,
                        All = Objects | Handles | BoundingBox };
  explicit AbstractRenderer(Scene& scene, Category filter);
  void render();
  const BoundingBox& bounding_box() const;

public:
  virtual void draw_spline( const std::vector<Point>& points, const Style& style,
                            bool closed = false ) = 0;
  virtual void draw_rectangle(const Rectangle& rect, const Style& style) = 0;
  virtual void push_transformation(const ObjectTransformation& transformation);
  virtual void pop_transformation();
  virtual ObjectTransformation current_transformation() const;
  virtual void draw_circle(const Vec2f& pos, const double radius, const Style& style) = 0;
  virtual void draw_image( const std::string& filename, const Vec2f& pos,
                           const Vec2f& size, const double opacity = 1.0) = 0;
  virtual void draw_image( const std::string& filename, const Vec2f& pos,
                           const double& width, const double opacity = 1.0) = 0;
  virtual void draw_text(const std::string& text, const TextOptions& options) = 0;
  virtual void toast(const Vec2f& pos, const std::string& text) = 0;
  Scene& scene;

  Category category_filter;

protected:
  bool is_active() const;

private:
  std::stack<ObjectTransformation> m_transformation_stack;

};

}  // namespace omm

template<> struct omm::EnableBitMaskOperators<omm::AbstractRenderer::Category> : std::true_type {};

