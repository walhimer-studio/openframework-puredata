#pragma once

#include "EmergentDNA.h"
#include "PentatonicPiano.h"
#include "ofMain.h"
#include "ofxOsc.h"
#include <functional>
#include <vector>

class ofApp : public ofBaseApp {
public:
	void setup() override;
	void update() override;
	void draw() override;
	void keyPressed(int key) override;
	void windowResized(int w, int h) override;

private:
	void regenerate(std::uint32_t newSeed);
	void buildQueue();
	void playStepSound(std::size_t lineIndex, bool reverseErase);
	void drawLifeline();
	void drawEmergentHud();

	std::uint32_t seed_{12345};
	SeedRng rng_{seed_};
	TechnicalTraits traits_{};
	PentatonicPiano piano_{};
	ofSoundPlayer bedTrack_;
	bool bedTrackLoaded_{false};

	ofxOscSender oscSender_;
	ofxOscReceiver oscReceiver_;
	bool useOscForAudio_{true};
	int oscToPdPort_{9000};
	int oscFromPdPort_{9001};
	float rmsFromPd_{0.f};

	std::vector<std::function<void()>> drawQueue_{};
	std::vector<std::string> noteCycle_{};
	std::vector<float> stepVelocity_{};
	std::size_t queueTotal_{0};

	std::size_t visibleCount_{0};
	bool forwardPhase_{true};
	bool holdingFull_{false};
	std::uint64_t startReverseAtMs_{0};
	std::uint64_t lastStepMs_{0};

	static constexpr std::uint64_t kStepMs = 480;
	static constexpr std::uint64_t kHoldAtFullMs = 900;

	float width_{1024.f};
	float height_{768.f};

	ofTrueTypeFont labelFont_{};
	bool labelFontReady_{false};
};
