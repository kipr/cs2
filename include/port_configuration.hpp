#ifndef _PORT_CONFIGURATION_HPP_
#define _PORT_CONFIGURATION_HPP_

#include <QDialog>
#include <QStringList>
#include <QMap>
#include <QMetatype>

namespace Ui
{
  class PortConfiguration;
}

struct PortConfig
{
  QString name;
  QMap<int, int> analog;
  QMap<int, int> digital;
  QMap<int, int> motor;
};

class PortConfiguration : public QDialog
{
Q_OBJECT
public:
  PortConfiguration(QWidget *const parent = 0);
  ~PortConfiguration();
  
  void setAnalogSize(const quint8 size, const quint8 offset = 0);
  void setDigitalSize(const quint8 size, const quint8 offset = 0);
  void setMotorSize(const quint8 size, const quint8 offset = 0);
  
  quint8 analogSize() const;
  quint8 digitalSize() const;
  quint8 motorSize() const;
  
  void setAnalogRoles(const QStringList &roles);
  void setDigitalRoles(const QStringList &roles);
  void setMotorRoles(const QStringList &roles);
  
  const QStringList &analogRoles() const;
  const QStringList &digitalRoles() const;
  const QStringList &motorRoles() const;
  
  QMap<int, int> analogMapping() const;
  QMap<int, int> digitalMapping() const;
  QMap<int, int> motorMapping() const;
  
  static QMap<int, int> currentAnalogMapping();
  static QMap<int, int> currentDigitalMapping();
  static QMap<int, int> currentMotorMapping();
  
  virtual int exec();
  
private slots:
  void newConfig();
  void deleteConfig();
  
  void currentConfigChanged(const int index);
  
private:
  Ui::PortConfiguration *ui;
  
  static PortConfig defaultConfig();
  
  static QList<PortConfig> loadConfigs();
  void saveConfigs();
  
  void refreshCombo();
  
  QList<PortConfig> _configs;
  int _current;
};

Q_DECLARE_METATYPE(PortConfig);

QDataStream &operator<<(QDataStream &out, const PortConfig &obj);
QDataStream &operator>>(QDataStream &in, PortConfig &obj);

#endif
