/**************************************************************************
 * ks2 - A 2D simulator for the Kovan Robot Controller                    *
 * Copyright (C) 2012 KISS Institute for Practical Robotics               *
 *                                                                        *
 * This program is free software: you can redistribute it and/or modify   *
 * it under the terms of the GNU General Public License as published by   *
 * the Free Software Foundation, either version 3 of the License, or      *
 * (at your option) any later version.                                    *
 *                                                                        *
 * This program is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 **************************************************************************/

#define _USE_MATH_DEFINES

#include "main_window.hpp"
#include "ui_main_window.h"

#include "kovan_kmod_sim.hpp"
#include "kovan_button_provider.hpp"
#include "robot.hpp"
#include "board_selector_dialog.hpp"
#include "light.hpp"
#include "process_output_buffer.hpp"
#include "cs2.hpp"
#include "board_file.hpp"
#include "server_thread.hpp"
#include "kovan_regs_p.hpp"
#include "heartbeat.hpp"
#include "mapping_model.hpp"
#include "port_configuration.hpp"
#include "settings_dialog.hpp"
#include "archives_model.hpp"
#include <kovan/camera.hpp>

#ifdef WIN32
#include <winsock2.h>
#endif

#include <kovanserial/kovan_serial.hpp>
#include <kovanserial/tcp_server.hpp>
#include <kovanserial/udp_advertiser.hpp>
#include <pcompiler/root_manager.hpp>

#include <QTimer>
#include <QGraphicsItem>
#include <QMessageBox>
#include <QThreadPool>
#include <QProcess>
#include <QDir>
#include <QDebug>
#include <QSettings>

