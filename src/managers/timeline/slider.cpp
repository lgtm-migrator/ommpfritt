#include "managers/timeline/slider.h"
#include "logging.h"
#include <QMouseEvent>
#include <QPainter>
#include <cmath>
#include "animation/animator.h"

namespace omm
{

Slider::Slider(Animator& animator) : animator(animator), m_min(1), m_max(100)
{
  connect(&animator, SIGNAL(start_changed(int)), this, SLOT(update()));
  connect(&animator, SIGNAL(end_changed(int)), this, SLOT(update()));
  connect(&animator, SIGNAL(current_changed(int)), this, SLOT(update()));
}

void Slider::set_min(double frame)
{
  m_min = frame;
  update();
}

void Slider::set_max(double frame)
{
  m_max = frame;
  update();
}

void Slider::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);
  const int left = frame_to_pixel(animator.start()) - pixel_per_frame()/2.0;
  const int right = frame_to_pixel(animator.end()) + pixel_per_frame()/2.0;
  if (left > 0) {
    painter.fillRect(QRect(QPoint(0, 0), QPoint(left-1, height())), Qt::gray);
  }
  if (right < width()) {
    painter.fillRect(QRect(QPoint(right+1, 0), QPoint(width(), height())), Qt::gray);
  }
  if (right > 0 && left < width()) {
    painter.fillRect(QRect(QPoint(left, 0), QPoint(right, height())), Qt::white);
  }

  painter.save();
  draw_lines(painter);
  painter.restore();

  painter.save();
  painter.translate(frame_to_pixel(animator.current()), 0);
  draw_current(painter);
  painter.restore();

  QWidget::paintEvent(event);
}

void Slider::mousePressEvent(QMouseEvent *event)
{
  m_mouse_down_pos = event->pos();
  m_last_mouse_pos = event->pos();
  m_pan_active = event->modifiers() & Qt::AltModifier && event->button() == Qt::LeftButton;
  m_zoom_active = event->modifiers() & Qt::AltModifier && event->button() == Qt::RightButton;
  if (!m_pan_active && !m_zoom_active) {
    Q_EMIT value_changed(pixel_to_frame(event->pos().x()) + 0.5);
  }
  QWidget::mousePressEvent(event);
}

void Slider::mouseMoveEvent(QMouseEvent *event)
{
  static constexpr double min_ppf = 0.5;
  static constexpr double max_ppf = 70;
  if (m_pan_active) {
    const QPoint d = m_last_mouse_pos - event->pos();
    QCursor::setPos(mapToGlobal(m_mouse_down_pos));
    const double min = pixel_to_frame(frame_to_pixel(m_min) + d.x());
    const double max = pixel_to_frame(frame_to_pixel(m_max) + d.x());
    m_min = min;
    m_max = max;
    update();
  } else if (m_zoom_active) {
    const QPoint d = m_last_mouse_pos - event->pos();
    QCursor::setPos(mapToGlobal(m_mouse_down_pos));
    const double center_frame = pixel_to_frame(m_mouse_down_pos.x());
    const int left = m_mouse_down_pos.x();
    const int right = width() - m_mouse_down_pos.x();
    double ppf = pixel_per_frame() * std::exp(-d.x() / 300.0);
    ppf = std::clamp(ppf, min_ppf, max_ppf);
    m_min = center_frame - left / ppf;
    m_max = center_frame + right  / ppf - 1;
    update();
  } else {
    Q_EMIT value_changed(pixel_to_frame(event->pos().x()) + 0.5);
  }
  QWidget::mouseMoveEvent(event);
}

void Slider::mouseReleaseEvent(QMouseEvent* event)
{
  m_pan_active = false;
  QWidget::mouseReleaseEvent(event);
}

double Slider::frame_to_pixel(double frame) const
{
  return (frame - m_min) * pixel_per_frame();
}

double Slider::pixel_to_frame(double pixel) const
{
  return pixel / pixel_per_frame() + m_min;
}

double Slider::pixel_per_frame() const
{
  return width() / static_cast<double>(m_max - m_min + 1);
}

void Slider::draw_lines(QPainter& painter) const
{
  const double pixel_per_frame = this->pixel_per_frame();
  QPen pen;
  pen.setColor(Qt::black);
  pen.setCosmetic(true);
  painter.setPen(pen);
  for (int frame = m_min; frame <= m_max + 1; ++frame) {
    QFont font;
    QFontMetrics fm(font);
    if (pixel_per_frame < 10 && (frame % 2 != 0)) {
      continue;
    } else if (pixel_per_frame < 2 && frame % 10 != 0) {
      continue;
    }
    pen.setWidthF(frame % 10 == 0 ? 2.0 : 1.0);
    painter.setPen(pen);
    const double x = frame_to_pixel(frame);
    const int line_start = frame % 2 == 0 ? 0 : height() / 2.0;
    const int line_end = height() - (frame %  10 == 0 ? fm.height() : 0);
    painter.drawLine(x, line_start, x, line_end);
    if (frame % 10 == 0) {
      const QString text = QString("%1").arg(frame);
      painter.drawText(QPointF(x - fm.horizontalAdvance(text)/2.0, height()), text);
    }
  }
}

void Slider::draw_current(QPainter& painter) const
{
  const double height = this->height();
  const double pixel_per_frame = this->pixel_per_frame();
  const QRectF current_rect(-pixel_per_frame/2.0, height/2.0, pixel_per_frame, height);
  painter.fillRect(current_rect, QColor(255, 128, 0, 60));
  QPen pen;
  pen.setColor(QColor(255, 128, 0, 120));
  pen.setWidthF(4.0);
  painter.setPen(pen);
  painter.drawRect(current_rect);
}


}  // namespace omm
