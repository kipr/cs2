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

#ifndef _MAIN_WINDOW_HPP_
#define _MAIN_WINDOW_HPP_

#include <QMainWindow>
#include <QDir>
#include <QMap>
#include <QModelIndex>
#include <QTime>

#include "button_ids.hpp"
#include "board_file_manager.hpp"
#include "settings_dialog.hpp"

namespace Ui
{
	class MainWindow;
}

class TouchDial;
class QLabel;
class QProcess;
class Robot;
class Light;
class Heartbeat;
class ServerThread;
class MappingModel;
class ProcessOutputBuffer;
class QTimer;

namespace Kovan
{
	class KmodSim;
	class ButtonProvider;
}

class ArchivesModel;

class MainWindow : public QMainWindow
{
Q_OBJECT
public:
  enum State {
    Simulator,
    Computer
  };
  
	MainWindow(State startingState, QWidget *parent = 0);
	~MainWindow();
  
  void setState(const State state);
  void setState(const QString &state);
  State state() const;
	
private slots:
	void buttonPressed();
	void buttonReleased();
	void textChanged(::Button::Type::Id id, const QString &text);
	void update();
	void reset();
  void settings();
  void vision();
	
	void run(const QString &executable, const QString &id);
  void run();
  void remove();
  void currentChanged(const QModelIndex index);
	void stop();
  
  void configPorts();
  
  void updateBoard();
  void selectBoard();
  
  void updateSettings();
  
  bool putRobotAndLight();
  
  void about();
  
  void processStarted();
  void processFinished();
  
  void selectArchive(const QString &name);
	
private:
  bool setBoard(BoardFile *boardFile);
  QString defaultBoard();
	void updateAdvert();
	int unfixPort(int port);
	void setDigital(int port, bool on);
  QString displayNameComputer();
  QString displayNameSimulator();
  
  void startServer(ServerThread *&server, const quint16 port, const QString &id);
  
  State _state;
  BoardFileManager _boardFileManager;
	
	Ui::MainWindow *ui;
  SettingsDialog m_settingsDialog;
	
  MappingModel *_analogs;
  MappingModel *_digitals;
  QMap<int, int> _motors;
	TouchDial *m_motors[4];
	TouchDial *m_servos[4];
  
	Robot *m_robot;
	Light *m_light;
	QPointF _robotOld;
	QPointF _lightOld;
  
	ServerThread *_computerServer;
	ServerThread *_simulatorServer;
	
	Kovan::ButtonProvider *m_buttonProvider;
	Kovan::KmodSim *m_kmod;
	
	Heartbeat *m_heartbeat;
	
	QProcess *m_process;
  
  QTimer *_timer;
  QTime _time;
  
  QDir _workingDirectory;
  ArchivesModel *_archivesModel;
  
  ProcessOutputBuffer *const _processOutputBuffer;
};

#endif
