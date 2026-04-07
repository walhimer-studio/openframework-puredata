#include "ofMain.h"
#include "ofApp.h"

int main() {
	ofGLFWWindowSettings settings;
	settings.setSize(1280, 720);
	settings.windowMode = OF_WINDOW;
	settings.numSamples = 8;

	auto window = ofCreateWindow(settings);
	ofRunApp(window, std::make_shared<ofApp>());
	ofRunMainLoop();
	return 0;
}
