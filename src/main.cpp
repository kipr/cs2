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

#include "main_window.hpp"

#include <QApplication>
#include <QDir>
#include "board_file_manager.hpp"
#include "board_selector_dialog.hpp"

#ifdef _MSC_VER
#pragma comment(linker, "/ENTRY:mainCRTStartup")
#endif

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
  
	QApplication::setOrganizationName("KIPR");
	QApplication::setOrganizationDomain("kipr.org");
	QApplication::setApplicationName("ks2");
  
#ifdef Q_OS_MAC
	QDir::setCurrent(QApplication::applicationDirPath() + "/../");
#else
	QDir::setCurrent(QApplication::applicationDirPath());
#endif
  
	MainWindow mainWindow;
	mainWindow.show();
	mainWindow.raise();
	return app.exec();
}