#include "PentatonicPiano.h"
#include <algorithm>
#include <map>

namespace {

const std::vector<std::string> kPentaPool = {
	"C2", "D2", "E2", "G2", "A2", "C3", "D3", "E3", "G3", "A3",
	"C4", "D4", "E4", "G4", "A4", "C5", "D5", "E5", "G5", "A5", "C6", "D6"
};

struct NoteDef {
	std::string sampleBase;
	float rate{1.f};
};

const std::map<std::string, NoteDef>& noteTable() {
	static const std::map<std::string, NoteDef> m = {
		{"C2", {"C2", 1.f}},
		{"D2", {"Ds2", 0.943874f}},
		{"E2", {"Ds2", 1.059463f}},
		{"G2", {"Fs2", 1.059463f}},
		{"A2", {"A2", 1.f}},
		{"C3", {"C3", 1.f}},
		{"D3", {"Ds3", 0.943874f}},
		{"E3", {"Ds3", 1.059463f}},
		{"G3", {"Fs3", 1.059463f}},
		{"A3", {"A3", 1.f}},
		{"C4", {"C4", 1.f}},
		{"D4", {"Ds4", 0.943874f}},
		// Aliases for pentatonicFiveDegreePool() (same samples as D4 / G4 rows).
		{"D#4", {"Ds4", 1.f}},
		{"E4", {"Ds4", 1.059463f}},
		{"G4", {"Fs4", 1.059463f}},
		{"F#4", {"Fs4", 1.f}},
		{"A4", {"A4", 1.f}},
		{"C5", {"C5", 1.f}},
		{"D5", {"Ds5", 0.943874f}},
		{"E5", {"Ds5", 1.059463f}},
		{"G5", {"Fs5", 1.059463f}},
		{"A5", {"A5", 1.f}},
		{"C6", {"C6", 1.f}},
		{"D6", {"Ds6", 0.943874f}},
	};
	return m;
}

} // namespace

const std::vector<std::string>& PentatonicPiano::pentatonicPool() {
	return kPentaPool;
}

const std::vector<std::string>& PentatonicPiano::pentatonicFiveDegreePool() {
	static const std::vector<std::string> five = {"C4", "D#4", "F#4", "A4", "C5"};
	return five;
}

bool PentatonicPiano::isReady() const {
	return !players_.empty();
}

void PentatonicPiano::loadSamples(const std::string& soundsSubdir) {
	players_.clear();
	std::map<std::string, bool> seen;
	for (const auto& kv : noteTable()) {
		const std::string& base = kv.second.sampleBase;
		if (seen.count(base)) continue;
		seen[base] = true;
		std::string rel = soundsSubdir + "/" + base + ".wav";
		std::string full = ofToDataPath(rel, true);
		if (!ofFile::doesFileExist(full)) {
			ofLogWarning("PentatonicPiano") << "missing sample: " << rel;
			continue;
		}
		ofSoundPlayer p;
		p.load(full);
		p.setMultiPlay(true);
		p.setVolume(masterVolume_);
		players_[base] = std::move(p);
	}
}

bool PentatonicPiano::playNote(const std::string& noteName, float velocity) {
	auto it = noteTable().find(noteName);
	if (it == noteTable().end()) return false;
	const std::string& base = it->second.sampleBase;
	float rate = it->second.rate;
	auto pit = players_.find(base);
	if (pit == players_.end()) return false;
	pit->second.setSpeed(rate);
	const float v = std::clamp(velocity, 0.f, 1.f);
	pit->second.setVolume(masterVolume_ * v);
	pit->second.play();
	return true;
}
