/**************************************************************************
 *  Copyright 2012 KISS Institute for Practical Robotics                  *
 *                                                                        *
 *  This file is part of libkovan.                                        *
 *                                                                        *
 *  libkovan is free software: you can redistribute it and/or modify      *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation, either version 2 of the License, or     *
 *  (at your option) any later version.                                   *
 *                                                                        *
 *  libkovan is distributed in the hope that it will be useful,           *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with libkovan. Check the LICENSE file in the project root.      *
 *  If not, see <http://www.gnu.org/licenses/>.                           *
 **************************************************************************/

#include "button.hpp"
#include "kovan_kmod_sim.hpp"
#include "kovan_regs_p.hpp"

#include <cstring>
#include <cstdio>
#include <iostream>

Kovan::Button::Button(KmodSim *sim)
	: m_sim(sim)
{
	resetButtons();
}

void Kovan::Button::setText(const ::Button::Type::Id &id, const char *text)
{
	const unsigned char offset = buttonOffset(id);
	if(offset > 5) return;
	
	strncpy(m_text[offset], text, MAX_BUTTON_TEXT_SIZE);
	m_text[offset][MAX_BUTTON_TEXT_SIZE - 1] = 0;
	
	unsigned short start = 0;
	unsigned short end = 0;
	if(!buttonRegs(start, end, id)) return;
	
	for(int j = 0; j < MAX_BUTTON_TEXT_SIZE; j += 2) {
		m_sim->state().t[start + j / 2] = m_text[offset][j] << 8 | m_text[offset][j + 1];
	}
	m_sim->state().t[BUTTON_TEXT_DIRTY] |= 1 << offset;
}

bool Kovan::Button::isTextDirty(const ::Button::Type::Id &id) const
{
	const unsigned char offset = buttonOffset(id);
	if(offset > 5) return false;
	unsigned short &dirty = m_sim->state().t[BUTTON_TEXT_DIRTY];
	
	if(!((dirty >> offset) & 1)) return false;
	dirty &= ~(1 << offset);
	return true;
}

const char *Kovan::Button::text(const ::Button::Type::Id &id) const
{
	const unsigned char offset = buttonOffset(id);
	if(offset > 5) return m_text[offset];
	
	unsigned short start = 0;
	unsigned short end = 0;
	if(!buttonRegs(start, end, id)) return 0;
	
	unsigned short *registers = m_sim->state().t;
	unsigned char j = 0;
	for(unsigned short i = start; j < MAX_BUTTON_TEXT_SIZE && i <= end; ++i) {
		m_text[offset][j++] = (registers[i] >> 8) & 0x00FF;
		m_text[offset][j++] = registers[i] & 0x00FF;
	}
	m_text[offset][MAX_BUTTON_TEXT_SIZE - 1] = 0;
	return m_text[offset];
}

void Kovan::Button::setPressed(const ::Button::Type::Id &id, bool pressed)
{
	const unsigned char offset = buttonOffset(id);
	if(offset > 5) return;
	unsigned short &states = m_sim->state().t[BUTTON_STATES];
	if(pressed) states |= (1 << offset);
	else states &= ~(1 << offset);
	std::cout << "New states: " << std::hex << states << std::endl;
}

bool Kovan::Button::isPressed(const ::Button::Type::Id &id) const
{
	if(id == ::Button::Type::Side) {
		return m_sim->state().t[SIDE_BUTTON];
	}
	
	const unsigned char offset = buttonOffset(id);
	if(offset > 5) return false;
	unsigned short &states = m_sim->state().t[BUTTON_STATES];
	return (states >> offset) & 1;
}

void Kovan::Button::setExtraShown(const bool& shown)
{
	unsigned short &states = m_sim->state().t[BUTTON_STATES];
	if(shown) states |= 0x8000;
	else states &= 0x7FFF;
}

bool Kovan::Button::isExtraShown() const
{
	return m_sim->state().t[BUTTON_STATES] & 0x8000;
}

void Kovan::Button::resetButtons()
{
	setText(::Button::Type::A, "A");
	setText(::Button::Type::B, "B");
	setText(::Button::Type::C, "C");
	setText(::Button::Type::X, "X");
	setText(::Button::Type::Y, "Y");
	setText(::Button::Type::Z, "Z");
}

unsigned char Kovan::Button::buttonOffset(const ::Button::Type::Id &id) const
{
	return (unsigned char)id;
}

bool Kovan::Button::buttonRegs(unsigned short &start, unsigned short &end, ::Button::Type::Id id) const
{
	switch(id) {
	case ::Button::Type::A:
		start = BUTTON_A_TEXT_START;
		end = BUTTON_A_TEXT_END;
		break;
	case ::Button::Type::B:
		start = BUTTON_B_TEXT_START;
		end = BUTTON_B_TEXT_END;
		break;
	case ::Button::Type::C:
		start = BUTTON_C_TEXT_START;
		end = BUTTON_C_TEXT_END;
		break;
	case ::Button::Type::X:
		start = BUTTON_X_TEXT_START;
		end = BUTTON_X_TEXT_END;
		break;
	case ::Button::Type::Y:
		start = BUTTON_Y_TEXT_START;
		end = BUTTON_Y_TEXT_END;
		break;
	case ::Button::Type::Z:
		start = BUTTON_Z_TEXT_START;
		end = BUTTON_Z_TEXT_END;
		break;
	default: return false;
	}
	return true;
}
