#ifndef _PORTS_WIDGET_HPP_
#define _PORTS_WIDGET_HPP_

#include <QWidget>
#include <QStringList>
#include <QMap>

class PortsWidget : public QWidget
{
Q_OBJECT
public:
  PortsWidget(QWidget *const parent = 0);
  ~PortsWidget();
  
  void setPorts(const QStringList &ports);
  void setRoles(const QStringList &roles);
  
  const QStringList &ports() const;
  const QStringList &roles() const;
  
  void setMapping(const QMap<int, int> &map);
  QMap<int, int> mapping() const;
  
protected:
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void paintEvent(QPaintEvent *event);
  
private:
  void portMouseHit(const int port);
  void roleMouseHit(const int role);
  void updateHighlights(const QPoint &pos);
  
  void addConnection(const int port, const int role);
  void removePortConnection(const int port);
  void removeRoleConnections(const int role);
  
  QStringList _ports;
  QStringList _roles;
  QMap<int, int> _mapping;
  
  QList<QRect> _portRects;
  QList<QRect> _roleRects;
  
  enum PortHighlightState {
    None,
    Hover,
    Active
  };
  
  QMap<int, PortHighlightState> _portStates;
  QMap<int, PortHighlightState> _roleStates;
  
  enum {
    Normal,
    DraggingToPort,
    DraggingToRole
  } _state;
  
  int _from;
};

#endif
