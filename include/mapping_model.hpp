#ifndef _MAPPING_MODEL_HPP_
#define _MAPPING_MODEL_HPP_

#include <QStandardItemModel>

class MappingModel : public QStandardItemModel
{
  Q_OBJECT
public:
  MappingModel(QObject *const parent = 0);
  ~MappingModel();
  
  void setValue(const int port, const int value);
  
  void setMapping(const QMap<int, int> &map, const QStringList &roles, const int offset = 0);
  const QMap<int, int> &mapping() const;
  const QStringList &roles() const;
  
private:
  QMap<int, int> _map;
  int _offset;
  QStringList _roles;
};

#endif
