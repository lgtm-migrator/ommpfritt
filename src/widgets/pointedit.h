#pragma once

#include "geometry/point.h"
#include "objects/path.h"
#include <QWidget>

class QPushButton;

namespace omm
{
class CoordinateEdit;
class Point;
enum class DisplayMode;

class PointEdit : public QWidget
{
  Q_OBJECT
public:
  PointEdit(Point& point, QWidget* parent = nullptr);
  void set_display_mode(const DisplayMode& display_mode);

private:
  QPushButton* m_mirror_from_left;
  QPushButton* m_mirror_from_right;
  QPushButton* m_coupled;
  QPushButton* m_vanish_left;
  QPushButton* m_vanish_right;
  CoordinateEdit* m_left_tangent_edit;
  CoordinateEdit* m_right_tangent_edit;
  CoordinateEdit* m_position_edit;
  Point& m_point;
  Path* m_path;

private Q_SLOTS:
  void mirror_from_right();
  void mirror_from_left();
  void set_left_maybe(const PolarCoordinates& old_right, const PolarCoordinates& new_right);
  void set_right_maybe(const PolarCoordinates& old_left, const PolarCoordinates& new_left);
  void update_point();
};

}  // namespace omm
