#include "voice.h"

#include <gst/gst.h>
#include <gst/sdp/sdp.h>
#include <gst/rtp/rtp.h>

#include <gobject/gobject.h>
#include <gobject/gsignal.h>
#include <gst/webrtc/webrtc.h>

#include <sdptransform/sdptransform.hpp>
#include <sdptransform/json.hpp>

#include <optional>

#include "sdp.h"

static const char* PIPELINE_DESC = R"(webrtcbin name=sendrecv bundle-policy=max-bundle
videotestsrc is-live=true pattern=ball ! videoconvert ! queue ! vp8enc deadline=1 ! rtpvp8pay !
queue ! application/x-rtp,media=video,encoding-name=VP8,payload=97 ! sendrecv.
audiotestsrc is-live=true wave=red-noise ! audioconvert ! audioresample ! queue ! opusenc ! rtpopuspay !
queue ! application/x-rtp,media=audio,encoding-name=OPUS,payload=96 ! sendrecv.)";

struct voiceCall::Private
{
	QMap<quint64, voice::UserConsumerOptions> data;
	QScopedPointer<Receive__protocol_voice_v1_StreamMessageResponse__Send__protocol_voice_v1_StreamMessageRequest__Stream> stream;

	GstPipeline* pipeline = nullptr;
	GstElement* webrtc = nullptr;

	QString host;
	quint64 guildID;
	quint64 channelID;

	voice::TransportOptions consumerOptions;
	voice::TransportOptions producerOptions;

	json localCaps;
	json remoteCaps;

	json rtpParams;
	json localSDP;
	QScopedPointer<mediasoupclient::Sdp::RemoteSdp> remoteSDP;
};

using STR = Receive__protocol_voice_v1_StreamMessageResponse__Send__protocol_voice_v1_StreamMessageRequest__Stream;

voiceCall::~voiceCall()
{

}

gboolean
newBusMessage(GstBus *bus G_GNUC_UNUSED, GstMessage *msg, gpointer user_data)
{
	switch (GST_MESSAGE_TYPE(msg)) {
	case GST_MESSAGE_EOS:
		qWarning() << "end of stream";
		break;
	case GST_MESSAGE_ERROR:
		GError *error;
		gchar *debug;
		gst_message_parse_error(msg, &error, &debug);
		qWarning() << "error from element" << GST_OBJECT_NAME(msg->src) << ":" << error->message << debug;
		g_clear_error(&error);
		g_free(debug);
		break;
	default:
		break;
	}
	return TRUE;
}

bool
voiceCall::createPipeline(int opusPayloadType)
{
	GstElement* source = gst_element_factory_make("autoaudiosrc", nullptr);
	GstElement* volume = gst_element_factory_make("volume", "srclevel");
	GstElement* convert = gst_element_factory_make("audioconvert", nullptr);
	GstElement* resample = gst_element_factory_make("audioresample", nullptr);
	GstElement* queue1 = gst_element_factory_make("queue", nullptr);
	GstElement* opusenc = gst_element_factory_make("opusenc", nullptr);
	GstElement* rtp = gst_element_factory_make("rtpopuspay", nullptr);
	GstElement* queue2 = gst_element_factory_make("queue", nullptr);
	GstElement* capsfilter = gst_element_factory_make("capsfilter", nullptr);

	GstCaps *rtpcaps = gst_caps_new_simple(
		"application/x-rtp", "media",
		G_TYPE_STRING, "audio",
		"encoding-name", G_TYPE_STRING,
		"OPUS", "payload",
		G_TYPE_INT, opusPayloadType, nullptr);

	g_object_set(capsfilter, "caps", rtpcaps, nullptr);
	gst_caps_unref(rtpcaps);

	GstElement *webrtcbin = gst_element_factory_make("webrtcbin", "webrtcbin");
	g_object_set(webrtcbin, "bundle-policy", GST_WEBRTC_BUNDLE_POLICY_MAX_BUNDLE, nullptr);

	d->pipeline = GST_PIPELINE(gst_pipeline_new(nullptr));

	GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(d->pipeline));
	gst_bus_add_watch(bus, newBusMessage, this);

	gst_bin_add_many(
		GST_BIN(d->pipeline),
		source, volume, convert,
		resample, queue1, opusenc,
		rtp, queue2, capsfilter,
		webrtcbin, nullptr);

	if (!gst_element_link_many(
		source, volume, convert,
		resample, queue1, opusenc,
		rtp, queue2, capsfilter,
		webrtcbin, nullptr)) {

		qWarning() << "failed to link many";
		return false;
	}

	return true;
}

void
voiceCall::startSignalling()
{
	s->api()->clientForHomeserver(d->host).then([=](SDK::Client* client)
{

	d->stream.reset(client->StreamMessage());

	QObject::connect(d->stream.get(), &STR::receivedMessage, d->stream.get(), [=](voice::StreamMessageResponse r) {
		switch (r.message_case()) {
		case voice::StreamMessageResponse::MessageCase::kInitialized: {
			initialised(QString::fromStdString(r.initialized().rtp_capabilities()));
			break;
		}
		case voice::StreamMessageResponse::MessageCase::kPreparedForJoinChannel: {
			preparedForJoinChannel(r.prepared_for_join_channel().consumer_transport_options(), r.prepared_for_join_channel().producer_transport_options());
			break;
		}
		case voice::StreamMessageResponse::MessageCase::kJoinedChannel: {
			QList<voice::UserConsumerOptions> data;

			const auto& other = r.joined_channel().other_users();
			for (const auto& it : other) {
				data << it;
			}
			joinedChannel(data);
			break;
		}
		case voice::StreamMessageResponse::MessageCase::kUserJoined: {
			userJoined(r.user_joined().data());
			break;
		}
		case voice::StreamMessageResponse::MessageCase::kUserLeft: {
			userLeft(r.user_left().user_id());
			break;
		}
		default:
			;
		}
	});

	connect(d->stream.get(), &QWebSocket::connected, this, [=]()
{
	auto req = voice::StreamMessageRequest{};

	req.set_allocated_initialize(new voice::StreamMessageRequest::Initialize);
	req.mutable_initialize()->set_guild_id(d->guildID);
	req.mutable_initialize()->set_channel_id(d->channelID);

	qWarning() << "===" << "Initializing...";

	qWarning() << d->stream->send(req) << d->stream->state();
});

});
}