#include <cmath>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , _analogs(new MappingModel)
  , _digitals(new MappingModel)
  , m_robot(new Robot)
  , m_light(new Light)
  , m_buttonProvider(0)
  , m_kmod(new Kovan::KmodSim(this))
  , m_heartbeat(new Heartbeat(this))
  , m_process(0)
  , _timer(new QTimer(this))
  , _archivesModel(new ArchivesModel(this))
  , _processOutputBuffer(new ProcessOutputBuffer(this))
{
  ui->setupUi(this);
  ui->sim->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
  
  _analogs->setMapping(PortConfiguration::currentAnalogMapping(), QStringList()
    << tr("Left Range") << tr("Middle Range") << tr("Right Range")
    << tr("Left Light") << tr("Right Light") << tr("Left Reflectance")
    << tr("Right Reflectance"));
  
  _digitals->setMapping(PortConfiguration::currentDigitalMapping(), QStringList()
    << tr("Left Touch") << tr("Right Touch"));
  
  _motors = PortConfiguration::currentMotorMapping();
  
  ui->programs->setModel(_archivesModel);
  
  ui->analogs->setModel(_analogs);
  ui->digitals->setModel(_digitals);
  
  m_motors[0] = ui->motor0;
  m_motors[1] = ui->motor1;
  m_motors[2] = ui->motor2;
  m_motors[3] = ui->motor3;

  m_servos[0] = ui->servo0;
  m_servos[1] = ui->servo1;
  m_servos[2] = ui->servo2;
  m_servos[3] = ui->servo3;

  for(quint16 i = 0; i < 4; ++i) {
    m_motors[i]->setLabel(i);
    m_motors[i]->setReadOnly(true);
    m_servos[i]->setLabel(i);
    m_servos[i]->setReadOnly(true);

    m_motors[i]->setMinimumValue(-100);
    m_motors[i]->setMaximumValue(100);

    m_servos[i]->setMinimumValue(0);
    m_servos[i]->setMaximumValue(2047);
  }

  connect(_timer, SIGNAL(timeout()), SLOT(update()));
  _timer->start(50);

  connect(ui->a, SIGNAL(pressed()), SLOT(buttonPressed()));
  connect(ui->b, SIGNAL(pressed()), SLOT(buttonPressed()));
  connect(ui->c, SIGNAL(pressed()), SLOT(buttonPressed()));
  connect(ui->x, SIGNAL(pressed()), SLOT(buttonPressed()));
  connect(ui->y, SIGNAL(pressed()), SLOT(buttonPressed()));
  connect(ui->z, SIGNAL(pressed()), SLOT(buttonPressed()));
  
  connect(ui->a, SIGNAL(released()), SLOT(buttonReleased()));
  connect(ui->b, SIGNAL(released()), SLOT(buttonReleased()));
  connect(ui->c, SIGNAL(released()), SLOT(buttonReleased()));
  connect(ui->x, SIGNAL(released()), SLOT(buttonReleased()));
  connect(ui->y, SIGNAL(released()), SLOT(buttonReleased()));
  connect(ui->z, SIGNAL(released()), SLOT(buttonReleased()));
  
  connect(ui->run, SIGNAL(clicked()), SLOT(run()));
  connect(ui->remove, SIGNAL(clicked()), SLOT(remove()));
  
  connect(ui->actionStop, SIGNAL(triggered()), SLOT(stop()));
  connect(ui->actionQuit, SIGNAL(triggered()), QCoreApplication::instance(), SLOT(quit()));
  
  connect(ui->actionReset, SIGNAL(triggered()), SLOT(reset()));
  
  connect(ui->portConfig, SIGNAL(clicked()), SLOT(configPorts()));
  connect(ui->selectBoard, SIGNAL(clicked()), SLOT(selectBoard()));
  
  connect(ui->actionAbout, SIGNAL(triggered()), SLOT(about()));
  
	connect(ui->programs->selectionModel(),
		SIGNAL(currentChanged(QModelIndex, QModelIndex)),
		SLOT(currentChanged(QModelIndex)));

  const bool ret = m_kmod->setup();
  if (!ret) qWarning() << "m_kmod->setup() failed.  (main_window.cpp : " << __LINE__ << ")";

  startServer(_computerServer, KOVAN_SERIAL_PORT + 1, "computer");
  startServer(_simulatorServer, KOVAN_SERIAL_PORT + 2, "simulator");

  m_buttonProvider = new Kovan::ButtonProvider(m_kmod, this);
  ui->extras->connect(m_buttonProvider, SIGNAL(extraShownChanged(bool)), SLOT(setVisible(bool)));
  connect(m_buttonProvider,
    SIGNAL(buttonTextChanged(::Button::Type::Id, QString)),
    SLOT(textChanged(::Button::Type::Id, QString)));

  ui->actionStop->setEnabled(false);

  updateAdvert();
  updateSettings();
  currentChanged(QModelIndex());
  
  _boardFileManager.addLocation(QDir::currentPath());
  _boardFileManager.addLocation(Compiler::RootManager(_computerServer->userRoot()).boardPath());
  _boardFileManager.addLocation(Compiler::RootManager(_simulatorServer->userRoot()).boardPath());
  _boardFileManager.reload();
  updateBoard();
  
  connect(_simulatorServer, SIGNAL(newBoard(const QString &)), this, SLOT(newBoard(const QString &)));
  
  setState(MainWindow::Simulator);
  
  ui->console->setProcessOutputBuffer(_processOutputBuffer);
  ui->linkConsole->setProcessOutputBuffer(_processOutputBuffer);
}

MainWindow::~MainWindow()
{
  stop();
  _computerServer->stop();
  _simulatorServer->stop();
  delete m_robot;
  delete ui;
}

void MainWindow::setState(const State state)
{
  _state = state;
  ui->sim->setVisible(state == MainWindow::Simulator);
  ui->console->setVisible(state == MainWindow::Computer);
  ui->simControls->setVisible(state == MainWindow::Simulator);
  ui->actionZoomIn->setEnabled(state == MainWindow::Computer);
  ui->actionZoomOut->setEnabled(state == MainWindow::Computer);
  switch(_state) {
    case MainWindow::Computer: setWindowTitle(tr("My Computer")); break;
    case MainWindow::Simulator: setWindowTitle(tr("Link 2D Simulator")); break;
  }
  
}

void MainWindow::setState(const QString &state)
{
  if(state == "computer") setState(MainWindow::Computer);
  else if(state == "simulator") setState(MainWindow::Simulator);
  else qWarning() << "unknown state id:" << state;
}

MainWindow::State MainWindow::state() const
{
  return _state;
}

