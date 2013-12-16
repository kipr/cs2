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

#ifndef _KOVAN_KMOD_SIM_HPP_
#define _KOVAN_KMOD_SIM_HPP_

#include <QObject>
#include <QTime>

#include "kovan_protocol_p.hpp"

class QUdpSocket;

namespace Kovan
{
	struct StateResponse
	{
		unsigned char hasState : 1;
		Kovan::State state;
	};

	class KmodSim : public QObject
	{
	Q_OBJECT
	public:
		KmodSim(QObject *parent = 0);
		~KmodSim();
	
		bool setup();
		
		void reset();
		
		unsigned short servoValue(const unsigned char &port) const;
		void setMotorCounter(unsigned char port, int value);
	
		Kovan::State &state();
	
	private slots:
		void readyRead();
	
	signals:
		void stateChanged(const State &state);
	
	private:
		StateResponse do_packet(const QByteArray &datagram);
	
		QUdpSocket *m_socket;
	
		Kovan::State m_state;
	};
}

#endif