voiceCall::voiceCall(State* state, const QString& host, quint64 guildID, quint64 channelID) : QObject(state), d(new Private), s(state)
{
	d->host = host;
	d->guildID = guildID;
	d->channelID = channelID;

	qWarning() << "=== Calling..." << host << guildID << channelID;

	createPipeline(111);

	d->pipeline = GST_PIPELINE(gst_parse_launch(PIPELINE_DESC, nullptr));
	auto ret = gst_element_set_state(GST_ELEMENT(d->pipeline), GST_STATE_PLAYING);
	if ( ret == GST_STATE_CHANGE_FAILURE ) {
		qWarning() << "failed to start pipeline";
	}
	d->webrtc = gst_bin_get_by_name(GST_BIN(d->pipeline), "sendrecv");

	startSignalling();

}

void voiceCall::initialised(QString rtpCapabilities)
{
	auto caps = json::parse(rtpCapabilities.toStdString());
	for (auto& codec : caps["codecs"]) {
		codec["payloadType"] = codec["preferredPayloadType"];
	}
	for (auto& ext : caps["headerExtensions"]) {
		ext["id"] = ext["preferredId"];
		ext["encrypt"] = ext["preferredEncrypt"];
	}
	auto req = voice::StreamMessageRequest{};

	req.set_allocated_prepare_for_join_channel(new voice::StreamMessageRequest::PrepareForJoinChannel);
	req.mutable_prepare_for_join_channel()->set_rtp_capabilities(caps.dump());

	d->rtpParams = caps;

	d->localCaps = caps;
	d->remoteCaps = caps;

	Q_UNUSED(d->stream->send(req));
}

/*

relevant excerpt from the protobuf docs:

think it's helpful.

/ Object containing all the necessary information about transport options required
/ from the server to establish transport connection on the client
/
/ This corresponds to https://mediasoup.org/documentation/v3/mediasoup-client/api/#TransportOptions on client:
/ - `id` -> `id`
/ - `ice_parameters` -> `iceParameters`
/ - `dtls_parameters` -> `dtlsParameters`
/ - `ice_candidates` -> `iceCandidates`
message TransportOptions {
	/ The transport ID.
	string id = 1;
	/ DTLS paramaters in JSON. Corresponds to `DtlsParameters` in mediasoup's TypeScript API.
	string dtls_parameters = 2;
	/ ICE candidates in JSON. Corresponds to `IceCandidate` in mediasoup's TypeScript API.
	repeated string ice_candidates = 3;
	/ ICE paramaters in JSON. Corresponds to `IceParameters` in mediasoup's TypeScript API.
	string ice_parameters = 4;
}

*/

// janet notes 11.11.21
//
// went to le epic gstreamer irc. guy was helpful.
//
// BIG things:
// convert signalling to and from SDPs
//		 g_signal_emit_by_name (object, "create-offer", options, promise);
//
// convert TransportOptions to an SDP.
//
// two connections?
// idk.
//
// i think our sending goes to the peer connection created from producer
// and our receiving comes from the connection created from consumer.
//
// so all incoming tracks -> consumer
// one outgoing track -> producer
//
// will have to report in tomorrow.
//
// consumer: a receive track
// producer: a send track

void offerCreated_(GstPromise *promise, gpointer self)
{
	((voiceCall*)(self))->offerCreated(promise);
}


// Static functions declaration.
static bool isRtxCodec(const json& codec);
static bool matchCodecs(json& aCodec, json& bCodec, bool strict = false, bool modify = false);
static bool matchHeaderExtensions(const json& aExt, const json& bExt);
static json reduceRtcpFeedback(const json& codecA, const json& codecB);
static uint8_t getH264PacketizationMode(const json& codec);
static uint8_t getH264LevelAssimetryAllowed(const json& codec);
static std::string getH264ProfileLevelId(const json& codec);
static std::string getVP9ProfileId(const json& codec);

static json reduceRtcpFeedback(const json& codecA, const json& codecB)
{
	auto reducedRtcpFeedback = json::array();
	auto rtcpFeedbackAIt     = codecA.find("rtcpFeedback");
	auto rtcpFeedbackBIt     = codecB.find("rtcpFeedback");

	for (const auto& aFb : *rtcpFeedbackAIt)
	{
		auto rtcpFeedbackIt =
		  std::find_if(rtcpFeedbackBIt->begin(), rtcpFeedbackBIt->end(), [&aFb](const json& bFb) {
			  return (aFb["type"] == bFb["type"] && aFb["parameter"] == bFb["parameter"]);
		  });

		if (rtcpFeedbackIt != rtcpFeedbackBIt->end())
			reducedRtcpFeedback.push_back(*rtcpFeedbackIt);
	}

	return reducedRtcpFeedback;
}

static uint8_t getH264PacketizationMode(const json& codec)
{
	auto& parameters         = codec["parameters"];
	auto packetizationModeIt = parameters.find("packetization-mode");

	// clang-format off
	if (
		packetizationModeIt == parameters.end() ||
		!packetizationModeIt->is_number_integer()
	)
	// clang-format on
	{
		return 0;
	}

	return packetizationModeIt->get<uint8_t>();
}