void MainWindow::buttonPressed()
{
  QObject *const from = sender();
  if(!from) return;	

  ::Button::Type::Id id = ::Button::Type::A;
  if(from == ui->b) id = ::Button::Type::B;
  else if(from == ui->c) id = ::Button::Type::C;
  else if(from == ui->x) id = ::Button::Type::X;
  else if(from == ui->y) id = ::Button::Type::Y;
  else if(from == ui->z) id = ::Button::Type::Z;

  m_buttonProvider->setPressed(id, true);
}

void MainWindow::buttonReleased()
{
	QObject *from = sender();
	if(!from) return;	
	
 	::Button::Type::Id id = ::Button::Type::A;
	if(from == ui->b) id = ::Button::Type::B;
	else if(from == ui->c) id = ::Button::Type::C;
	else if(from == ui->x) id = ::Button::Type::X;
	else if(from == ui->y) id = ::Button::Type::Y;
	else if(from == ui->z) id = ::Button::Type::Z;
	
	m_buttonProvider->setPressed(id, false);
}

void MainWindow::textChanged(::Button::Type::Id id, const QString &text)
{
	switch(id) {
	case ::Button::Type::A:
		ui->a->setText(text);
		break;
	case ::Button::Type::B:
		ui->b->setText(text);
		break;
	case ::Button::Type::C:
		ui->c->setText(text);
		break;
	case ::Button::Type::X:
		ui->x->setText(text);
		break;
	case ::Button::Type::Y:
		ui->y->setText(text);
		break;
	case ::Button::Type::Z:
		ui->z->setText(text);
		break;
	default: break;
	}
}

#define TIMEDIV (1.0 / 13000000) // 13 MHz clock
#define PWM_PERIOD_RAW 0.02F
#define SERVO_MAX_RAW 0.0025f
#define SERVO_MIN_RAW 0.0005f
#define PWM_PERIOD ((unsigned int)(PWM_PERIOD_RAW / TIMEDIV))
#define SERVO_MAX (SERVO_MAX_RAW / TIMEDIV)
#define SERVO_MIN (SERVO_MIN_RAW / TIMEDIV)

