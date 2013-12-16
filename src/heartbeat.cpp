#include "heartbeat.hpp"

#include <QTimer>
#include <QDebug>

Heartbeat::Heartbeat(QObject *parent)
	: QObject(parent),
	_advertiser(true)
{
	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), SLOT(beat()));
	timer->start(3000);
}

Heartbeat::~Heartbeat()
{
}

void Heartbeat::setAdverts(const QList<Advert> &adverts)
{
	_adverts = adverts;
}

const QList<Advert> &Heartbeat::adverts() const
{
	return _adverts;
}

void Heartbeat::beat()
{
	_advertiser.reset();
	Q_FOREACH(const Advert &a, _adverts) _advertiser.pulse(a);
}