static uint8_t getH264LevelAssimetryAllowed(const json& codec)
{
	const auto& parameters       = codec["parameters"];
	auto levelAssimetryAllowedIt = parameters.find("level-asymmetry-allowed");

	// clang-format off
	if (
		levelAssimetryAllowedIt == parameters.end() ||
		!levelAssimetryAllowedIt->is_number_integer()
	)
	// clang-format on
	{
		return 0;
	}

	return levelAssimetryAllowedIt->get<uint8_t>();
}

static std::string getH264ProfileLevelId(const json& codec)
{
	const auto& parameters = codec["parameters"];
	auto profileLevelIdIt  = parameters.find("profile-level-id");

	if (profileLevelIdIt == parameters.end())
		return "";
	else if (profileLevelIdIt->is_number())
		return std::to_string(profileLevelIdIt->get<int32_t>());
	else
		return profileLevelIdIt->get<std::string>();
}

static std::string getVP9ProfileId(const json& codec)
{
	const auto& parameters = codec["parameters"];
	auto profileIdIt       = parameters.find("profile-id");

	if (profileIdIt == parameters.end())
		return "0";

	if (profileIdIt->is_number())
		return std::to_string(profileIdIt->get<int32_t>());
	else
		return profileIdIt->get<std::string>();
}

json getSendingRtpParameters(const std::string& kind, const json& extendedRtpCapabilities)
{
	// clang-format off
	json rtpParameters =
	{
		{ "mid",							nullptr				},
		{ "codecs",					 json::array()	},
		{ "headerExtensions", json::array()	},
		{ "encodings",				json::array()	},
		{ "rtcp",						 json::object() }
	};
	// clang-format on

	for (const auto& extendedCodec : extendedRtpCapabilities["codecs"])
	{
		if (kind != extendedCodec["kind"].get<std::string>())
			continue;

		// clang-format off
		json codec =
		{
			{ "mimeType",		 extendedCodec["mimeType"]				 },
			{ "payloadType",	extendedCodec["localPayloadType"] },
			{ "clockRate",		extendedCodec["clockRate"]				},
			{ "parameters",	 extendedCodec["localParameters"]	},
			{ "rtcpFeedback", extendedCodec["rtcpFeedback"]		 }
		};
		// clang-format on

		if (extendedCodec.contains("channels"))
			codec["channels"] = extendedCodec["channels"];

		rtpParameters["codecs"].push_back(codec);

		// Add RTX codec.
		if (extendedCodec["localRtxPayloadType"] != nullptr)
		{
			auto mimeType = extendedCodec["kind"].get<std::string>().append("/rtx");

			// clang-format off
			json rtxCodec =
			{
				{ "mimeType",		mimeType														 },
				{ "payloadType", extendedCodec["localRtxPayloadType"] },
				{ "clockRate",	 extendedCodec["clockRate"]					 },
				{
					"parameters",
					{
						{ "apt", extendedCodec["localPayloadType"].get<uint8_t>() }
					}
				},
				{ "rtcpFeedback", json::array() }
			};
			// clang-format on

			rtpParameters["codecs"].push_back(rtxCodec);
		}

		// NOTE: We assume a single media codec plus an optional RTX codec.
		break;
	}

	for (const auto& extendedExtension : extendedRtpCapabilities["headerExtensions"])
	{
		if (kind != extendedExtension["kind"].get<std::string>())
			continue;

		std::string direction = extendedExtension["direction"].get<std::string>();

		// Ignore RTP extensions not valid for sending.
		if (direction != "sendrecv" && direction != "sendonly")
			continue;

		// clang-format off
		json ext =
		{
			{ "uri",				extendedExtension["uri"]		 },
			{ "id",				 extendedExtension["sendId"]	},
			{ "encrypt",		extendedExtension["encrypt"] },
			{ "parameters", json::object()							 }
		};
		// clang-format on

		rtpParameters["headerExtensions"].push_back(ext);
	}

	return rtpParameters;
}

/**
	* Generate RTP parameters of the given kind for sending media.
	*/