void MainWindow::update()
{
  if(!m_process) {
    if(_robotOld != m_robot->pos() || _lightOld != m_light->pos()) _timer->setInterval(50);
    else _timer->setInterval(300);
    
    _robotOld = m_robot->pos();
    _lightOld = m_light->pos();
    ui->sim->update();
    return;
  } else {
    _timer->setInterval(50);
  }

  m_buttonProvider->refresh();

  Kovan::State &s = m_kmod->state();

  static const int motors[4] = {
    MOTOR_PWM_0,
    MOTOR_PWM_1,
    MOTOR_PWM_2,
    MOTOR_PWM_3
  };

  static const int servos[4] = {
    SERVO_COMMAND_0,
    SERVO_COMMAND_1,
    SERVO_COMMAND_2,
    SERVO_COMMAND_3
  };

  unsigned short modes = s.t[PID_MODES];

  static const int GOAL_EPSILON = 20;
  static const int MOTOR_SCALE  = 500;

  for(int i = 0; i < 4; ++i) {
    unsigned char mode = (modes >> ((3 - i) * 2)) & 0x3;
    double val = 0.0;
    bool pwm = false;
    int code = 0;

    int pos_goal = ((int)s.t[GOAL_POS_0_HIGH + i] << 16) | s.t[GOAL_POS_0_LOW + i];
    int pos_err = 0;

    const int port = unfixPort(i);
    Q_FOREACH(const int p, _motors.keys(0)) if(port == p) pos_err = pos_goal - MOTOR_SCALE
      * (int)m_robot->leftTravelDistance();
    Q_FOREACH(const int p, _motors.keys(0)) if(port == p) pos_err = pos_goal - MOTOR_SCALE
      * (int)m_robot->rightTravelDistance();

    int desired_speed = s.t[(GOAL_SPEED_0_HIGH + i)] << 16 | s.t[(GOAL_SPEED_0_LOW + i)];

    switch(mode) {
    case 0: // pwm
    code = (s.t[MOTOR_DRIVE_CODE_T] >> ((3 - i) * 2)) & 0x3;
      val = s.t[MOTOR_PWM_0 + i] / 2600.0;
      if(code == 1) val = -val;
      else if(code != 2) val = 0.0;
      pwm = true;
      if(val > 1.0) val = 1.0;
      break;
    case 1: // position
      if ((pos_err > 0 && pos_err < GOAL_EPSILON)
          || (pos_err < 0 && pos_err > -GOAL_EPSILON)) {
        val = 0.0;
      } else {
        val = pos_err / 2000.0;
        if (val > 1.0) val = 1.0;
        else if (val < -1.0) val = -1.0;
      }
      break;
    case 2: // speed
      val = (desired_speed) / 1000.0;
      break;
    case 3: // position at speed
    if ((pos_err > 0 && pos_err < GOAL_EPSILON)
        || (pos_err < 0 && pos_err > -GOAL_EPSILON)
        || (pos_err < 0 && desired_speed > 0)
        || (pos_err > 0 && desired_speed < 0)){
      val = 0.0;
    } else {
      val = (desired_speed) / 1000.0;
    }
    break;
    }

    const static double m = 2.5;
    Q_FOREACH(const int p, _motors.keys(0)) if(port == p) m_robot->setLeftSpeed(val * (pwm ? m : 1.0));
    Q_FOREACH(const int p, _motors.keys(1)) if(port == p) m_robot->setRightSpeed(val * (pwm ? m : 1.0));

    m_motors[port]->setValue(val * 100.0);
    m_servos[i]->setValue((s.t[servos[port]] - 6500) * 2048 / 26000);
  }
  
  static const int analogs[8] = {
    AN_IN_0,
    AN_IN_1,
    AN_IN_2,
    AN_IN_3,
    AN_IN_4,
    AN_IN_5,
    AN_IN_6,
    AN_IN_7
  };
  
  const QList<int> leftRangeKeys    = _analogs->mapping().keys(0);
  const QList<int> frontRangeKeys   = _analogs->mapping().keys(1);
  const QList<int> rightRangeKeys   = _analogs->mapping().keys(2);
  const QList<int> leftLightKeys    = _analogs->mapping().keys(3);
  const QList<int> rightLightKeys   = _analogs->mapping().keys(4);
  const QList<int> leftReflectKeys  = _analogs->mapping().keys(5);
  const QList<int> rightReflectKeys = _analogs->mapping().keys(6);
  
  const QList<int> leftBumpKeys  = _digitals->mapping().keys(0);
  const QList<int> rightBumpKeys = _digitals->mapping().keys(1);
  
  if(!leftRangeKeys.isEmpty()) {
    const unsigned leftRange = m_robot->leftRange() / m_robot->rangeLength() * 1023.0;
    foreach(const int port, leftRangeKeys) s.t[analogs[port]] = leftRange;
  }
  
  if(!frontRangeKeys.isEmpty()) {
    const unsigned frontRange = m_robot->frontRange() / m_robot->rangeLength() * 1023.0;
    foreach(const int port, frontRangeKeys) s.t[analogs[port]] = frontRange;
  }
  
  if(!rightRangeKeys.isEmpty()) {
    const unsigned rightRange = m_robot->rightRange() / m_robot->rangeLength() * 1023.0;
    foreach(const int port, rightRangeKeys) s.t[analogs[port]] = rightRange;
  }
  
  if(!leftBumpKeys.isEmpty()) {
    const unsigned leftRange = m_robot->leftRange() / m_robot->rangeLength() * 1023.0;
    const bool leftBump = leftRange < 150;
    foreach(const int port, leftBumpKeys) setDigital(port, leftBump);
  }
  
  if(rightBumpKeys.isEmpty()) {
    const unsigned rightRange = m_robot->rightRange() / m_robot->rangeLength() * 1023.0;
    const bool rightBump = rightRange < 150;
    foreach(const int port, rightBumpKeys) setDigital(port, rightBump);
  }

  const static double lightDisp = 15.0;
  
  if(!leftLightKeys.isEmpty()) {
    const double lRad = M_PI * (m_robot->robot()[0]->rotation() + 45.0) / 180.0;
    const QPointF leftLightPos = m_robot->robot()[0]->pos() + lightDisp * QPointF(cos(lRad), sin(lRad));
    QLineF leftLightline(leftLightPos, m_light->pos());
    double leftLightValue = leftLightline.length() / 50.0 * 1023.0;
    if(leftLightValue > 1023.0) leftLightValue = 1023.0;
    const unsigned leftLight = m_light->isOn() ? leftLightValue : 1023.0;
    Q_FOREACH(const int port, _analogs->mapping().keys(3)) s.t[analogs[port]] = leftLight;
  }
  
  if(!rightLightKeys.isEmpty()) {
    const double rRad = M_PI * (m_robot->robot()[0]->rotation() - 45.0) / 180.0;
    const QPointF rightLightPos = m_robot->robot()[0]->pos() + lightDisp * QPointF(cos(rRad), sin(rRad));
    QLineF rightLightline(rightLightPos, m_light->pos());
    double rightLightValue = rightLightline.length() / 50.0 * 1023.0;
    if(rightLightValue > 1023.0) rightLightValue = 1023.0;
    const unsigned rightLight = m_light->isOn() ? rightLightValue : 1023.0;
    Q_FOREACH(const int port, _analogs->mapping().keys(4)) s.t[analogs[port]] = rightLight;
  }
  
  if(!leftReflectKeys.isEmpty()) {
    const unsigned leftReflect = m_robot->leftReflectance() * 1023.0;;
    Q_FOREACH(const int port, _analogs->mapping().keys(5)) s.t[analogs[port]] = leftReflect;
  }
  
  if(!rightReflectKeys.isEmpty()) {
    const unsigned rightReflect = m_robot->rightReflectance() * 1023.0;;
    Q_FOREACH(const int port, _analogs->mapping().keys(6)) s.t[analogs[port]] = rightReflect;
  }
  
  for(unsigned i = 0; i < 8; ++i) {
    _analogs->setValue(i, s.t[analogs[i]]);
    _digitals->setValue(i, s.t[DIG_IN] & (1 << (7 - i)) ? 0 : 1);
  }

  ui->scrollArea->update();
  ui->sim->update();
}


