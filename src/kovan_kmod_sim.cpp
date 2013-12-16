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

#include "kovan_kmod_sim.hpp"
#include "kovan_regs_p.hpp"

#include <QUdpSocket>
#include <QThread>

#define NUM_RW_REGS 19
#define RO_REG_OFFSET 0
#define RW_REG_OFFSET 24

#define WRITE_COMMAND_BUFF_SIZE NUM_RW_REGS

#define TIMEDIV (1.0 / 13000000) // 13 MHz clock
#define PWM_PERIOD_RAW 0.02F
#define SERVO_MAX_RAW 0.0025f
#define SERVO_MIN_RAW 0.0005f
#define PWM_PERIOD ((unsigned int)(PWM_PERIOD_RAW / TIMEDIV))
#define SERVO_MAX (SERVO_MAX_RAW / TIMEDIV)
#define SERVO_MIN (SERVO_MIN_RAW / TIMEDIV)

static const int servos[4] = {
	SERVO_COMMAND_0,
	SERVO_COMMAND_1,
	SERVO_COMMAND_2,
	SERVO_COMMAND_3
};

Kovan::KmodSim::KmodSim(QObject *parent)
	: QObject(parent),
	m_socket(new QUdpSocket(this))
{
	reset();
	connect(m_socket, SIGNAL(readyRead()), SLOT(readyRead()));
}

Kovan::KmodSim::~KmodSim()
{
	delete m_socket;
}

bool Kovan::KmodSim::setup()
{
	return m_socket->bind(QHostAddress::LocalHost, 4628, QUdpSocket::ReuseAddressHint);
}

void Kovan::KmodSim::reset()
{
	memset(&m_state, 0, sizeof(State));
}

unsigned short Kovan::KmodSim::servoValue(const unsigned char &port) const
{
	return ((m_state.t[servos[port]] << 8) - SERVO_MIN) / (SERVO_MAX - SERVO_MIN) * 1024.0;
}

void Kovan::KmodSim::setMotorCounter(unsigned char port, int value)
{
	m_state.t[BEMF_0_HIGH + port] = (value >> 16) & 0x0000FFFF;
	m_state.t[BEMF_0_LOW + port] = (value >> 0) & 0x0000FFFF;
}

Kovan::State &Kovan::KmodSim::state()
{
	return m_state;
}

void Kovan::KmodSim::readyRead()
{
	while(m_socket->hasPendingDatagrams()) {
		QByteArray datagram;
		datagram.resize(m_socket->pendingDatagramSize());
	
		QHostAddress sender;
		quint16 senderPort;
		m_socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
		
		Kovan::StateResponse s = do_packet(datagram);
		if(!s.hasState) continue;
		
		m_socket->writeDatagram(reinterpret_cast<const char *>(&s.state), sizeof(State), sender, senderPort);
	}
	
	emit stateChanged(m_state);
}

Kovan::StateResponse Kovan::KmodSim::do_packet(const QByteArray &datagram)
{


	int have_state_request = 0;
	int num_write_commands = 0;

	StateResponse response;
	memset(&response, 0, sizeof(Kovan::StateResponse));
	
	// qDebug() << "Doing packet";
	
	if(datagram.size() < sizeof(Packet)) {
		qWarning() << "Packet was too small!! Not processing.";
		return response; // Error: Packet is too small?
	}
	
	Packet *packet = (Packet *)datagram.data();
	for(unsigned short i = 0; i < packet->num; ++i) {
		Command cmd = packet->commands[i];
		
		WriteCommand *w_cmd = 0;
		switch(cmd.type) {
		case StateCommandType:
			have_state_request = 1;
			break;
		
		case WriteCommandType:
			w_cmd = (WriteCommand *) &(cmd.data);
			if(w_cmd->addy >= TOTAL_REGS) break;
			m_state.t[w_cmd->addy] = w_cmd->val;
			break;
		
		default: break;
		}
	}

	if(have_state_request) {
		Kovan::StateResponse s;
		s.hasState = 1;
		s.state = m_state;
		response = s;
	}

	return response;
}