json getSendingRemoteRtpParameters(const std::string& kind, const json& extendedRtpCapabilities)
{
	// clang-format off
	json rtpParameters =
	{
		{ "mid",							nullptr				},
		{ "codecs",					 json::array()	},
		{ "headerExtensions", json::array()	},
		{ "encodings",				json::array()	},
		{ "rtcp",						 json::object() }
	};
	// clang-format on

	for (const auto& extendedCodec : extendedRtpCapabilities["codecs"])
	{
		if (kind != extendedCodec["kind"].get<std::string>())
			continue;

		// clang-format off
		json codec =
		{
			{ "mimeType",		 extendedCodec["mimeType"]				 },
			{ "payloadType",	extendedCodec["localPayloadType"] },
			{ "clockRate",		extendedCodec["clockRate"]				},
			{ "parameters",	 extendedCodec["remoteParameters"] },
			{ "rtcpFeedback", extendedCodec["rtcpFeedback"]		 }
		};
		// clang-format on

		if (extendedCodec.contains("channels"))
			codec["channels"] = extendedCodec["channels"];

		rtpParameters["codecs"].push_back(codec);

		// Add RTX codec.
		if (extendedCodec["localRtxPayloadType"] != nullptr)
		{
			auto mimeType = extendedCodec["kind"].get<std::string>().append("/rtx");

			// clang-format off
			json rtxCodec =
			{
				{ "mimeType",		mimeType														 },
				{ "payloadType", extendedCodec["localRtxPayloadType"] },
				{ "clockRate",	 extendedCodec["clockRate"]					 },
				{
					"parameters",
					{
						{ "apt", extendedCodec["localPayloadType"].get<uint8_t>() }
					}
				},
				{ "rtcpFeedback", json::array() }
			};
			// clang-format on

			rtpParameters["codecs"].push_back(rtxCodec);
		}

		// NOTE: We assume a single media codec plus an optional RTX codec.
		break;
	}

	for (const auto& extendedExtension : extendedRtpCapabilities["headerExtensions"])
	{
		if (kind != extendedExtension["kind"].get<std::string>())
			continue;

		std::string direction = extendedExtension["direction"].get<std::string>();

		// Ignore RTP extensions not valid for sending.
		if (direction != "sendrecv" && direction != "sendonly")
			continue;

		// clang-format off
		json ext =
		{
			{ "uri",				extendedExtension["uri"]		 },
			{ "id",				 extendedExtension["sendId"]	},
			{ "encrypt",		extendedExtension["encrypt"] },
			{ "parameters", json::object()							 }
		};
		// clang-format on

		rtpParameters["headerExtensions"].push_back(ext);
	}

	auto headerExtensionsIt = rtpParameters.find("headerExtensions");

	// Reduce codecs' RTCP feedback. Use Transport-CC if available, REMB otherwise.
	auto headerExtensionIt =
		std::find_if(headerExtensionsIt->begin(), headerExtensionsIt->end(), [](json& ext) {
			return ext["uri"].get<std::string>() ==
					"http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01";
		});

	if (headerExtensionIt != headerExtensionsIt->end())
	{
		for (auto& codec : rtpParameters["codecs"])
		{
			auto& rtcpFeedback = codec["rtcpFeedback"];

			for (auto it = rtcpFeedback.begin(); it != rtcpFeedback.end();)
			{
				auto& fb	= *it;
				auto type = fb["type"].get<std::string>();

				if (type == "goog-remb")
					it = rtcpFeedback.erase(it);
				else
					++it;
			}
		}

		return rtpParameters;
	}

	headerExtensionIt =
		std::find_if(headerExtensionsIt->begin(), headerExtensionsIt->end(), [](json& ext) {
			return ext["uri"].get<std::string>() ==
					"http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time";
		});

	if (headerExtensionIt != headerExtensionsIt->end())
	{
		for (auto& codec : rtpParameters["codecs"])
		{
			auto& rtcpFeedback = codec["rtcpFeedback"];

			for (auto it = rtcpFeedback.begin(); it != rtcpFeedback.end();)
			{
				auto& fb	= *it;
				auto type = fb["type"].get<std::string>();

				if (type == "transport-cc")
					it = rtcpFeedback.erase(it);
				else
					++it;
			}
		}

		return rtpParameters;
	}

	for (auto& codec : rtpParameters["codecs"])
	{
		auto& rtcpFeedback = codec["rtcpFeedback"];

		for (auto it = rtcpFeedback.begin(); it != rtcpFeedback.end();)
		{
			auto& fb	= *it;
			auto type = fb["type"].get<std::string>();

			if (type == "transport-cc" || type == "goog-remb")
				it = rtcpFeedback.erase(it);
			else
				++it;
		}
	}

	return rtpParameters;
}

static bool matchHeaderExtensions(const json& aExt, const json& bExt)
{
	if (aExt["kind"] != bExt["kind"])
		return false;

	return aExt["uri"] == bExt["uri"];
}


static bool isRtxCodec(const json& codec)
{
	static const std::regex RtxMimeTypeRegex("^(audio|video)/rtx$", std::regex_constants::ECMAScript | std::regex_constants::icase);

	std::smatch match;
	auto mimeType = codec["mimeType"].get<std::string>();

	return std::regex_match(mimeType, match, RtxMimeTypeRegex);
}

const char kProfileLevelId[] = "profile-level-id";
const char kLevelAsymmetryAllowed[] = "level-asymmetry-allowed";

enum Profile {
	kProfileConstrainedBaseline,
	kProfileBaseline,
	kProfileMain,
	kProfileConstrainedHigh,
	kProfileHigh,
};


// All values are equal to ten times the level number, except level 1b which is
// special.
enum Level {
	kLevel1_b = 0,
	kLevel1 = 10,
	kLevel1_1 = 11,
	kLevel1_2 = 12,
	kLevel1_3 = 13,
	kLevel2 = 20,
	kLevel2_1 = 21,
	kLevel2_2 = 22,
	kLevel3 = 30,
	kLevel3_1 = 31,
	kLevel3_2 = 32,
	kLevel4 = 40,
	kLevel4_1 = 41,
	kLevel4_2 = 42,
	kLevel5 = 50,
	kLevel5_1 = 51,
	kLevel5_2 = 52
};

const uint8_t kConstraintSet3Flag = 0x10;

constexpr uint8_t ByteMaskString(char c, const char (&str)[9]) {
	return (str[0] == c) << 7 | (str[1] == c) << 6 | (str[2] == c) << 5 |
			(str[3] == c) << 4 | (str[4] == c) << 3 | (str[5] == c) << 2 |
			(str[6] == c) << 1 | (str[7] == c) << 0;
}


class BitPattern {
 public:
	explicit constexpr BitPattern(const char (&str)[9])
			: mask_(~ByteMaskString('x', str)),
				masked_value_(ByteMaskString('1', str)) {}

	bool IsMatch(uint8_t value) const { return masked_value_ == (value & mask_); }

 private:
	const uint8_t mask_;
	const uint8_t masked_value_;
};

