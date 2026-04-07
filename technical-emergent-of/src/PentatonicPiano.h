#pragma once

#include "ofMain.h"
#include <map>
#include <string>
#include <vector>

class PentatonicPiano {
public:
	static const std::vector<std::string>& pentatonicPool();
	static const std::vector<std::string>& pentatonicFiveDegreePool();

	void loadSamples(const std::string& soundsSubdir);
	bool isReady() const;
	bool playNote(const std::string& noteName, float velocity);

private:
	std::map<std::string, ofSoundPlayer> players_;
	float masterVolume_{1.f};
};
