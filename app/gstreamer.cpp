#include <json-glib/json-glib.h>

#include "gstreamer.h"

static const char* PIPELINE_DESC = R"(webrtcbin name=sendrecv bundle-policy=max-bundle
videotestsrc is-live=true pattern=ball ! videoconvert ! queue ! vp8enc deadline=1 ! rtpvp8pay !
queue ! application/x-rtp,media=video,encoding-name=VP8,payload=97 ! sendrecv.
audiotestsrc is-live=true wave=red-noise ! audioconvert ! audioresample ! queue ! opusenc ! rtpopuspay !
queue ! application/x-rtp,media=audio,encoding-name=OPUS,payload=96 ! sendrecv.)";

void onNegotiationNeeded(GstElement* obj, idk* self)
{
	self->negotiationNeeded(obj);
}

void onIceCandidate(GstElement* obj, guint mline_index, char* iceCandidate, idk* self)
{
	self->iceCandidate(obj, mline_index, iceCandidate);
}

void onPadAdded(GstElement* obj, GstPad* pad, idk* self)
{
	self->padAdded(obj, pad);
}

void onDecodeBinPadAdded(GstElement* obj, GstPad* pad, idk* self)
{
	self->decodeBinPadAdded(obj, pad);
}

idk::~idk()
{
	gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);
	gst_object_unref(pipeline);
}

idk::idk()
{
	pipeline = GST_PIPELINE(gst_parse_launch(PIPELINE_DESC, nullptr));
}

void idk::startPipeline()
{
	webrtc = gst_bin_get_by_name(GST_BIN(pipeline), "sendrecv");
	Q_ASSERT(webrtc);

	g_signal_connect(webrtc, "on-negotiation-needed", G_CALLBACK(onNegotiationNeeded), this);
	g_signal_connect(webrtc, "on-ice-candidate", G_CALLBACK(onIceCandidate), this);
	g_signal_connect(webrtc, "on-pad-added", G_CALLBACK(onPadAdded), this);

	gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);
}

void on_offer_created (GstPromise * promise, idk* self)
{
	self->offerCreated(promise);
}


void idk::negotiationNeeded(GstElement* obj)
{
	state = PEER_CALL_NEGOTIATING;

	if (false) {
		// soup_websocket_connection_send_text (ws_conn, "OFFER_REQUEST");
	} else {
		auto promise =
			gst_promise_new_with_change_func(
				(GstPromiseChangeFunc)on_offer_created, this, nullptr);

		g_signal_emit_by_name(webrtc, "create-offer", nullptr, promise);
	}
}

void idk::offerCreated(GstPromise* promise)
{
	GstWebRTCSessionDescription *offer = NULL;

	Q_ASSERT(state == PEER_CALL_NEGOTIATING);
	Q_ASSERT(gst_promise_wait(promise) == GST_PROMISE_RESULT_REPLIED);

	auto reply = gst_promise_get_reply (promise);
	gst_structure_get (reply, "offer",
		GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &offer, NULL);
	gst_promise_unref (promise);

	promise = gst_promise_new ();
	g_signal_emit_by_name (webrtc, "set-local-description", offer, promise);
	gst_promise_interrupt (promise);
	gst_promise_unref (promise);

	/* Send offer to peer */
	sendSDPToPeer(offer);
	gst_webrtc_session_description_free (offer);
}

static gchar *
get_string_from_json_object (JsonObject * object)
{
	JsonNode *root;
	JsonGenerator *generator;
	gchar *text;

	/* Make it the root node */
	root = json_node_init_object (json_node_alloc (), object);
	generator = json_generator_new ();
	json_generator_set_root (generator, root);
	text = json_generator_to_data (generator, NULL);

	/* Release everything */
	g_object_unref (generator);
	json_node_free (root);
	return text;
}