struct ProfilePattern {
	const uint8_t profile_idc;
	const BitPattern profile_iop;
	const Profile profile;
};


// This is from https://tools.ietf.org/html/rfc6184#section-8.1.
constexpr ProfilePattern kProfilePatterns[] = {
	{0x42, BitPattern("x1xx0000"), kProfileConstrainedBaseline},
	{0x4D, BitPattern("1xxx0000"), kProfileConstrainedBaseline},
	{0x58, BitPattern("11xx0000"), kProfileConstrainedBaseline},
	{0x42, BitPattern("x0xx0000"), kProfileBaseline},
	{0x58, BitPattern("10xx0000"), kProfileBaseline},
	{0x4D, BitPattern("0x0x0000"), kProfileMain},
	{0x64, BitPattern("00000000"), kProfileHigh},
	{0x64, BitPattern("00001100"), kProfileConstrainedHigh}};

struct ProfileLevelId {
	constexpr ProfileLevelId(Profile profile, Level level)
		: profile(profile), level(level) {}
	Profile profile;
	Level level;
};

std::optional<ProfileLevelId> ParseProfileLevelId(const char* str) {
	// The string should consist of 3 bytes in hexadecimal format.
	if (strlen(str) != 6u)
		return {};
	const uint32_t profile_level_id_numeric = strtol(str, nullptr, 16);
	if (profile_level_id_numeric == 0)
		return {};

	// Separate into three bytes.
	const uint8_t level_idc =
			static_cast<uint8_t>(profile_level_id_numeric & 0xFF);
	const uint8_t profile_iop =
			static_cast<uint8_t>((profile_level_id_numeric >> 8) & 0xFF);
	const uint8_t profile_idc =
			static_cast<uint8_t>((profile_level_id_numeric >> 16) & 0xFF);

	// Parse level based on level_idc and constraint set 3 flag.
	Level level;
	switch (level_idc) {
		case kLevel1_1:
			level = (profile_iop & kConstraintSet3Flag) != 0 ? kLevel1_b : kLevel1_1;
			break;
		case kLevel1:
		case kLevel1_2:
		case kLevel1_3:
		case kLevel2:
		case kLevel2_1:
		case kLevel2_2:
		case kLevel3:
		case kLevel3_1:
		case kLevel3_2:
		case kLevel4:
		case kLevel4_1:
		case kLevel4_2:
		case kLevel5:
		case kLevel5_1:
		case kLevel5_2:
			level = static_cast<Level>(level_idc);
			break;
		default:
			// Unrecognized level_idc.
			return {};
	}

	// Parse profile_idc/profile_iop into a Profile enum.
	for (const ProfilePattern& pattern : kProfilePatterns) {
		if (profile_idc == pattern.profile_idc &&
				pattern.profile_iop.IsMatch(profile_iop)) {
			return ProfileLevelId(pattern.profile, level);
		}
	}

	// Unrecognized profile_idc/profile_iop combination.
	return {};
}


std::optional<ProfileLevelId> ParseSdpProfileLevelId(
		const std::map<std::string,std::string>& params) {
	// TODO(magjed): The default should really be kProfileBaseline and kLevel1
	// according to the spec: https://tools.ietf.org/html/rfc6184#section-8.1. In
	// order to not break backwards compatibility with older versions of WebRTC
	// where external codecs don't have any parameters, use
	// kProfileConstrainedBaseline kLevel3_1 instead. This workaround will only be
	// done in an interim period to allow external clients to update their code.
	// http://crbug/webrtc/6337.
	static const ProfileLevelId kDefaultProfileLevelId(
		kProfileConstrainedBaseline, kLevel3_1);

	const auto profile_level_id_it = params.find(kProfileLevelId);
	return (profile_level_id_it == params.end())
		? kDefaultProfileLevelId
		: ParseProfileLevelId(profile_level_id_it->second.c_str());
}


static bool IsSameH264Profile(
	const std::map<std::string, std::string>& params1,
	const std::map<std::string, std::string>& params2)
{
	// TODO: complete this
	const std::optional<ProfileLevelId> profile_level_id =
		ParseSdpProfileLevelId(params1);
	const std::optional<ProfileLevelId> other_profile_level_id =
		ParseSdpProfileLevelId(params2);
	// Compare H264 profiles, but not levels.
	return profile_level_id && other_profile_level_id &&
			profile_level_id->profile == other_profile_level_id->profile;
}

// Compare H264 levels and handle the level 1b case.
bool IsLess(Level a, Level b) {
	if (a == kLevel1_b)
		return b != kLevel1 && b != kLevel1_b;
	if (b == kLevel1_b)
		return a == kLevel1;
	return a < b;
}

Level Min(Level a, Level b) {
	return IsLess(a, b) ? a : b;
}

bool IsLevelAsymmetryAllowed(const std::map<std::string,std::string>& params) {
	const auto it = params.find(kLevelAsymmetryAllowed);
	return it != params.end() && strcmp(it->second.c_str(), "1") == 0;
}

std::optional<std::string> ProfileLevelIdToString(
	const ProfileLevelId& profile_level_id) {
	// Handle special case level == 1b.
	if (profile_level_id.level == kLevel1_b) {
		switch (profile_level_id.profile) {
		case kProfileConstrainedBaseline:
			return {"42f00b"};
		case kProfileBaseline:
			return {"42100b"};
		case kProfileMain:
			return {"4d100b"};
		// Level 1b is not allowed for other profiles.
		default:
			return {};
		}
	}

	const char* profile_idc_iop_string;
	switch (profile_level_id.profile) {
		case kProfileConstrainedBaseline:
		profile_idc_iop_string = "42e0";
		break;
		case kProfileBaseline:
		profile_idc_iop_string = "4200";
		break;
		case kProfileMain:
		profile_idc_iop_string = "4d00";
		break;
		case kProfileConstrainedHigh:
		profile_idc_iop_string = "640c";
		break;
		case kProfileHigh:
		profile_idc_iop_string = "6400";
		break;
		// Unrecognized profile.
		default:
		return {};
	}

	char str[7];
	snprintf(str, 7u, "%s%02x", profile_idc_iop_string, profile_level_id.level);
	return {str};
}

