#include "launch_settings.h"

#include "framework/assert.h"

namespace feather {

LaunchSettings* LaunchSettings::_instance = nullptr;

LaunchSettings::LaunchSettings() { fassert(!_instance, "Attempt to create Launchsettings but it already exists"); }

LaunchSettings::LaunchSettings(int argc, char* argv[]) {
	if (_instance)
		throw std::runtime_error("LaunchSettings instance already exists!");

	_instance = this;

	_parser.ParseCLI(argc, argv);

	switch (_parser.GetError()) {
	case args::Error::Help:
		std ::cout << _parser;
		exit(0);
		break;
	default:
		break;
	}
}

void LaunchSettings::init(int argc, char* argv[]) {
	fassert(_instance, "LaunchSettings instance does not exists!");

	_parser.ParseCLI(argc, argv);

	switch (_parser.GetError()) {
	case args::Error::Help:
		std ::cout << _parser;
		exit(0);
		break;
	default:
		break;
	}
}

LaunchSettings& LaunchSettings::get() {
	if (!_instance)
		_instance = new LaunchSettings();

	return *_instance;
}

} //namespace feather