void MainWindow::finished(int exitCode)
{
  ui->actionStop->setEnabled(false);
  stop();
}

void MainWindow::run(const QString &executable, const QString &id)
{
  stop();
  
  setState(id);
  
  m_process = new QProcess();
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  // TODO: This will only work on OS X
#ifdef Q_OS_MAC
  env.insert("DYLD_LIBRARY_PATH", QDir::currentPath() + "/prefix/usr/lib:" + env.value("DYLD_LIBRARY_PATH"));
  env.insert("DYLD_LIBRARY_PATH", QDir::currentPath() + "/prefix/usr:" + env.value("DYLD_LIBRARY_PATH"));
#endif
  
  env.insert("CAMERA_BASE_CONFIG_PATH", _workingDirectory.filePath("vision"));
  
  ServerThread *server = qobject_cast<ServerThread *>(sender());
  if(!server) server = _computerServer;
  Compiler::RootManager root(server->userRoot());
#ifdef Q_OS_MAC
  env.insert("DYLD_LIBRARY_PATH", env.value("DYLD_LIBRARY_PATH") + ":"
    + root.libDirectoryPaths().join(":"));
#elif defined(Q_OS_WIN)
  env.insert("PATH", env.value("PATH") + ";" + QDir::currentPath().replace("/", "\\"));
  env.insert("PATH", env.value("PATH") + ";" + root.libDirectoryPaths().join(";").replace("/", "\\"));
  // QMessageBox::information(this, "test", env.value("PATH"));
#else
  env.insert("LD_LIBRARY_PATH", env.value("LD_LIBRARY_PATH") + ":"
    + root.libDirectoryPaths().join(":"));
#endif

  m_process->setProcessEnvironment(env);
  m_process->setWorkingDirectory(_workingDirectory.path());
  connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished()));

  m_process->start(root.bin(executable).filePath(executable), QStringList());
  _processOutputBuffer->setProcess(m_process);
  if(!m_process->waitForStarted(1000)) {
    const QString msg = tr("Failed to start %1").arg(executable);
    ui->console->append(msg);
    ui->linkConsole->append(msg);
    return;
  }
  
  processStarted();

  raise();
}

void MainWindow::stop()
{
  update();
  if(!m_process) return;
  _processOutputBuffer->setProcess(0);

  m_process->kill();
  m_process->waitForFinished();
  delete m_process;
  m_process = 0;
  ui->actionStop->setEnabled(false);
  m_kmod->reset();
}

