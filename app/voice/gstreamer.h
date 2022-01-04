#pragma once

#include <gst/gst.h>
#include <gst/sdp/sdp.h>
#include <gst/rtp/rtp.h>

#include <gobject/gobject.h>
#include <gobject/gsignal.h>
#include <gst/webrtc/webrtc.h>

#include <QtGlobal>
#include <QScopedPointer>

struct idk
{
	enum State
	{
		APP_STATE_UNKNOWN = 0,
		APP_STATE_ERROR = 1,          /* generic error */
		SERVER_CONNECTING = 1000,
		SERVER_CONNECTION_ERROR,
		SERVER_CONNECTED,             /* Ready to register */
		SERVER_REGISTERING = 2000,
		SERVER_REGISTRATION_ERROR,
		SERVER_REGISTERED,            /* Ready to call a peer */
		SERVER_CLOSED,                /* server connection closed by us or the server */
		PEER_CONNECTING = 3000,
		PEER_CONNECTION_ERROR,
		PEER_CONNECTED,
		PEER_CALL_NEGOTIATING = 4000,
		PEER_CALL_STARTED,
		PEER_CALL_STOPPING,
		PEER_CALL_STOPPED,
		PEER_CALL_ERROR,
	};

	GstPipeline* pipeline = nullptr;
	GstElement* webrtc = nullptr;
	State state = APP_STATE_UNKNOWN;

	idk();
	~idk();

	void startPipeline();

	void negotiationNeeded(GstElement* obj);
	void iceCandidate(GstElement* obj, guint mline_index, char* iceCandidate);
	void padAdded(GstElement* obj, GstPad* pad);
	void decodeBinPadAdded(GstElement* obj, GstPad* pad);
	void offerCreated(GstPromise* promise);
	void sendSDPToPeer(GstWebRTCSessionDescription* desc);
};

void gst_main(int argc, char* argv[]);
