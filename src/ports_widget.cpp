#include "ports_widget.hpp"
#include <QPainter>
#include <QMouseEvent>

PortsWidget::PortsWidget(QWidget *const parent)
  : QWidget(parent)
  , _state(PortsWidget::Normal)
  , _from(-1)
{
  setMouseTracking(true);
}

PortsWidget::~PortsWidget()
{
  
}

void PortsWidget::setPorts(const QStringList &ports)
{
  _ports = ports;
  update();
}

void PortsWidget::setRoles(const QStringList &roles)
{
  _roles = roles;
  update();
}

const QStringList &PortsWidget::ports() const
{
  return _ports;
}

const QStringList &PortsWidget::roles() const
{
  return _roles;
}

void PortsWidget::setMapping(const QMap<int, int> &map)
{
  _mapping = map;
  update();
}

QMap<int, int> PortsWidget::mapping() const
{
  return _mapping;
}

void PortsWidget::mousePressEvent(QMouseEvent *event)
{
  if(event->button() != Qt::LeftButton) return;
  
  bool consumed = false;
  // Ports
  {
    int hit = -1;
    for(int i = 0; i < _portRects.size(); ++i) {
      if(_portRects[i].contains(event->pos())) {
        hit = i;
        break;
      }
    }
    if(hit >= 0) {
      portMouseHit(hit);
      consumed = true;
    }
  }
  
  // Roles
  {
    int hit = -1;
    for(int i = 0; i < _roleRects.size(); ++i) {
      if(_roleRects[i].contains(event->pos())) {
        hit = i;
        break;
      }
    }
    if(hit >= 0) {
      roleMouseHit(hit);
      consumed = true;
    }
  }
  
  if(!consumed) {
    if(_state == PortsWidget::DraggingToRole) removePortConnection(_from);
    else if(_state == PortsWidget::DraggingToPort) removeRoleConnections(_from);
    _state = PortsWidget::Normal;
    _from = -1;
    _portStates.clear();
    _roleStates.clear();
  }
  
  updateHighlights(event->pos());
}
void PortsWidget::mouseMoveEvent(QMouseEvent *event) {
  updateHighlights(event->pos());
}

