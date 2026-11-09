import feather.main;
import feather.modules;

int main(int argc, char* argv[]) {
	return feather::run(argc, argv, { feather::register_modules, feather::unregister_modules });
}
