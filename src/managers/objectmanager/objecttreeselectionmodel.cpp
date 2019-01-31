#include "managers/objectmanager/objecttreeselectionmodel.h"
#include "scene/objecttreeadapter.h"
#include "scene/scene.h"

namespace omm
{

ObjectTreeSelectionModel::ObjectTreeSelectionModel(ObjectTreeAdapter& adapter)
  : QItemSelectionModel(&adapter)
{
}

bool ObjectTreeSelectionModel::is_selected(Tag& tag) const
{
  return ::contains(m_selected_tags, &tag);
}

void ObjectTreeSelectionModel::select(Tag& tag, QItemSelectionModel::SelectionFlags command)
{
  if (command & QItemSelectionModel::Clear) { clear_selection(); }

  if (command & QItemSelectionModel::Select) {
    m_selected_tags.insert(&tag);
  } else if (command & QItemSelectionModel::Deselect) {
    m_selected_tags.erase(&tag);
  } else if (command & QItemSelectionModel::Toggle) {
    select(tag, is_selected(tag) ? QItemSelectionModel::Deselect : QItemSelectionModel::Select);
  }
}

void ObjectTreeSelectionModel::clear_selection()
{
  QItemSelectionModel::clearSelection();
  m_selected_tags.clear();
}

void ObjectTreeSelectionModel::select( const QModelIndex &index,
                                       QItemSelectionModel::SelectionFlags command)
{
  const bool is_tag_index = index.column() == omm::ObjectTreeAdapter::TAGS_COLUMN;
  if (command & QItemSelectionModel::Clear && !is_tag_index) {
    m_selected_tags.clear();
  }
  QItemSelectionModel::select(index, command);
}

void ObjectTreeSelectionModel::select( const QItemSelection &selection,
                                       QItemSelectionModel::SelectionFlags command)
{

  const auto has_tag_index = [](const QItemSelectionRange& range) {
    return range.left() <= ObjectTreeAdapter::TAGS_COLUMN
        && range.right() >= ObjectTreeAdapter::TAGS_COLUMN;
  };
  const bool selection_has_tags = std::any_of(selection.begin(), selection.end(), has_tag_index);

  if (command & QItemSelectionModel::Clear && !selection_has_tags) {
    m_selected_tags.clear();
  }
  QItemSelectionModel::select(selection, command);
}

std::set<Tag*> ObjectTreeSelectionModel::selected_tags() const { return m_selected_tags; }

std::vector<Tag*> ObjectTreeSelectionModel::selected_tags_ordered(Scene& scene) const
{
  std::list<Tag*> selected_tags;
  std::stack<Object*> stack;
  stack.push(&scene.object_tree.root());
  while (stack.size() > 0) {
    Object* object = stack.top();
    stack.pop();

    for (Tag* tag : object->tags.ordered_items()) {
      if (::contains(m_selected_tags, tag)) { selected_tags.push_back(tag); }
    }

    for (Object* child : object->children()) { stack.push(child); }
  }

  return std::vector(selected_tags.begin(), selected_tags.end());
}



}  // namespace omm
