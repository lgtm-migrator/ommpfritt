#include "commands/cutpathcommand.h"
#include "objects/path.h"

namespace
{

void cut(const Geom::Curve& curve, std::list<double> cuts, Geom::Path& path)
{
  assert(std::is_sorted(cuts.begin(), cuts.end()));
  if (cuts.empty() || cuts.back() < 1.0) {
    cuts.push_back(1.0);
  }
  if (!cuts.empty() && cuts.front() == 0.0) {
    cuts.erase(cuts.begin());
  }

  double t0 = 0.0;
  for (double t : cuts) {
    path.append(curve.portion(t0, t));
    t0 = t;
  }
}

Geom::Path cut(const Geom::Path& path, std::list<Geom::PathTime> cuts)
{
  if (cuts.empty()) {
    // this is the most common case.
    return path;
  }

  assert(std::is_sorted(cuts.begin(), cuts.end()));
  auto current_cut = cuts.begin();
  Geom::Path cut_path;
  for (std::size_t i = 0; i < path.size(); ++i) {
    std::list<double> curve_cut_buffer;
    for (; current_cut != cuts.end() && current_cut->curve_index == i; ++current_cut) {
      curve_cut_buffer.push_back(current_cut->t);
    }
    cut(path[i], curve_cut_buffer, cut_path);
  }
  return cut_path;
}

Geom::PathVector cut(const Geom::PathVector& paths, std::vector<Geom::PathVectorTime> cuts)
{
  std::sort(cuts.begin(), cuts.end());
  std::vector<Geom::Path> cut_paths;
  cut_paths.reserve(paths.size());

  auto current_cut = cuts.begin();
  for (std::size_t i = 0; i < paths.size(); ++i) {
    std::list<Geom::PathTime> path_cut_buffer;
    for (; current_cut != cuts.end() && current_cut->path_index == i; ++current_cut) {
      path_cut_buffer.push_back(*current_cut);
    }
    cut_paths.push_back(cut(paths[i], path_cut_buffer));
  }

  return Geom::PathVector(cut_paths.begin(), cut_paths.end());
}

}  // namespace

namespace omm
{

CutPathCommand::CutPathCommand(Path& path, const std::vector<Geom::PathVectorTime>& cuts)
  : Command(QObject::tr("CutPathCommand"))
  , m_path(path)
  , m_original(path.geom_paths())
  , m_cut(cut(path.geom_paths(), cuts))
{
}

void CutPathCommand::undo()
{
  m_path.set(m_original);
}

void CutPathCommand::redo()
{
  m_path.set(m_cut);
}

}  // namespace omm
