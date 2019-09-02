#pragma once

#include <QObject>
#include <set>
#include "aspects/propertyowner.h"

namespace omm
{

class Object;
class Tag;
class Style;
class Tool;

class MessageBox : public QObject
{
  Q_OBJECT
public:
  explicit MessageBox();

Q_SIGNALS:
  /**
   * @brief appearance_changed is emitted when the appearance of a tool has changed.
   * This signal forwards to @code appearance_changed().
   */
  void appearance_changed(Tool&);

  /**
   * @brief appearance_changed is emitted when the appearance of an object has changed.
   *  Usually, this signal is emitted at the end of the Object::update method.
   * This signal is not emitted when only the transformation of an object changed.
   * @see transformation_changed(Object&)
   * This signal forwards to @code appearance_changed().
   * If the specified object has a parent, this signal will forward to that very parent's
   * child_appearance_change signal.
   */
  void appearance_changed(Object&);

  /**
   * @brief transformation_changed similar to appearance_changed, however, this signal is only
   *  emitted when the transformation of an object changed.
   * This signal forwards to @code appearance_changed().
   */
  void transformation_changed(Object&);

  /**
   * @brief appearance_changed is emitted when the appearance of the scene changed, i.e., if it
   *  needs to be redrawn.
   * Many signals of this class forward to this one.
   * This is the weakest signal, it is emitted very frequently.
   */
  void appearance_changed();

  /**
   * @brief object_inserted is emitted when an object was inserted into the parent object
   * This signal forwards to @code appearance_changed().
   */
  void object_inserted(Object& parent, Object& object);

  /**
   * @brief object_inserted is emitted when an object was removed from the parent object.
   * This signal forwards to @code appearance_changed().
   */
  void object_removed(Object& parent, Object& object);

  /**
   * @brief object_inserted is emitted when an object was moved from old_parent to new_parent.
   * This signal forwards to @code appearance_changed().
   */
  void object_moved(Object& old_parent, Object& new_parent, Object& object);

  /**
   * @brief object_inserted is emitted when a style was inserted into the scene.
   * This signal forwards to @code appearance_changed().
   */
  void style_inserted(Style&);

  /**
   * @brief object_inserted is emitted when a style was removed from the scene.
   * This signal forwards to @code appearance_changed().
   */
  void style_removed(Style&);

  /**
   * @brief style_moved is emitted when a style was moved.
   */
  void style_moved(Style&);

  /**
   * @brief object_inserted is emitted when a tag was attached to the object.
   * This signal forwards to @code appearance_changed().
   */
  void tag_inserted(Object&, Tag&);

  /**
   * @brief object_inserted is emitted when a tag was removed from the object.
   * This signal forwards to @code appearance_changed().
   */
  void tag_removed(Object&, Tag&);

  /**
   * @brief selection_changed is emitted when the object selection changed.
   */
  void selection_changed(const std::set<Object*>&);

  /**
   * @brief selection_changed is emitted when the style selection changed.
   */
  void selection_changed(const std::set<Style*>&);

  /**
   * @brief selection_changed is emitted when the tag selection changed.
   */
  void selection_changed(const std::set<Tag*>&);

  /**
   * @brief selection_changed is emitted when the tool selection changed.
   */
  void selection_changed(const std::set<Tool*>&);

  /**
   * @brief selection_changed is emitted when the selection changed.
   */
  void selection_changed(const std::set<AbstractPropertyOwner*>&);

  /**
   * @brief selection_changed is emitted when tag, style, object or tool selection changed.
   * The kind of selection is transferred via the second argument.
   */
  void selection_changed(const std::set<AbstractPropertyOwner*>&, AbstractPropertyOwner::Kind);

  /**
   * @brief filename_changed is emitted when the filename of the scene changes.
   * This includes changes of the pending-changes-indicator (usually an asterisk or nothing).
   */
  void filename_changed();

  /**
   * @brief scene_reseted is emitted when the scene was reset.
   */
  void scene_reseted();

  /**
   * @brief appearance_changed is emitted when the style appearance changed.
   */
  void appearance_changed(Style&);

  /**
   * @param owner the owner of the property
   * @param key the
   * @param property
   */

  /**
   * @brief property_value_changed is emitted when the value of the property has changed.
   * @param key the key of the property. The following invariant applies:
   *    `owner.property(key) == &p`
   */
  void property_value_changed(AbstractPropertyOwner& owner, const std::string& key, Property& p);
};

}  // namespace omm