void MainWindow::updateSettings()
{
  QSettings settings;
  settings.beginGroup(APPEARANCE);
  QColor consoleColor = settings.value(CONSOLE_COLOR).value<QColor>();
  QColor textColor = settings.value(TEXT_COLOR).value<QColor>();
  QFont font = settings.value(FONT).value<QFont>();
  qreal fontSize = settings.value(FONT_SIZE).toInt();
  
  ui->console->document()->setMaximumBlockCount(settings.value(MAXIMUM_SCROLLBACK, 100000).toInt());
  ui->linkConsole->document()->setMaximumBlockCount(settings.value(MAXIMUM_SCROLLBACK, 100000).toInt());
  
  settings.endGroup();

  updateAdvert();

  QPalette pal = ui->console->palette();
  pal.setColor(QPalette::Base, consoleColor);

  QString contents = ui->console->toPlainText();
  ui->console->clear();
  ui->console->setTextColor(textColor);
  font.setPointSize(fontSize);
  ui->console->setFont(font);
  ui->console->setPlainText(contents);

  settings.beginGroup(STORAGE);
  QString workPath = settings.value(WORKING_DIRECTORY, QDir::homePath() + "/"
  	+ tr("KISS Work Dir")).toString();

  _workingDirectory = QDir(workPath);
  if(!_workingDirectory.exists()) QDir().mkpath(workPath);

  Camera::ConfigPath::setBasePath(_workingDirectory.absoluteFilePath("vision").toStdString());

  QString progPath = settings.value(PROGRAM_DIRECTORY, QDir::homePath() + "/"
  	+ tr("KISS Programs")).toString();

  QDir prog(progPath);
  prog.makeAbsolute();
  if(!prog.exists()) QDir().mkpath(prog.absolutePath());
  
  Compiler::RootManager root(prog.path());
  root.ensureSetup();
  _simulatorServer->setUserRoot(prog.path());
  _computerServer->setUserRoot(prog.path());
  _archivesModel->setArchivesRoot(root.archivesPath());

  settings.endGroup();
}

void MainWindow::updateAdvert()
{
  if(!m_heartbeat) return;
  QString version = (QString::number(CS2_VERSION_MAJOR) + "." + QString::number(CS2_VERSION_MINOR));
  #if defined(Q_OS_WIN)
  version += " for Windows";
  #elif defined(Q_OS_MAC)
  version += " for Mac OS X";
  #else
  version += " for *nix";
  #endif

  Advert ads(tr("N/A").toUtf8(),
    version.toUtf8(),
    tr("2D Simulator").toUtf8(),
    tr("Simulator").toUtf8(),
    KOVAN_SERIAL_PORT + 2);
    
  Advert adc(tr("N/A").toUtf8(),
    version.toUtf8(),
    tr("Local Computer").toUtf8(),
    tr("Computer").toUtf8(),
    KOVAN_SERIAL_PORT + 1);
    
	m_heartbeat->setAdverts(QList<Advert>() << ads << adc);
}

int MainWindow::unfixPort(int port)
{
  return port ^ (~port & 1);
}

void MainWindow::reset()
{
  m_buttonProvider->reset();
  m_robot->setLeftSpeed(0.0);
  m_robot->setRightSpeed(0.0);
  m_robot->setLeftTravelDistance(0.0);
  m_robot->setRightTravelDistance(0.0);
  m_robot->reset();
  m_light->reset();
  m_robot->robot()[0]->setRotation(45);
  ui->linkConsole->clear();
  ui->console->clear();
}

void MainWindow::setDigital(int port, bool on)
{
  Kovan::State &s = m_kmod->state();
  if(!on) s.t[DIG_IN] |= 1 << (7 - port);
  else s.t[DIG_IN] &= ~(1 << (7 - port));
}

void MainWindow::configPorts()
{
  PortConfiguration config;
  config.setAnalogRoles(_analogs->roles());
  config.setDigitalRoles(_digitals->roles());
  config.setMotorRoles(QStringList() << "Left Wheel" << "Right Wheel");
  config.setAnalogSize(8);
  config.setDigitalSize(8, 8);
  config.setMotorSize(4);
  if(config.exec() == QDialog::Rejected) return;
  _analogs->setMapping(config.analogMapping(), _analogs->roles());
  _digitals->setMapping(config.digitalMapping(), _digitals->roles(), 8);
  _motors = config.motorMapping();
}