void GenerateProfileLevelIdForAnswer(
	const std::map<std::string, std::string>& local_supported_params,
	const std::map<std::string, std::string>& remote_offered_params,
	std::map<std::string, std::string>* answer_params
) {
	// If both local and remote haven't set profile-level-id, they are both using
	// the default profile. In this case, don't set profile-level-id in answer
	// either.
	if (!local_supported_params.count(kProfileLevelId) &&
		!remote_offered_params.count(kProfileLevelId)) {
		return;
	}

	// Parse profile-level-ids.
	const std::optional<ProfileLevelId> local_profile_level_id =
		ParseSdpProfileLevelId(local_supported_params);
	const std::optional<ProfileLevelId> remote_profile_level_id =
		ParseSdpProfileLevelId(remote_offered_params);
	// The local and remote codec must have valid and equal H264 Profiles.
	Q_ASSERT(local_profile_level_id);
	Q_ASSERT(remote_profile_level_id);
	Q_ASSERT(local_profile_level_id->profile == remote_profile_level_id->profile);

	// Parse level information.
	const bool level_asymmetry_allowed =
		IsLevelAsymmetryAllowed(local_supported_params) &&
		IsLevelAsymmetryAllowed(remote_offered_params);
	const Level local_level = local_profile_level_id->level;
	const Level remote_level = remote_profile_level_id->level;
	const Level min_level = Min(local_level, remote_level);

	// Determine answer level. When level asymmetry is not allowed, level upgrade
	// is not allowed, i.e., the level in the answer must be equal to or lower
	// than the level in the offer.
	const Level answer_level = level_asymmetry_allowed ? local_level : min_level;

	// Set the resulting profile-level-id in the answer parameters.
	(*answer_params)[kProfileLevelId] = *ProfileLevelIdToString(
		ProfileLevelId(local_profile_level_id->profile, answer_level));
}

static bool matchCodecs(json& aCodec, json& bCodec, bool strict, bool modify)
{
	auto aMimeTypeIt = aCodec.find("mimeType");
	auto bMimeTypeIt = bCodec.find("mimeType");
	auto aMimeType = aMimeTypeIt->get<std::string>();
	auto bMimeType = bMimeTypeIt->get<std::string>();

	std::transform(aMimeType.begin(), aMimeType.end(), aMimeType.begin(), ::tolower);
	std::transform(bMimeType.begin(), bMimeType.end(), bMimeType.begin(), ::tolower);

	if (aMimeType != bMimeType)
		return false;

	if (aCodec["clockRate"] != bCodec["clockRate"])
		return false;

	if (aCodec.contains("channels") != bCodec.contains("channels"))
		return false;

	if (aCodec.contains("channels") && aCodec["channels"] != bCodec["channels"])
		return false;

	// Match H264 parameters.
	if (aMimeType == "video/h264")
	{
		auto aPacketizationMode = getH264PacketizationMode(aCodec);
		auto bPacketizationMode = getH264PacketizationMode(bCodec);

		if (aPacketizationMode != bPacketizationMode)
			return false;

		// If strict matching check profile-level-id.
		if (strict)
		{
			std::map<std::string, std::string> aParameters;
			std::map<std::string, std::string> bParameters;

			aParameters["level-asymmetry-allowed"] = std::to_string(getH264LevelAssimetryAllowed(aCodec));
			aParameters["packetization-mode"]			= std::to_string(aPacketizationMode);
			aParameters["profile-level-id"]				= getH264ProfileLevelId(aCodec);
			bParameters["level-asymmetry-allowed"] = std::to_string(getH264LevelAssimetryAllowed(bCodec));
			bParameters["packetization-mode"]			= std::to_string(bPacketizationMode);
			bParameters["profile-level-id"]				= getH264ProfileLevelId(bCodec);

			if (!IsSameH264Profile(aParameters, bParameters))
				return false;

			std::map<std::string, std::string> newParameters;

			try
			{
				GenerateProfileLevelIdForAnswer(aParameters, bParameters, &newParameters);
			}
			catch (std::runtime_error)
			{
				return false;
			}

			if (modify)
			{
				auto profileLevelIdIt = newParameters.find("profile-level-id");

				if (profileLevelIdIt != newParameters.end())
				{
					aCodec["parameters"]["profile-level-id"] = profileLevelIdIt->second;
					bCodec["parameters"]["profile-level-id"] = profileLevelIdIt->second;
				}
				else
				{
					aCodec["parameters"].erase("profile-level-id");
					bCodec["parameters"].erase("profile-level-id");
				}
			}
		}
	}
	// Match VP9 parameters.
	else if (aMimeType == "video/vp9")
	{
		// If strict matching check profile-id.
		if (strict)
		{
			auto aProfileId = getVP9ProfileId(aCodec);
			auto bProfileId = getVP9ProfileId(bCodec);

			if (aProfileId != bProfileId)
				return false;
		}
	}

	return true;
}

