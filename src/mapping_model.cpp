#include "mapping_model.hpp"

class PortItem : public QStandardItem
{
public:
  PortItem(const int port, const int offset)
    : _port(port)
    , _offset(offset)
  {
    setText(QString::number(_port + _offset));
  }
  
  void setPort(const int port)
  {
    _port = port;
    setText(QString::number(_port + _offset));
  }
  
  int port() const
  {
    return _port;
  }
  
  template<typename T>
  static PortItem *portitem_cast(T *const t)
  {
    return dynamic_cast<PortItem *>(t);
  }
  
  template<typename T>
  static const PortItem *portitem_cast(const T *const t)
  {
    return dynamic_cast<const PortItem *>(t);
  }
  
private:
  int _port;
  int _offset;
};

class ValueItem : public QStandardItem
{
public:
  ValueItem()
  {
  }
  
  void setValue(const int value)
  {
    _value = value;
    setText(QString::number(_value));
  }
  
  int value() const
  {
    return _value;
  }
  
  template<typename T>
  static ValueItem *valueitem_cast(T *const t)
  {
    return dynamic_cast<ValueItem *>(t);
  }
  
  template<typename T>
  static const ValueItem *valueitem_cast(const T *const t)
  {
    return dynamic_cast<const ValueItem *>(t);
  }
  
private:
  int _value;
};

class RoleItem : public QStandardItem
{
public:
  RoleItem(const QString &role)
    : _role(role)
  {
    setText(_role);
  }
  
  void setRole(const QString &role)
  {
    _role = role;
    setText(_role);
  }
  
  const QString &role() const
  {
    return _role;
  }
  
  template<typename T>
  static RoleItem *roleitem_cast(T *const t)
  {
    return dynamic_cast<RoleItem *>(t);
  }
  
  template<typename T>
  static const RoleItem *roleitem_cast(const T *const t)
  {
    return dynamic_cast<const RoleItem *>(t);
  }
  
private:
  QString _role;
};

MappingModel::MappingModel(QObject *const parent)
  : QStandardItemModel(parent)
{
  setHorizontalHeaderItem(0, new QStandardItem(tr("Port")));
  setHorizontalHeaderItem(1, new QStandardItem(tr("Value")));
  setHorizontalHeaderItem(2, new QStandardItem(tr("Role")));
}

MappingModel::~MappingModel()
{
}

void MappingModel::setValue(const int port, const int value)
{
  for(int i = 0; i < rowCount(); ++i) {
    PortItem *const p = PortItem::portitem_cast(item(i, 0));
    if(!p || p->port() != port) continue;
    ValueItem *const val = ValueItem::valueitem_cast(item(i, 1));
    if(!val) continue;
    val->setValue(value);
  }
}

void MappingModel::setMapping(const QMap<int, int> &map, const QStringList &roles, const int offset)
{
  while(rowCount()) qDeleteAll(takeRow(0));

  _map = map;
  _roles = roles;
  _offset = offset;
  foreach(const int key, _map.keys()) {
    RoleItem *const role = new RoleItem(roles[_map[key]]);
    PortItem *const port = new PortItem(key, _offset);
    ValueItem *const value = new ValueItem();
    appendRow(QList<QStandardItem *>() << port << value << role);
  }
}

const QMap<int, int> &MappingModel::mapping() const
{
  return _map;
}

const QStringList &MappingModel::roles() const
{
  return _roles;
}