void idk::sendSDPToPeer(GstWebRTCSessionDescription* desc)
{
	Q_ASSERT(state < PEER_CALL_NEGOTIATING);

	auto text = gst_sdp_message_as_text(GST_SDP_MESSAGE(desc));
	auto sdp = json_object_new();

	switch (desc->type) {
	case GST_WEBRTC_SDP_TYPE_OFFER:
		json_object_set_string_member(sdp, "type", "offer");
		break;
	case GST_WEBRTC_SDP_TYPE_ANSWER:
		json_object_set_string_member(sdp, "type", "answer");
		break;
	default:
		Q_UNREACHABLE();
	}

	json_object_set_string_member(sdp, "sdp", text);
	g_free(text);

	auto msg = json_object_new();
	json_object_set_object_member(msg, "sdp", sdp);
	auto jsonText = get_string_from_json_object(msg);
	json_object_unref(msg);

	// TODO: send json

	g_free(text);
}

void idk::iceCandidate(GstElement* obj, guint mline_index, char* iceCandidate)
{
	Q_UNUSED(obj)

	auto ice = json_object_new();

	json_object_set_string_member(ice, "candidate", iceCandidate);
	json_object_set_int_member(ice, "sdpMLineIndex", mline_index);

	auto msg = json_object_new();
	json_object_set_object_member(msg, "ice", ice);

	auto text = get_string_from_json_object(msg);
	json_object_unref(msg);

	// TODO: do something with text

	g_free(text);
}

void idk::padAdded(GstElement* obj, GstPad* pad)
{
	if (gst_pad_get_direction(pad) != GST_PAD_SRC)
		return;

	auto decodeBin =
		gst_element_factory_make("decodebin", nullptr);

	g_signal_connect(
		decodeBin, "pad-added",
		G_CALLBACK(onDecodeBinPadAdded), this);

	gst_bin_add(GST_BIN(pipeline), decodeBin);
	gst_element_sync_state_with_parent(decodeBin);

	gst_pad_link(pad, GST_PAD(webrtc));
}

bool has_prefix(const char *str, const char *pre)
{
	return strncmp(pre, str, strlen(pre)) == 0;
}

void idk::decodeBinPadAdded(GstElement* obj, GstPad* pad)
{
	if (!gst_pad_has_current_caps(pad))
		return;

	auto caps = gst_pad_get_current_caps(pad);
	Q_ASSERT(!gst_caps_is_empty(caps));

	auto structure = gst_caps_get_structure(caps, 0);
	if (!has_prefix(g_quark_to_string(structure->name), "audio")) {
		return;
	}

	auto queue = gst_element_factory_make("queue", nullptr);
	auto conv = gst_element_factory_make("audioconvert", nullptr);
	auto resample = gst_element_factory_make("audioresample", nullptr);
	auto sink = gst_element_factory_make("autoaudiosink", nullptr);

	gst_bin_add_many(GST_BIN(pipe), queue, conv, resample, sink, nullptr);

	gst_element_sync_state_with_parent (queue);
	gst_element_sync_state_with_parent (conv);
	gst_element_sync_state_with_parent (resample);
	gst_element_sync_state_with_parent (sink);

	gst_element_link_many(queue, conv, resample, sink, nullptr);

	auto queuePad = gst_element_get_static_pad(queue, "sink");

	auto ret = gst_pad_link(pad, queuePad);
	Q_ASSERT(ret == GST_PAD_LINK_OK);
}

void gst_main(int argc, char* argv[])
{
	gst_init(&argc, &argv);

	auto pipeline =
		gst_parse_launch(
			"playbin uri=https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm",
			nullptr
		);

	gst_element_set_state(pipeline, GST_STATE_PLAYING);

	auto bus = gst_element_get_bus(pipeline);

	auto msg =
		gst_bus_timed_pop_filtered(
			bus, GST_CLOCK_TIME_NONE,
			(GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS)
		);

	if (GST_MESSAGE_TYPE (msg) == GST_MESSAGE_ERROR) {
		g_error ("An error occurred! Re-run with the GST_DEBUG=*:WARN environment "
			"variable set for more details.");
	}

	gst_message_unref(msg);
	gst_object_unref(bus);
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(pipeline);
}
