#include "voice/v1/voice.hrpc.client.h"
auto VoiceServiceServiceClient::Connect(QMap<QByteArray,QString> headers) -> Receive__protocol_voice_v1_Signal__Send__protocol_voice_v1_ClientSignal__Stream*
{
auto url = QUrl(wsProtocol()+host); url.setPath(QStringLiteral("/protocol.voice.v1.VoiceService/Connect")); auto req = QNetworkRequest(url);

					for (const auto& item : headers.keys()) {
						req.setRawHeader(item, headers[item].toLocal8Bit());
					}
				
	auto sock = new Receive__protocol_voice_v1_Signal__Send__protocol_voice_v1_ClientSignal__Stream();
	sock->open(req);
	return sock;
}
