/*
   Copyright (C) 2010 Marco Ballesio <gibrovacco@gmail.com>
   Copyright (C) 2011-2012 Collabora Ltd.
     @author George Kiagiadakis <george.kiagiadakis@collabora.co.uk>

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "player.h"
#include <QGlib/Connect>
#include <QGlib/Error>
#include <QGst/ElementFactory>
#include <QGst/Bus>

Player::Player(QObject *parent)
    : QObject(parent)
{
}

void Player::setVideoSink(const QGst::ElementPtr & sink)
{
    m_videoSink = sink;
}

void Player::play()
{
    if (m_pipeline) {
        m_pipeline->setState(QGst::StatePlaying);
    }
}

void Player::secret ()
{
    if (!m_pipeline) {
		m_pipeline = QGst::Pipeline::create();
        if (m_pipeline) {

			QGst::CapsPtr caps = QGst::Caps::fromString (
				"application/x-rtp, media=(string)video"
				);

			QGst::ElementPtr src = QGst::ElementFactory::make("udpsrc");
			src->setProperty ("port", 5000);
			src->setProperty ("caps", caps);
//			caps->unref();

			QGst::ElementPtr jitterbufferfuck = QGst::ElementFactory::make("rtpjitterbuffer");
			QGst::ElementPtr depayfuck = QGst::ElementFactory::make("rtph264depay");
			QGst::ElementPtr avdecfuck = QGst::ElementFactory::make("avdec_h264");
			QGst::ElementPtr videoconvertfuck = QGst::ElementFactory::make("videoconvert");

			m_pipeline->add(src,
							jitterbufferfuck,
							depayfuck,
							avdecfuck,
							videoconvertfuck,
							m_videoSink);

			src->link(jitterbufferfuck);
			jitterbufferfuck->link(depayfuck);
			depayfuck->link(avdecfuck);
			avdecfuck->link(videoconvertfuck);
			videoconvertfuck->link(m_videoSink);

            //watch the bus for messages
            QGst::BusPtr bus = m_pipeline->bus();
            bus->addSignalWatch();
            QGlib::connect(bus, "message", this, &Player::onBusMessage);
        } else {
            qCritical() << "Failed to create the pipeline";
        }
    }
}

void Player::onBusMessage(const QGst::MessagePtr & message)
{
    switch (message->type()) {
    case QGst::MessageEos: //End of stream. We reached the end of the file.
		qCritical() << "Reached end of stream !";
        break;
    case QGst::MessageError: //Some error occurred.
		qCritical() << "Got error message : ";
        qCritical() << message.staticCast<QGst::ErrorMessage>()->error();
        break;
    default:
        break;
    }
}

#include "moc_player.cpp"
