#pragma once

#include <QAbstractItemDelegate>
#include "scene/history/macro.h"
#include <memory>
#include "objects/object.h"
#include "commands/command.h"

namespace omm
{

class ObjectTreeView;
class Tag;
class ObjectTreeSelectionModel;
class Property;

class PropertyArea
{
public:
  PropertyArea(const QRectF& area, ObjectTreeView& view, const QString& property_key);
  bool draw_active = false;
  virtual void draw(QPainter& painter, const QModelIndex& index) = 0;
  const QRectF area;
  ObjectTreeView& view;
  Property& property(const QModelIndex& index) const;
  bool is_active = false;
  virtual std::unique_ptr<Command> make_command(const QModelIndex& index, bool update_cache) = 0;

private:
  const QString m_property_key;
};

class VisibilityPropertyArea : public PropertyArea
{
public:
  explicit VisibilityPropertyArea(ObjectTreeView& view, const QRectF& rect, const QString& key);
  void draw(QPainter &painter, const QModelIndex &index) override;

protected:
  std::unique_ptr<Command> make_command(const QModelIndex& index, bool update_cache) override;

private:
  Object::Visibility m_new_value;
};

class IsEnabledPropertyArea : public PropertyArea
{
public:
  explicit IsEnabledPropertyArea(ObjectTreeView& view);
  void draw(QPainter &painter, const QModelIndex &index) override;

protected:
  std::unique_ptr<Command> make_command(const QModelIndex& index, bool update_cache) override;

private:
  bool m_new_value;
};

class ObjectQuickAccessDelegate : public QAbstractItemDelegate
{
public:
  enum class ActiveItem { None, Activeness, Visibility };
  explicit ObjectQuickAccessDelegate(ObjectTreeView& view);
  void paint( QPainter *painter, const QStyleOptionViewItem &option,
              const QModelIndex &index ) const override;
  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  bool on_mouse_button_press(QMouseEvent& event);
  void on_mouse_move(QMouseEvent& event);
  void on_mouse_release(QMouseEvent& event);

private:
  ObjectTreeView& m_view;

  ActiveItem m_active_item = ActiveItem::None;
  int m_active_item_value;

  // a mouse click commits a command.
  // when the mousebutton is hold down and moved, that command is undone and a macro is started.
  // when the mousebutton is released again, the macro is ended.
  std::unique_ptr<Macro> m_macro;
  std::unique_ptr<Command> m_command_on_hold;

  std::list<std::unique_ptr<PropertyArea>> m_areas;
  QPointF to_local(const QPoint &view_global, const QModelIndex &index) const;
};

}  // namespace omm
