#include "port_configuration.hpp"
#include "ui_port_configuration.h"

#include <QSettings>
#include <QInputDialog>

QDataStream &operator<<(QDataStream &out, const PortConfig &obj)
{
  
  out << obj.name << obj.analog << obj.digital << obj.motor;
  return out;
}
 
QDataStream &operator>>(QDataStream &in, PortConfig &obj)
{
  in >> obj.name >> obj.analog >> obj.digital >> obj.motor;
  return in;
}


PortConfiguration::PortConfiguration(QWidget *const parent)
  : QDialog(parent)
  , ui(new Ui::PortConfiguration)
  , _current(-1)
{
  ui->setupUi(this);
  
  connect(ui->configs, SIGNAL(currentIndexChanged(int)), SLOT(currentConfigChanged(int)));
  connect(ui->newConfig, SIGNAL(clicked()), SLOT(newConfig()));
  connect(ui->deleteConfig, SIGNAL(clicked()), SLOT(deleteConfig()));
  
  _configs = loadConfigs();
  refreshCombo();
}

PortConfiguration::~PortConfiguration()
{
  delete ui;
}

void PortConfiguration::setAnalogSize(const quint8 size, const quint8 offset)
{
  QStringList strs;
  for(quint8 i = 0; i < size; ++i) strs << tr("Analog %1").arg(i + offset);
  ui->analogs->setPorts(strs);
}

void PortConfiguration::setDigitalSize(const quint8 size, const quint8 offset)
{
  QStringList strs;
  for(quint8 i = 0; i < size; ++i) strs << tr("Digital %1").arg(i + offset);
  ui->digitals->setPorts(strs);
}

void PortConfiguration::setMotorSize(const quint8 size, const quint8 offset)
{
  QStringList strs;
  for(quint8 i = 0; i < size; ++i) strs << tr("Motor %1").arg(i + offset);
  ui->motors->setPorts(strs);
}

quint8 PortConfiguration::analogSize() const
{
  return ui->analogs->ports().size();
}

quint8 PortConfiguration::digitalSize() const
{
  return ui->digitals->ports().size();
}

quint8 PortConfiguration::motorSize() const
{
  return ui->motors->ports().size();
}

void PortConfiguration::setAnalogRoles(const QStringList &roles)
{
  ui->analogs->setRoles(roles);
}

void PortConfiguration::setDigitalRoles(const QStringList &roles)
{
  ui->digitals->setRoles(roles);
}

void PortConfiguration::setMotorRoles(const QStringList &roles)
{
  ui->motors->setRoles(roles);
}

const QStringList &PortConfiguration::analogRoles() const
{
  return ui->analogs->roles();
}

const QStringList &PortConfiguration::digitalRoles() const
{
  return ui->digitals->roles();
}

const QStringList &PortConfiguration::motorRoles() const
{
  return ui->motors->roles();
}

QMap<int, int> PortConfiguration::analogMapping() const
{
  return ui->analogs->mapping();
}

QMap<int, int> PortConfiguration::digitalMapping() const
{
  return ui->digitals->mapping();
}

QMap<int, int> PortConfiguration::motorMapping() const
{
  return ui->motors->mapping();
}

QMap<int, int> PortConfiguration::currentAnalogMapping()
{
  return loadConfigs()[0].analog;
}

QMap<int, int> PortConfiguration::currentDigitalMapping()
{
  return loadConfigs()[0].digital;
}

QMap<int, int> PortConfiguration::currentMotorMapping()
{
  return loadConfigs()[0].motor;
}

int PortConfiguration::exec()
{
  int ret = QDialog::exec();
  if(ret == QDialog::Rejected) return ret;
  currentConfigChanged(-1);
  saveConfigs();
  return ret;
}

void PortConfiguration::newConfig()
{
  QString config = QInputDialog::getText(this, tr("Configuration Name"), tr("New Configuration Name"));
  if(config.isEmpty()) return;
  currentConfigChanged(-1);
  PortConfig c;
  c.name = config;
  _configs.prepend(c);
  refreshCombo();
}

void PortConfiguration::deleteConfig()
{
  currentConfigChanged(-1);
  _configs.removeAt(ui->configs->currentIndex());
  refreshCombo();
}

void PortConfiguration::currentConfigChanged(const int index)
{
  if(_current < _configs.size() && _current >= 0) {
    _configs[_current].analog = ui->analogs->mapping();
    _configs[_current].digital = ui->digitals->mapping();
    _configs[_current].motor = ui->motors->mapping();
  }
  _current = index;
  if(index >= _configs.size() || index < 0) return;
  ui->analogs->setMapping(_configs[index].analog);
  ui->digitals->setMapping(_configs[index].digital);
  ui->motors->setMapping(_configs[index].motor);
  
}

PortConfig PortConfiguration::defaultConfig()
{
  PortConfig c;
  c.name = "Default";
  
  c.analog[0] = 0;
  c.analog[1] = 1;
  c.analog[2] = 2;
  c.analog[3] = 3;
  c.analog[4] = 4;
  c.analog[5] = 5;
  c.analog[6] = 6;
  
  c.digital[0] = 0;
  c.digital[1] = 1;
  
  c.motor[0] = 0;
  c.motor[2] = 1;
  return c;
}

QList<PortConfig> PortConfiguration::loadConfigs()
{
  qRegisterMetaType<PortConfig>("PortConfig");
  qRegisterMetaTypeStreamOperators<PortConfig>("PortConfig");
  QSettings settings;
  settings.beginGroup("port_configs");
  const int size = settings.value("size", 0).toInt();
  QList<PortConfig> configs;
  if(size > 0) {
    for(int i = 0; i < size; ++i) {
      configs.append(settings.value(QString::number(i)).value<PortConfig>());
    }
    const int active = settings.value("active", -1).toInt();
    if(active < configs.size() && active >= 0) configs.swap(active, 0);
  }
  // Add default config
  if(configs.isEmpty()) configs.append(defaultConfig());
  settings.endGroup();
  return configs;
}

void PortConfiguration::saveConfigs()
{
  qRegisterMetaType<PortConfig>("PortConfig");
  qRegisterMetaTypeStreamOperators<PortConfig>("PortConfig");
  QSettings settings;
  settings.beginGroup("port_configs");
  settings.remove("");
  settings.setValue("size", _configs.size());
  for(int i = 0; i < _configs.size(); ++i) settings.setValue(QString::number(i), QVariant::fromValue(_configs[i]));
  settings.setValue("active", ui->configs->currentIndex());
  settings.endGroup();
}

void PortConfiguration::refreshCombo()
{
  ui->configs->clear();
  foreach(const PortConfig &config, _configs) ui->configs->addItem(config.name);
  ui->deleteConfig->setEnabled(_configs.size() > 1);
}