/**
* Generate extended RTP capabilities for sending and receiving.
*/
json getExtendedRtpCapabilities(json& localCaps, json& remoteCaps)
{
	// validateRtpCapabilities(localCaps);
	// validateRtpCapabilities(remoteCaps);

	static const std::regex MimeTypeRegex(
		"^(audio|video)/(.+)", std::regex_constants::ECMAScript | std::regex_constants::icase);

	// clang-format off
	json extendedRtpCapabilities =
	{
		{ "codecs", json::array() },
		{ "headerExtensions", json::array() }
	};
	// clang-format on

	// Match media codecs and keep the order preferred by remoteCaps.
	auto remoteCapsCodecsIt = remoteCaps.find("codecs");

	for (auto& remoteCodec : *remoteCapsCodecsIt)
	{
		if (isRtxCodec(remoteCodec))
			continue;

		json& localCodecs = localCaps["codecs"];

		auto matchingLocalCodecIt =
			std::find_if(localCodecs.begin(), localCodecs.end(), [&remoteCodec](json& localCodec) {
				return matchCodecs(localCodec, remoteCodec, /*strict*/ true, /*modify*/ true);
			});

		if (matchingLocalCodecIt == localCodecs.end())
			continue;

		auto& matchingLocalCodec = *matchingLocalCodecIt;

		// clang-format off
		json extendedCodec =
		{
			{ "mimeType",						 matchingLocalCodec["mimeType"]											},
			{ "kind",								 matchingLocalCodec["kind"]													},
			{ "clockRate",						matchingLocalCodec["clockRate"]										 },
			{ "localPayloadType",		 matchingLocalCodec["preferredPayloadType"]					},
			{ "localRtxPayloadType",	nullptr																						 },
			{ "remotePayloadType",		remoteCodec["preferredPayloadType"]								 },
			{ "remoteRtxPayloadType", nullptr																						 },
			{ "localParameters",			matchingLocalCodec["parameters"]										},
			{ "remoteParameters",		 remoteCodec["parameters"]													 },
			{ "rtcpFeedback",				 reduceRtcpFeedback(matchingLocalCodec, remoteCodec) }
		};
		// clang-format on

		if (matchingLocalCodec.contains("channels"))
			extendedCodec["channels"] = matchingLocalCodec["channels"];

		extendedRtpCapabilities["codecs"].push_back(extendedCodec);
	}

	// Match RTX codecs.
	json& extendedCodecs = extendedRtpCapabilities["codecs"];

	for (json& extendedCodec : extendedCodecs)
	{
		auto& localCodecs = localCaps["codecs"];
		auto localCodecIt = std::find_if(
			localCodecs.begin(), localCodecs.end(), [&extendedCodec](const json& localCodec) {
				return isRtxCodec(localCodec) &&
						localCodec["parameters"]["apt"] == extendedCodec["localPayloadType"];
			});

		if (localCodecIt == localCodecs.end())
			continue;

		auto& matchingLocalRtxCodec = *localCodecIt;
		auto& remoteCodecs					= remoteCaps["codecs"];
		auto remoteCodecIt					= std::find_if(
	remoteCodecs.begin(), remoteCodecs.end(), [&extendedCodec](const json& remoteCodec) {
	return isRtxCodec(remoteCodec) &&
			remoteCodec["parameters"]["apt"] == extendedCodec["remotePayloadType"];
	});

		if (remoteCodecIt == remoteCodecs.end())
			continue;

		auto& matchingRemoteRtxCodec = *remoteCodecIt;

		extendedCodec["localRtxPayloadType"]	= matchingLocalRtxCodec["preferredPayloadType"];
		extendedCodec["remoteRtxPayloadType"] = matchingRemoteRtxCodec["preferredPayloadType"];
	}

	// Match header extensions.
	auto& remoteExts = remoteCaps["headerExtensions"];

	for (auto& remoteExt : remoteExts)
	{
		auto& localExts = localCaps["headerExtensions"];
		auto localExtIt =
			std::find_if(localExts.begin(), localExts.end(), [&remoteExt](const json& localExt) {
				return matchHeaderExtensions(localExt, remoteExt);
			});

		if (localExtIt == localExts.end())
			continue;

		auto& matchingLocalExt = *localExtIt;

		// TODO: Must do stuff for encrypted extensions.

		// clang-format off
		json extendedExt =
		{
			{ "kind",		remoteExt["kind"]										},
			{ "uri",		 remoteExt["uri"]										 },
			{ "sendId",	matchingLocalExt["preferredId"]			},
			{ "recvId",	remoteExt["preferredId"]						 },
			{ "encrypt", matchingLocalExt["preferredEncrypt"] }
		};
		// clang-format on

		auto remoteExtDirection = remoteExt["direction"].get<std::string>();

		if (remoteExtDirection == "sendrecv")
			extendedExt["direction"] = "sendrecv";
		else if (remoteExtDirection == "recvonly")
			extendedExt["direction"] = "sendonly";
		else if (remoteExtDirection == "sendonly")
			extendedExt["direction"] = "recvonly";
		else if (remoteExtDirection == "inactive")
			extendedExt["direction"] = "inactive";

		extendedRtpCapabilities["headerExtensions"].push_back(extendedExt);
	}

	return extendedRtpCapabilities;
}

GstWebRTCSessionDescription *
parseSDP(const std::string &sdp, GstWebRTCSDPType type)
{
	GstSDPMessage *msg;
	gst_sdp_message_new(&msg);
	if (gst_sdp_message_parse_buffer((guint8 *)sdp.c_str(), sdp.size(), msg) == GST_SDP_OK) {
		return gst_webrtc_session_description_new(type, msg);
	} else {
		gst_sdp_message_free(msg);
		return nullptr;
	}
}

void remoteDescriptionCreated_(GstPromise *promise, void* self)
{
	((voiceCall*)(self))->remoteDescriptionCreated(promise);
}

