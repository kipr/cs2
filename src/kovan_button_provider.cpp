#include "kovan_button_provider.hpp"

#include "button.hpp"

Kovan::ButtonProvider::ButtonProvider(KmodSim *sim, QObject *parent)
	: QObject(parent),
	m_button(sim),
	m_extraShown(false)
{
	reset();
}

Kovan::ButtonProvider::~ButtonProvider()
{
}

bool Kovan::ButtonProvider::isExtraShown() const
{
	return m_button.isExtraShown();
}

QString Kovan::ButtonProvider::text(::Button::Type::Id id) const
{
	const char *text = m_button.text(id);
	return text ? QString::fromUtf8(text) : QString();
}

bool Kovan::ButtonProvider::setPressed(::Button::Type::Id id, bool pressed)
{
	m_button.setPressed(id, pressed);
	return true;
}

void Kovan::ButtonProvider::reset()
{
	m_button.resetButtons();
	m_extraShown = false;
	emit extraShownChanged(false);
}

void Kovan::ButtonProvider::refresh()
{
	if(m_button.isTextDirty(::Button::Type::A)) emit buttonTextChanged(::Button::Type::A, text(::Button::Type::A));
	if(m_button.isTextDirty(::Button::Type::B)) emit buttonTextChanged(::Button::Type::B, text(::Button::Type::B));
	if(m_button.isTextDirty(::Button::Type::C)) emit buttonTextChanged(::Button::Type::C, text(::Button::Type::C));
	if(m_button.isTextDirty(::Button::Type::X)) emit buttonTextChanged(::Button::Type::X, text(::Button::Type::X));
	if(m_button.isTextDirty(::Button::Type::Y)) emit buttonTextChanged(::Button::Type::Y, text(::Button::Type::Y));
	if(m_button.isTextDirty(::Button::Type::Z)) emit buttonTextChanged(::Button::Type::Z, text(::Button::Type::Z));
	if(m_extraShown != m_button.isExtraShown()) {
		m_extraShown = m_button.isExtraShown();
		emit extraShownChanged(m_extraShown);
	}
}