void MainWindow::run()
{
  const QModelIndexList indexes = ui->programs->selectionModel()->selectedIndexes();
  if(indexes.size() != 1) return;
  QModelIndex index = indexes[0];
  const kiss::KarPtr archive = _archivesModel->archive(index);
  const QString id(archive->data(SERVER_ID_FILE));
  run(_archivesModel->name(index), id.isEmpty() ? "computer" : id);
}

void MainWindow::remove()
{
  const QModelIndexList indexes = ui->programs->selectionModel()->selectedIndexes();
  if(indexes.size() != 1) return;
  QModelIndex index = indexes[0];
  QDir dir(_archivesModel->archivesRoot());
  dir.cdUp();
  Compiler::RootManager(dir.absolutePath()).uninstall(_archivesModel->name(index));
  currentChanged(QModelIndex());
}

void MainWindow::currentChanged(const QModelIndex index)
{
  const bool e = index.isValid();
  ui->run->setEnabled(e);
  ui->remove->setEnabled(e);
  
  if(e) setState(_archivesModel->id(index));
}

void MainWindow::updateBoard()
{
  QSettings settings;
  settings.beginGroup("board");
  BoardFile *const boardFile = _boardFileManager.lookupBoardFile(settings.value("current_board",
    "2013").toString());
  settings.endGroup();
  ui->sim->setScene(boardFile->scene());
  putRobotAndLight();
}

void MainWindow::selectBoard()
{
  _boardFileManager.reload();
  
  BoardSelectorDialog boardSelector(this);
  boardSelector.setBoardFiles(_boardFileManager.boardFiles());
  if(boardSelector.exec() != QDialog::Accepted) return;
  BoardFile *const board = boardSelector.selectedBoardFile();
  if(!board) return;
  
  QSettings settings;
  settings.beginGroup("board");
  settings.setValue("current_board", board->name());
  settings.endGroup();
  settings.sync();
  updateBoard();
}

void MainWindow::newBoard(const QString& board)
{ 
  _boardFileManager.reload();
  
  QSettings settings;
  settings.beginGroup("board");
  settings.setValue("current_board", QFileInfo(board).baseName());
  settings.endGroup();
  settings.sync();
  updateBoard();
}

bool MainWindow::putRobotAndLight()
{
	if(!ui->sim->scene()) return false;
  
  Q_FOREACH(QGraphicsItem *item, m_robot->robot())
    ui->sim->scene()->addItem(item);
  ui->sim->scene()->addItem(m_light);
  m_robot->robot()[0]->setRotation(45);
  
  return true;
}


void MainWindow::about()
{
  QMessageBox::information(this, tr("About cs2"), tr("Version %1.%2\nCopyright 2013 KISS"
    " Institute for Practical Robotics")
    .arg(CS2_VERSION_MAJOR).arg(CS2_VERSION_MINOR));
}

void MainWindow::processStarted()
{
  ui->actionStop->setEnabled(true);
  if(ui->console->isVisible()) {
    _time.restart();
    ui->console->append(tr("Started at %1\n\n").arg(_time.toString()));
  }
}

void MainWindow::processFinished()
{
  ui->actionStop->setEnabled(false);
  if(ui->console->isVisible()) {
	  const int msecs = _time.elapsed();
	  _time.restart();
	  ui->console->append(tr("\nFinished at %1 in %2 seconds").arg(_time.toString()).arg(msecs / 1000.0));
  }
}

void MainWindow::startServer(ServerThread *&server, const quint16 port, const QString &id)
{
  TcpServer *const serial = new TcpServer;
  serial->setConnectionRestriction(TcpServer::OnlyLocal);
  if(!serial->bind(port)) {
    perror("bind");
#ifdef WIN32
    qCritical() << WSAGetLastError();
#endif
  }
  if(!serial->listen(2)) {
    perror("listen");
#ifdef WIN32
    qCritical() << WSAGetLastError();
#endif
  }
  server = new ServerThread(serial, id);
  server->setUserRoot(QDir::tempPath());
  connect(server, SIGNAL(run(QString, QString)), SLOT(run(QString, QString)));
  server->setAutoDelete(true);
  QThreadPool::globalInstance()->start(server);
}
