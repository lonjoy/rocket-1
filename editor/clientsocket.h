#include "../sync/base.h"
#include <map>

class ClientSocket {
public:
	ClientSocket() : socket(INVALID_SOCKET) {}
	explicit ClientSocket(SOCKET socket) : socket(socket), clientPaused(true) {}

	bool connected() const
	{
		return INVALID_SOCKET != socket;
	}

	void disconnect()
	{
		closesocket(socket);
		socket = INVALID_SOCKET;
		clientTracks.clear();
	}

	virtual bool recv(char *buffer, size_t length, int flags)
	{
		if (!connected())
			return false;
		int ret = ::recv(socket, buffer, int(length), flags);
		if (ret != int(length)) {
			disconnect();
			return false;
		}
		return true;
	}

	virtual bool send(const char *buffer, size_t length, int flags)
	{
		if (!connected())
			return false;
		int ret = ::send(socket, buffer, int(length), flags);
		if (ret != int(length)) {
			disconnect();
			return false;
		}
		return true;
	}

	virtual bool pollRead()
	{
		if (!connected())
			return false;
		return !!socket_poll(socket);
	}

	void sendSetKeyCommand(const std::string &trackName, const struct track_key &key);
	void sendDeleteKeyCommand(const std::string &trackName, int row);
	void sendSetRowCommand(int row);
	void sendPauseCommand(bool pause);
	void sendSaveCommand();

	bool clientPaused;
	std::map<const std::string, size_t> clientTracks;

protected:
	SOCKET socket;
};

class WebSocket : public ClientSocket {
public:
	WebSocket() : ClientSocket(INVALID_SOCKET) {}
	explicit WebSocket(SOCKET socket) : ClientSocket(socket) {}

	bool recv(char *buffer, size_t length, int flags)
	{
		if (!connected())
			return false;
		while (length) {
			if (!buf.length() && !readFrame(buf))
				return false;

			int bytes = std::min(buf.length(), length);
			memcpy(buffer, &buf[0], bytes);
			buf = buf.substr(bytes);
			buffer += bytes;
			length -= bytes;
		}
		return true;
	}

	bool send(const char *buffer, size_t length, int flags)
	{
		unsigned char header[2];
		header[0] = 16 | 2;
		header[1] = length < 126 ? (unsigned char)(length) : 126;
		if (!ClientSocket::send((const char *)header, 2, 0))
			return false;

		if (length >= 126) {
			if (length > 0xffff)
				return false;

			unsigned short tmp = htons(unsigned short(length));
			if (!ClientSocket::send((const char *)&tmp, 2, 0))
				return false;
		}

		return ClientSocket::send(buffer, length, 0);
	}

	bool pollRead()
	{
		if (!connected())
			return false;
		if (buf.length())
			return true;
		return ClientSocket::pollRead();
	}

	bool readFrame(std::string &buf)
	{
		unsigned char header[2];
		int flags, opcode, masked, payload_len;
		unsigned char mask[4] = { 0 };

		if (!ClientSocket::recv((char *)header, 2, 0))
			return false;

		flags = header[0] >> 4;
		opcode = header[0] & 0xF;
		masked = header[1] >> 7;
		payload_len = header[1] & 0x7f;

		if (payload_len == 126) {
			unsigned short tmp;
			if (!ClientSocket::recv((char *)&tmp, 2, 0))
				return false;
			payload_len = ntohs(tmp);
		} else if (payload_len == 127) {
			// dude, that's one crazy big payload! let's bail!
			return false;
		}

		if (masked) {
			if (!ClientSocket::recv((char *)mask, 4, 0))
				return false;
		}

		buf.resize(payload_len);
		if (!ClientSocket::recv(&buf[0], payload_len, 0))
			return false;

		for (int i = 0; i < payload_len; ++i)
			buf[i] = buf[i] ^ mask[i & 3];

		return true;
	}
private:
	std::string buf;
};