void voiceCall::remoteDescriptionCreated(GstPromise* promise)
{
	gst_promise_unref(promise);

	auto req = voice::StreamMessageRequest{};

	req.set_allocated_join_channel(new voice::StreamMessageRequest::JoinChannel);
	req.mutable_join_channel()->set_rtp_paramaters(d->rtpParams.dump());
	req.mutable_join_channel()->set_producer_dtls_paramaters(d->producerOptions.dtls_parameters());
	req.mutable_join_channel()->set_consumer_dtls_paramaters(d->consumerOptions.dtls_parameters());

	qWarning() << "===" << "sending new thing" << d->stream->send(req);
}

void voiceCall::offerCreated(GstPromise* promise)
{
	qWarning() << "===" << "Received offer!";

	const GstStructure *reply = gst_promise_get_reply(promise);
	gboolean isAnswer = gst_structure_id_has_field(reply, g_quark_from_string("answer"));
	auto dump = gst_structure_to_string(reply);
	qWarning() << dump;
	g_free(dump);

	GstWebRTCSessionDescription *gstsdp = nullptr;
	gst_structure_get(reply, isAnswer ? "answer" : "offer", GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &gstsdp, nullptr);
	gst_promise_unref(promise);
	g_signal_emit_by_name(d->webrtc, "set-local-description", gstsdp, nullptr);

	gchar *sdp = gst_sdp_message_as_text(gstsdp->sdp);
	const auto localSDP = QString::fromLocal8Bit(sdp);
	qWarning() << "===" << "got sdp" << localSDP;
	d->localSDP = sdptransform::parse(localSDP.toStdString());
	g_free(sdp);
	gst_webrtc_session_description_free(gstsdp);


	const auto& producer = d->producerOptions;

	auto it = producer.ice_candidates();
	json cands = {};
	for (const auto& item : producer.ice_candidates()) {
		cands.push_back(json::parse(item));
	}
	d->remoteSDP.reset(new mediasoupclient::Sdp::RemoteSdp(
		json::parse(producer.ice_parameters()),
		cands,
		json::parse(producer.dtls_parameters()),
		{}
	));

	const mediasoupclient::Sdp::RemoteSdp::MediaSectionIdx mediaSectionIdx = d->remoteSDP->GetNextMediaSectionIdx();
	json& offerMediaObject = d->localSDP["media"][mediaSectionIdx.idx];

	auto extended = getExtendedRtpCapabilities(d->localCaps, d->remoteCaps);

	json sendingRtpParametersByKind = {
		{ "audio", getSendingRtpParameters("audio", extended) },
	};

	json sendingRemoteRtpParametersByKind = {
		{ "audio", getSendingRemoteRtpParameters("audio", extended) },
	};

	json& sendingRtpParameters = sendingRtpParametersByKind["audio"];
	json& receivingRtpParameters = sendingRtpParametersByKind["audio"];

	d->remoteSDP->Send(
		offerMediaObject,
		mediaSectionIdx.reuseMid,
		sendingRtpParameters,
		receivingRtpParameters,
		nullptr);

	auto remoteDescription = parseSDP(d->remoteSDP->GetSdp(), GST_WEBRTC_SDP_TYPE_OFFER);
	GstPromise *setRemotePromise = gst_promise_new_with_change_func(remoteDescriptionCreated_, this, nullptr);
	g_signal_emit_by_name(d->webrtc, "set-remote-description", remoteDescription, setRemotePromise);
	gst_webrtc_session_description_free(remoteDescription);
}

void voiceCall::preparedForJoinChannel(voice::TransportOptions consumer, voice::TransportOptions producer)
{
	Q_UNUSED(consumer)

	d->consumerOptions = consumer;
	d->producerOptions = producer;

	qWarning() << "===" << "Creating offer...";

	GstPromise *promise = gst_promise_new_with_change_func(offerCreated_, this, nullptr);
	g_signal_emit_by_name(d->webrtc, "create-offer", nullptr, promise);
}

// we can just treat a bunch of users joining at once like they all joined
// one after the other.
// easy.
void voiceCall::joinedChannel(QList<voice::UserConsumerOptions> otherUsers)
{
	qWarning() << "=== joined channel!";
	for (const auto& user : otherUsers) {
		userJoined(user);
	}
	// TODO: anything else?
}

/*

another relevant excerpt from the protobuf docs:

think it's helpful.

/ Data containing all the necessary information to
/ create a consumer for a user in a voice channel
/
/ This corresponds to https://mediasoup.org/documentation/v3/mediasoup-client/api/#ConsumerOptions on client:
/ - `consumer_id` -> `id`
/ - `producer_id` -> `producerId`
/ - `rtp_parameters` -> `rtpParameters`
/ - and `kind` should be set to "audio".
message UserConsumerOptions {
	/ User ID of the user.
	uint64 user_id = 1;
	/ Producer ID of the producer being consumed.
	string producer_id = 2;
	/ Consumer ID for the user's producer consumer.
	string consumer_id = 3;
	/ RTP paramaters for the user's audio track. Corresponds to `RtpParameters` in mediasoup's TypeScript API.
	string rtp_parameters = 4;
}

*/

// this is a receiver track. we do the same sdp dance here.
// convert UserConsumerOptions to an SDP for a receiving track?
void voiceCall::userJoined(voice::UserConsumerOptions userData)
{
	qWarning() << "===" << "User is joining!...";
	d->data[userData.user_id()] = userData;

	// TODO: connect stream
}

// this is a track going brrrrr.
// we yeet.
// this should be fairly easy.
void voiceCall::userLeft(quint64 user)
{
	qWarning() << "===" << "User is leave!...";
	d->data.remove(user);

	// TODO: disconnect stream
}

