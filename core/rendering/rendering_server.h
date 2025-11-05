#pragma once

namespace feather {

class RenderingServer {
	static RenderingServer* _instance;

public:
	RenderingServer();

	static RenderingServer* get() { return _instance; };
};

} //namespace feather