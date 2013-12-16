#ifndef _KOVAN_BUTTON_PROVIDER_HPP_
#define _KOVAN_BUTTON_PROVIDER_HPP_

#include <QObject>

#include "button.hpp"

namespace Kovan
{
	class Button;
	class KmodSim;
	
	class ButtonProvider : public QObject
	{
	Q_OBJECT
	public:
		ButtonProvider(KmodSim *sim, QObject *parent = 0);
		~ButtonProvider();
		
		virtual bool isExtraShown() const;
		virtual QString text(::Button::Type::Id id) const;
		virtual bool setPressed(::Button::Type::Id id, bool pressed);
		
	public slots:
		virtual void reset();
		virtual void refresh();
	
	signals:
		void buttonTextChanged(::Button::Type::Id id, const QString& text);
		void extraShownChanged(const bool& shown);
	
	private:
		Button m_button;
		bool m_extraShown;
	};
}

#endif
