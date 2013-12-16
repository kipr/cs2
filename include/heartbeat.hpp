#ifndef _HEARTBEAT_HPP_
#define _HEARTBEAT_HPP_

#include <QObject>
#include <QList>

#include <kovanserial/udp_advertiser.hpp>

class Heartbeat : public QObject
{
Q_OBJECT
public:
	Heartbeat(QObject *parent = 0);
	~Heartbeat();
	
	void setAdverts(const QList<Advert> &advert);
	const QList<Advert> &adverts() const;
	
private slots:
	void beat();
	
private:
	UdpAdvertiser _advertiser;
	QList<Advert> _adverts;
};

#endif