void PortsWidget::paintEvent(QPaintEvent *event)
{
  QPainter p(this);
  
  p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
  const int goldenAngle = 137;
  if(!_ports.isEmpty()) {
    QFont f = p.font();
    f.setPixelSize(qMax(height() / _ports.size() / 2, 12));
    p.setFont(f);
    
    const double spacing = (double)height() / _ports.size();
    int largestWidth = 0;
    for(int i = 0; i < _ports.size(); ++i) {
      const int yOffset = i * spacing;
      const int nextYOffset = (i + 1.0) * spacing;
      QRect bounding;
      p.drawText(5, yOffset, width(), nextYOffset - yOffset, Qt::AlignLeft | Qt::AlignVCenter, _ports[i], &bounding);
      if(largestWidth < bounding.width()) largestWidth = bounding.width();
    }
    p.save();
    
    _portRects.clear();
    for(int i = 0; i < _ports.size(); ++i) {
      const int yOffset = i * spacing;
      const int x1 = largestWidth + width() / 20;
      const int y1 = yOffset + spacing * 0.25;
      const int size = spacing * 0.5;
      const int hue = (i * goldenAngle) % 360;
      p.setPen(QPen(QColor::fromHsvF(hue / 360.0, 1.0, 0.5), 2));
      switch(_portStates.value(i, PortsWidget::None)) {
        case PortsWidget::None:
        p.setBrush(Qt::transparent);
        break;
        case PortsWidget::Hover:
        p.setBrush(p.pen().color().lighter());
        break;
        case PortsWidget::Active:
        p.setBrush(p.pen().color());
        break;
      }
      
      QRect bounding(x1, y1, size, size);
      p.drawEllipse(bounding);
      _portRects.append(bounding);
    }
    p.restore();
  }
  
  if(!_roles.isEmpty()) {
    const double spacing = (double)height() / _roles.size();
    int largestWidth = 0;
    for(int i = 0; i < _roles.size(); ++i) {
      const int yOffset = i * spacing;
      const int nextYOffset = (i + 1.0) * spacing;
      QRect bounding;
      p.drawText(0, yOffset, width() - 5, nextYOffset - yOffset, Qt::AlignRight | Qt::AlignVCenter, _roles[i], &bounding);
      if(largestWidth < bounding.width()) largestWidth = bounding.width();
    }
    p.save();
    _roleRects.clear();
    p.setPen(QPen(Qt::black, 2));
    for(int i = 0; i < _roles.size(); ++i) {
      const int yOffset = i * spacing;
      const int x1 = largestWidth + width() / 20;
      const int y1 = yOffset + spacing * 0.25;
      const int size = spacing * 0.5;
      switch(_roleStates.value(i, PortsWidget::None)) {
        case PortsWidget::None:
        p.setBrush(Qt::transparent);
        break;
        case PortsWidget::Hover:
        p.setBrush(p.pen().color().lighter());
        break;
        case PortsWidget::Active:
        p.setBrush(p.pen().color());
        break;
      }
      QRect bounding(width() - x1, y1, -size, size);
      p.drawEllipse(bounding);
      _roleRects.append(bounding);
    }
    p.restore();
  }
  
  {
    p.save();
    QList<int> keys = _mapping.keys();
    foreach(const int port, keys) {
      const int hue = (port * goldenAngle) % 360;
      QPen pen(QColor::fromHsvF(hue / 360.0, 1.0, 0.5), 4);
      pen.setCapStyle(Qt::RoundCap);
      p.setPen(pen);
      p.drawLine(_portRects[port].center(), _roleRects[_mapping[port]].center());
    }
    p.restore();
  }
}

void PortsWidget::portMouseHit(const int port)
{
  switch(_state) {
    case PortsWidget::Normal:
    _state = PortsWidget::DraggingToRole;
    _from = port;
    _portStates[port] = PortsWidget::Active;
    break;
    case PortsWidget::DraggingToPort:
    _state = PortsWidget::Normal;
    addConnection(port, _from);
    _from = -1;
    break;
    default: break;
  }
  
  update();
}

void PortsWidget::roleMouseHit(const int role)
{
  switch(_state) {
    case PortsWidget::Normal:
    _state = PortsWidget::DraggingToPort;
    _from = role;
    _roleStates[role] = PortsWidget::Active;
    break;
    case PortsWidget::DraggingToRole:
    _state = PortsWidget::Normal;
    addConnection(_from, role);
    _from = -1;
    break;
    default: break;
  }
  
  update();
}

void PortsWidget::updateHighlights(const QPoint &pos)
{
  // Ports
  for(int i = 0; i < _portRects.size(); ++i) {
    if(_portStates.value(i, PortsWidget::None) == PortsWidget::Active) continue;
    _portStates[i] = _portRects[i].contains(pos) ? PortsWidget::Hover : PortsWidget::None;
  }
  
  // Roles
  for(int i = 0; i < _roleRects.size(); ++i) {
    if(_roleStates.value(i, PortsWidget::None) == PortsWidget::Active) continue;
    _roleStates[i] = _roleRects[i].contains(pos) ? PortsWidget::Hover : PortsWidget::None;
  }
  
  update();
}

void PortsWidget::addConnection(const int port, const int role)
{
  _mapping[port] = role;
  _portStates.clear();
  _roleStates.clear();
}

void PortsWidget::removePortConnection(const int port)
{
  _mapping.remove(port);
  _portStates.clear();
  _roleStates.clear();
}

void PortsWidget::removeRoleConnections(const int role)
{
  QMap<int, int>::iterator it = _mapping.begin();
  for(; it != _mapping.end(); ++it) {
    if(it.value() == role) it = _mapping.erase(it);
  }
  _portStates.clear();
  _roleStates.clear();
}

