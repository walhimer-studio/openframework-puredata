#include "ofApp.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::vector<std::string> copyPool(SeedRng& rng) {
	auto pool = PentatonicPiano::pentatonicPool();
	rng.shuffle(pool);
	return pool;
}

std::vector<std::string> copyPoolFive(SeedRng& rng) {
	auto pool = PentatonicPiano::pentatonicFiveDegreePool();
	rng.shuffle(pool);
	return pool;
}

int degreeForFiveNoteName(const std::string& note) {
	const auto& p = PentatonicPiano::pentatonicFiveDegreePool();
	for (size_t i = 0; i < p.size(); ++i) {
		if (p[i] == note) return static_cast<int>(i);
	}
	return 0;
}

void drawSolidLineSegment(float x0, float y0, float x1, float y1, float thickness, const ofColor& c) {
	float dx = x1 - x0;
	float dy = y1 - y0;
	float len = std::sqrt(dx * dx + dy * dy);
	if (len < 1e-5f) return;
	dx /= len;
	dy /= len;
	const float h = thickness * 0.5f;
	const float px = -dy * h;
	const float py = dx * h;
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
	mesh.addVertex({x0 + px, y0 + py, 0.f});
	mesh.addVertex({x0 - px, y0 - py, 0.f});
	mesh.addVertex({x1 + px, y1 + py, 0.f});
	mesh.addVertex({x1 - px, y1 - py, 0.f});
	ofSetColor(c);
	mesh.draw();
}

int pickWeightedKind(SeedRng& rng, const TechnicalTraits& tr) {
	const float s = tr.wLine + tr.wCircle + tr.wRect + tr.wRadial + tr.wArc + tr.wDimTicks;
	if (s < 1e-5f) return 0;
	float p = static_cast<float>(rng.next()) * s;
	if ((p -= tr.wLine) < 0) return 0;
	if ((p -= tr.wCircle) < 0) return 1;
	if ((p -= tr.wRect) < 0) return 2;
	if ((p -= tr.wRadial) < 0) return 3;
	if ((p -= tr.wArc) < 0) return 4;
	if ((p -= tr.wDimTicks) < 0) return 5;
	return 0;
}

template <std::size_t N>
const char* pickStr(SeedRng& rng, const char* const (&arr)[N]) {
	return arr[rng.randInt(0, static_cast<int>(N) - 1)];
}

/// Pseudo-Latin greeking: plausible word shapes, no real meaning (cf. Lorem ipsum).
std::string fakeWord(SeedRng& rng, bool capitalize) {
	static const char* a[] = {"vel", "cor", "mer", "ost", "pul", "tra", "qui", "sce", "vol", "tem", "inc", "lab", "dol", "mag", "ali", "com", "dui", "rep", "par", "exc", "sun", "mol", "ani", "est", "lor", "ips", "met", "con", "sec", "adip", "elit", "eius", "temp", "cid", "ven", "nost", "ull", "co", "aute", "volup", "cill", "fug", "null", "caec", "cup", "pro", "culp", "off", "des", "min", "ullam", "arcu", "ante", "donec", "enim", "justo", "nisl", "orci", "purus", "risus", "urna"};
	static const char* b[] = {"trum", "pin", "lux", "ent", "ate", "ium", "ora", "are", "ere", "unt", "mus", "tis", "tur", "que", "ndi", "cit", "tat", "nem", "rum", "lit", "bor", "tum", "sum", "dolor", "amet", "elit", "sed", "mod", "enim", "minim", "nisi", "ex", "ea", "duis", "irure", "esse", "eur", "sint", "non", "mollit", "laborum", "sapien", "lectus", "felis", "metus", "neque", "velit", "vitae", "augue", "magna"};
	std::string w = std::string(pickStr(rng, a)) + pickStr(rng, b);
	if (capitalize && !w.empty()) {
		w[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(w[0])));
	}
	return w;
}

std::string greekingTitle(SeedRng& rng) {
	const int nw = rng.randInt(2, 5);
	std::string s;
	for (int i = 0; i < nw; ++i) {
		if (i > 0) s += ' ';
		s += fakeWord(rng, i == 0);
	}
	return s;
}

std::string greekingPhrase(SeedRng& rng, int minWords, int maxWords) {
	const int nw = rng.randInt(minWords, maxWords);
	std::string s;
	for (int i = 0; i < nw; ++i) {
		if (i > 0) s += ' ';
		s += fakeWord(rng, i == 0);
	}
	return s;
}

std::string greekingBody(SeedRng& rng) {
	const int ns = rng.randInt(2, 4);
	std::string p;
	for (int si = 0; si < ns; ++si) {
		if (si > 0) p += ' ';
		const int nw = rng.randInt(5, 12);
		for (int wi = 0; wi < nw; ++wi) {
			if (wi > 0) p += ' ';
			p += fakeWord(rng, wi == 0);
		}
		p += '.';
	}
	return p;
}

std::vector<std::string> wrapLinesTtf(ofTrueTypeFont& font, const std::string& text, float maxW) {
	std::vector<std::string> lines;
	std::istringstream iss(text);
	std::string word;
	std::string line;
	while (iss >> word) {
		const std::string trial = line.empty() ? word : (line + " " + word);
		const float w = font.stringWidth(trial);
		if (w > maxW && !line.empty()) {
			lines.push_back(line);
			line = word;
		} else {
			line = trial;
		}
	}
	if (!line.empty()) {
		lines.push_back(line);
	}
	return lines;
}

std::vector<std::string> wrapLinesPlain(const std::string& text, std::size_t maxChars) {
	std::vector<std::string> lines;
	std::istringstream iss(text);
	std::string word;
	std::string line;
	while (iss >> word) {
		const std::string trial = line.empty() ? word : (line + " " + word);
		if (trial.size() > maxChars && !line.empty()) {
			lines.push_back(line);
			line = word;
		} else {
			line = trial;
		}
	}
	if (!line.empty()) {
		lines.push_back(line);
	}
	return lines;
}

} // namespace

void ofApp::setup() {
	ofSetVerticalSync(true);
	ofSetFrameRate(60);
	ofEnableSmoothing();
	ofSetBackgroundAuto(true);
	ofBackground(250, 245, 235);
	ofEnableAlphaBlending();
	ofSetCircleResolution(96);

	width_ = static_cast<float>(ofGetWidth());
	height_ = static_cast<float>(ofGetHeight());

	piano_.loadSamples("sounds");
	if (!piano_.isReady()) {
		ofLogNotice("ofApp") << "No WAVs in bin/data/sounds - local playback silent. Press 'o' toggles OSC vs local.";
	}

	{
		const std::string bedPath = ofToDataPath("sounds/bed.wav", true);
		if (ofFile::doesFileExist(bedPath)) {
			bedTrack_.load(bedPath);
			bedTrack_.setLoop(true);
			bedTrack_.setVolume(0.2f);
			bedTrack_.play();
			bedTrackLoaded_ = true;
			ofLogNotice("ofApp") << "Playing optional bed track: bin/data/sounds/bed.wav";
		}
	}

	oscReceiver_.setup(oscFromPdPort_);
	ofLogNotice("ofApp") << "OSC from Pd: listening on port " << oscFromPdPort_ << " (/to-of-rms)";
	oscSender_.setup("127.0.0.1", oscToPdPort_);
	ofLogNotice("ofApp") << "OSC to Pd: sending to 127.0.0.1:" << oscToPdPort_ << " (/from-of)";

	{
		const std::string pathData = ofToDataPath("fonts/verdana.ttf", true);
		const std::string pathDataAlt = ofToDataPath("fonts/Verdana.ttf", true);
#ifdef __APPLE__
		const std::string pathSys = "/System/Library/Fonts/Supplemental/Verdana.ttf";
		const char* candidates[] = {pathData.c_str(), pathDataAlt.c_str(), pathSys.c_str()};
		const int nCand = 3;
#else
		const char* candidates[] = {pathData.c_str(), pathDataAlt.c_str()};
		const int nCand = 2;
#endif
		for (int i = 0; i < nCand; ++i) {
			const std::string p(candidates[i]);
			const bool absPath = !p.empty() && p[0] == '/';
			if (ofFile::doesFileExist(p, !absPath) && labelFont_.load(p, 11, true, true)) {
				labelFontReady_ = true;
				ofLogNotice("ofApp") << "Greeking label font: " << p;
				break;
			}
		}
		if (!labelFontReady_) {
			ofLogNotice("ofApp") << "Add bin/data/fonts/verdana.ttf for smoother exhibit text; using bitmap fallback.";
		}
	}

	regenerate(seed_);
}

void ofApp::update() {
	while (oscReceiver_.hasWaitingMessages()) {
		ofxOscMessage m;
		oscReceiver_.getNextMessage(m);
		if (m.getAddress() == "/to-of-rms" && m.getNumArgs() > 0) {
			rmsFromPd_ = m.getArgAsFloat(0);
		}
	}

	if (drawQueue_.empty()) return;

	const std::uint64_t now = static_cast<std::uint64_t>(ofGetElapsedTimeMillis());
	const bool firstStroke = (visibleCount_ == 0 && forwardPhase_ && !holdingFull_);
	const bool waited = (now - lastStepMs_ >= kStepMs);
	const bool releasingHold = (forwardPhase_ && holdingFull_);
	if (!releasingHold && !firstStroke && !waited) return;

	if (forwardPhase_) {
		if (holdingFull_) {
			if (now >= startReverseAtMs_) {
				holdingFull_ = false;
				forwardPhase_ = false;
				lastStepMs_ = now;
			}
			return;
		}
		if (visibleCount_ < drawQueue_.size()) {
			lastStepMs_ = now;
			playStepSound(visibleCount_, false);
			++visibleCount_;
			if (visibleCount_ == drawQueue_.size()) {
				holdingFull_ = true;
				startReverseAtMs_ = now + kHoldAtFullMs;
			}
		}
	} else {
		if (visibleCount_ > 0) {
			lastStepMs_ = now;
			--visibleCount_;
			if (visibleCount_ > 0) {
				playStepSound(visibleCount_, true);
			}
		}
		if (visibleCount_ == 0) {
			forwardPhase_ = true;
			holdingFull_ = false;
			regenerate(static_cast<std::uint32_t>(ofRandom(1, 0x7fffffff)));
		}
	}
}

void ofApp::windowResized(int w, int h) {
	width_ = static_cast<float>(w);
	height_ = static_cast<float>(h);
	regenerate(seed_);
}

void ofApp::regenerate(std::uint32_t newSeed) {
	seed_ = newSeed;
	rng_ = SeedRng(seed_);
	traits_ = expressTechnicalTraits(rng_);
	buildQueue();
	queueTotal_ = drawQueue_.size();
	if (useOscForAudio_) {
		noteCycle_ = copyPoolFive(rng_);
	} else {
		noteCycle_ = copyPool(rng_);
	}
	stepVelocity_.clear();
	{
		SeedRng audioRng(seed_ ^ 0xC0FFEEu);
		for (std::size_t i = 0; i < drawQueue_.size(); ++i) {
			stepVelocity_.push_back(0.12f + static_cast<float>(audioRng.next()) * 0.26f);
		}
	}
	visibleCount_ = 0;
	forwardPhase_ = true;
	holdingFull_ = false;
	lastStepMs_ = static_cast<std::uint64_t>(ofGetElapsedTimeMillis());
}

void ofApp::buildQueue() {
	drawQueue_.clear();
	const float lineW = traits_.lineWeight;
	const float elevY = traits_.elevationYFactor * height_;

	auto mx = [this](float x) {
		const float cx = width_ * 0.5f;
		return cx + (x - cx) * traits_.layoutScale + traits_.layoutShiftX * width_;
	};
	auto my = [this](float y) {
		const float cy = height_ * 0.5f;
		return cy + (y - cy) * traits_.layoutScale + traits_.layoutShiftY * height_;
	};

	const ofColor& cMain = traits_.inkMain;
	const ofColor& cDim = traits_.inkDim;
	const ofColor& cGrid = traits_.inkGrid;

	auto lineMain = [lineW, cMain](float x0, float y0, float x1, float y1) {
		drawSolidLineSegment(x0, y0, x1, y1, lineW, cMain);
	};
	const float gridW = std::max(1.f, lineW * 0.35f);
	auto lineGrid = [gridW, cGrid](float x0, float y0, float x1, float y1) {
		drawSolidLineSegment(x0, y0, x1, y1, gridW, cGrid);
	};
	auto lineDim = [lineW, cDim](float x0, float y0, float x1, float y1) {
		drawSolidLineSegment(x0, y0, x1, y1, lineW, cDim);
	};
	const float hatchW = std::max(1.f, lineW * 0.38f);
	const ofColor hatchCol = cDim;

	if (traits_.showGrid) {
		const float spacing = std::min(width_, height_) / rng_.range(traits_.gridDivMin, traits_.gridDivMax);
		for (float gx = 0; gx < width_; gx += spacing) {
			const float x0 = gx;
			drawQueue_.push_back([=, this]() {
				lineGrid(mx(x0), my(0), mx(x0), my(height_));
			});
		}
		for (float gy = 0; gy < height_; gy += spacing) {
			const float y0 = gy;
			drawQueue_.push_back([=, this]() {
				lineGrid(mx(0), my(y0), mx(width_), my(y0));
			});
		}
	}

	if (traits_.showElevation) {
		const float ey = elevY;
		drawQueue_.push_back([=, this]() { lineMain(mx(0), my(ey), mx(width_), my(ey)); });
		const float tickSpacing = width_ / 8.f;
		for (float tx = tickSpacing; tx < width_; tx += tickSpacing) {
			drawQueue_.push_back([=, this]() { lineMain(mx(tx), my(ey - 5), mx(tx), my(ey + 5)); });
		}
		const float hatchSpacing = 15.f;
		for (float hx = 0; hx < width_; hx += hatchSpacing) {
			drawQueue_.push_back([=, this]() {
				drawSolidLineSegment(mx(hx), my(ey + 2), mx(hx - 8), my(ey + 12), hatchW, hatchCol);
			});
		}
	}

	auto pushCircleSegments = [&](float cx, float cy, float r) {
		const float arcPx = 1.1f;
		int segs = static_cast<int>(std::round((2.f * PI * r) / arcPx));
		segs = static_cast<int>(ofClamp(static_cast<float>(segs), 56.f, 128.f));
		for (int s = 0; s < segs; ++s) {
			const float t0 = (static_cast<float>(s) / segs) * TWO_PI;
			const float t1 = (static_cast<float>(s + 1) / segs) * TWO_PI;
			const float xa = cx + std::cos(t0) * r;
			const float ya = cy + std::sin(t0) * r;
			const float xb = cx + std::cos(t1) * r;
			const float yb = cy + std::sin(t1) * r;
			drawQueue_.push_back([=, this]() { lineMain(mx(xa), my(ya), mx(xb), my(yb)); });
		}
	};

	for (int pass = 0; pass < traits_.detailPasses; ++pass) {
		for (int i = 0; i < traits_.complexity; ++i) {
			const int kind = pickWeightedKind(rng_, traits_);
			switch (kind) {
			case 0: {
				float x1 = rng_.range(width_ * 0.1f, width_ * 0.9f);
				float y1 = rng_.range(height_ * 0.1f, height_ * 0.9f);
				float x2 = rng_.range(width_ * 0.1f, width_ * 0.9f);
				float y2 = rng_.range(height_ * 0.1f, height_ * 0.9f);
				drawQueue_.push_back([=, this]() { lineMain(mx(x1), my(y1), mx(x2), my(y2)); });
				if (rng_.next() < 0.6f) {
					float ang = std::atan2(y2 - y1, x2 - x1);
					float perp = ang + static_cast<float>(PI) * 0.5f;
					float sp = 10.f;
					for (int j = 1; j <= 2; ++j) {
						float ox = std::cos(perp) * sp * j;
						float oy = std::sin(perp) * sp * j;
						ofColor c2 = cMain;
						c2.a = 150;
						drawQueue_.push_back([=, this]() {
							drawSolidLineSegment(mx(x1 + ox), my(y1 + oy), mx(x2 + ox), my(y2 + oy), lineW, c2);
						});
					}
				}
				break;
			}
			case 1: {
				float cx = rng_.range(width_ * 0.2f, width_ * 0.8f);
				float cy = rng_.range(height_ * 0.2f, height_ * 0.8f);
				float r = rng_.range(20.f, 95.f);
				pushCircleSegments(cx, cy, r);
				break;
			}
			case 2: {
				float x = rng_.range(width_ * 0.1f, width_ * 0.9f);
				float w = rng_.range(50.f, 180.f);
				float h = rng_.range(50.f, 180.f);
				float y = (traits_.showElevation && rng_.next() < 0.5f) ? (elevY - h)
																		 : rng_.range(height_ * 0.1f, height_ * 0.9f);
				drawQueue_.push_back([=, this]() { lineMain(mx(x), my(y), mx(x + w), my(y)); });
				drawQueue_.push_back([=, this]() { lineMain(mx(x + w), my(y), mx(x + w), my(y + h)); });
				drawQueue_.push_back([=, this]() { lineMain(mx(x + w), my(y + h), mx(x), my(y + h)); });
				drawQueue_.push_back([=, this]() { lineMain(mx(x), my(y + h), mx(x), my(y)); });
				if (rng_.next() < 0.45f) {
					float d = rng_.range(14.f, 28.f);
					drawQueue_.push_back([=, this]() { lineDim(mx(x), my(y), mx(x + d), my(y + d)); });
				}
				break;
			}
			case 3: {
				float vx = rng_.range(width_ * 0.3f, width_ * 0.7f);
				float vy = rng_.range(height_ * 0.3f, height_ * 0.7f);
				int n = rng_.randInt(4, 9);
				for (int j = 0; j < n; ++j) {
					float sx = rng_.range(0.f, width_);
					float sy = rng_.range(0.f, height_);
					drawQueue_.push_back([=, this]() { lineDim(mx(sx), my(sy), mx(vx), my(vy)); });
				}
				break;
			}
			case 4: {
				float ax = rng_.range(width_ * 0.15f, width_ * 0.85f);
				float ay = rng_.range(height_ * 0.15f, height_ * 0.85f);
				float sweep = rng_.range(static_cast<float>(PI) * 0.4f, static_cast<float>(PI) * 1.25f);
				float start = rng_.range(0.f, TWO_PI);
				float r = rng_.range(35.f, 120.f);
				int segs = static_cast<int>(ofClamp(sweep * r / 1.2f, 24.f, 120.f));
				for (int s = 0; s < segs; ++s) {
					float u0 = start + (static_cast<float>(s) / segs) * sweep;
					float u1 = start + (static_cast<float>(s + 1) / segs) * sweep;
					float x0 = ax + std::cos(u0) * r;
					float y0 = ay + std::sin(u0) * r;
					float x1 = ax + std::cos(u1) * r;
					float y1 = ay + std::sin(u1) * r;
					drawQueue_.push_back([=, this]() { lineMain(mx(x0), my(y0), mx(x1), my(y1)); });
				}
				break;
			}
			case 5: {
				float x = rng_.range(width_ * 0.2f, width_ * 0.75f);
				float y = rng_.range(height_ * 0.2f, height_ * 0.75f);
				float L = rng_.range(40.f, 140.f);
				float ang = rng_.range(0.f, TWO_PI);
				float dx = std::cos(ang) * L;
				float dy = std::sin(ang) * L;
				int ticks = rng_.randInt(4, 11);
				for (int t = 0; t < ticks; ++t) {
					float f = static_cast<float>(t) / static_cast<float>(std::max(1, ticks - 1));
					float px = x + dx * f;
					float py = y + dy * f;
					const float hp = static_cast<float>(PI) * 0.5f;
					float ox = std::cos(ang + hp) * 6.f;
					float oy = std::sin(ang + hp) * 6.f;
					drawQueue_.push_back([=, this]() {
						lineMain(mx(px - ox), my(py - oy), mx(px + ox), my(py + oy));
					});
				}
				break;
			}
			default:
				break;
			}
		}
	}

	for (int m = 0; m < traits_.microTickCount; ++m) {
		float x0 = rng_.range(width_ * 0.05f, width_ * 0.95f);
		float y0 = rng_.range(height_ * 0.05f, height_ * 0.95f);
		float len = rng_.range(8.f, 42.f);
		float ang = rng_.range(0.f, TWO_PI);
		float x1 = x0 + std::cos(ang) * len;
		float y1 = y0 + std::sin(ang) * len;
		ofColor tickCol = cDim;
		tickCol.a = 70;
		drawQueue_.push_back([=, this]() {
			drawSolidLineSegment(mx(x0), my(y0), mx(x1), my(y1), std::max(0.8f, lineW * 0.45f), tickCol);
		});
	}

	if (traits_.showPrisms) {
		for (int k = 0; k < rng_.randInt(4, 12); ++k) {
			float x = rng_.range(width_ * 0.2f, width_ * 0.8f);
			float sz = rng_.range(30.f, 95.f);
			float y = elevY - sz;
			float a1 = ofDegToRad(30);
			float a2 = ofDegToRad(150);
			float trx = x + std::cos(a1) * sz;
			float trY = y + std::sin(a1) * sz;
			float tlx = x + std::cos(a2) * sz;
			float tly = y + std::sin(a2) * sz;
			float frx = trx + std::cos(a2) * sz;
			float fry = trY + std::sin(a2) * sz;
			drawQueue_.push_back([=, this]() { lineMain(mx(x), my(y), mx(trx), my(trY)); });
			drawQueue_.push_back([=, this]() { lineMain(mx(x), my(y), mx(tlx), my(tly)); });
			drawQueue_.push_back([=, this]() { lineMain(mx(tlx), my(tly), mx(frx), my(fry)); });
			drawQueue_.push_back([=, this]() { lineMain(mx(trx), my(trY), mx(frx), my(fry)); });
			drawQueue_.push_back([=, this]() { lineMain(mx(x), my(y), mx(x), my(y + sz)); });
			drawQueue_.push_back([=, this]() { lineMain(mx(trx), my(trY), mx(trx), my(trY + sz)); });
			drawQueue_.push_back([=, this]() { lineMain(mx(tlx), my(tly), mx(tlx), my(tly + sz)); });
			drawQueue_.push_back([=, this]() { lineMain(mx(x), my(y + sz), mx(trx), my(trY + sz)); });
			drawQueue_.push_back([=, this]() { lineMain(mx(x), my(y + sz), mx(tlx), my(tly + sz)); });
		}
	}

	if (traits_.showZigzags) {
		for (int zz = 0; zz < rng_.randInt(6, 20); ++zz) {
			float x0 = rng_.range(width_ * 0.08f, width_ * 0.92f);
			float y0 = rng_.range(height_ * 0.08f, height_ * 0.92f);
			float x1 = rng_.range(width_ * 0.08f, width_ * 0.92f);
			float y1 = rng_.range(height_ * 0.08f, height_ * 0.92f);
			const float dx = x1 - x0;
			const float dy = y1 - y0;
			const float len = std::sqrt(dx * dx + dy * dy);
			if (len < 30.f) continue;
			const float ux = dx / len;
			const float uy = dy / len;
			const float px = -uy;
			const float py = ux;
			const float amp = rng_.range(7.f, 36.f);
			const int nSeg = rng_.randInt(5, 14);
			for (int i = 0; i < nSeg; ++i) {
				const float ta = static_cast<float>(i) / static_cast<float>(nSeg);
				const float tb = static_cast<float>(i + 1) / static_cast<float>(nSeg);
				const float sx = x0 + dx * ta;
				const float sy = y0 + dy * ta;
				const float ex = x0 + dx * tb;
				const float ey = y0 + dy * tb;
				const float sign = (i % 2 == 0) ? 1.f : -1.f;
				const float midx = (sx + ex) * 0.5f + px * amp * sign;
				const float midy = (sy + ey) * 0.5f + py * amp * sign;
				ofColor zc = cMain;
				zc.a = 200;
				drawQueue_.push_back([=, this]() {
					drawSolidLineSegment(mx(sx), my(sy), mx(midx), my(midy), lineW * 0.95f, zc);
				});
				drawQueue_.push_back([=, this]() {
					drawSolidLineSegment(mx(midx), my(midy), mx(ex), my(ey), lineW * 0.95f, zc);
				});
			}
		}
	}

	if (traits_.showArrows) {
		for (int ar = 0; ar < rng_.randInt(10, 32); ++ar) {
			float sx = rng_.range(width_ * 0.1f, width_ * 0.9f);
			float sy = rng_.range(height_ * 0.1f, height_ * 0.9f);
			float ex = rng_.range(width_ * 0.1f, width_ * 0.9f);
			float ey = rng_.range(height_ * 0.1f, height_ * 0.9f);
			float dx = ex - sx;
			float dy = ey - sy;
			float len = std::sqrt(dx * dx + dy * dy);
			if (len < 40.f) continue;
			float ux = dx / len;
			float uy = dy / len;
			float px = -uy;
			float py = ux;
			const float headLen = rng_.range(14.f, 26.f);
			const float wing = headLen * 0.48f;
			const float bx = ex - ux * headLen;
			const float by = ey - uy * headLen;
			ofColor ac = cDim;
			ac.a = 220;
			drawQueue_.push_back([=, this]() {
				drawSolidLineSegment(mx(sx), my(sy), mx(bx), my(by), lineW * 0.9f, ac);
			});
			drawQueue_.push_back([=, this]() {
				drawSolidLineSegment(mx(bx + px * wing), my(by + py * wing), mx(ex), my(ey), lineW * 0.9f, ac);
			});
			drawQueue_.push_back([=, this]() {
				drawSolidLineSegment(mx(bx - px * wing), my(by - py * wing), mx(ex), my(ey), lineW * 0.9f, ac);
			});
		}
	}

	if (traits_.showDotMarks) {
		for (int d = 0; d < rng_.randInt(22, 72); ++d) {
			float cx = rng_.range(width_ * 0.05f, width_ * 0.95f);
			float cy = rng_.range(height_ * 0.05f, height_ * 0.95f);
			float s = rng_.range(2.5f, 8.f);
			ofColor dc = cDim;
			dc.a = 160;
			drawQueue_.push_back([=, this]() {
				drawSolidLineSegment(mx(cx - s), my(cy), mx(cx + s), my(cy), std::max(0.7f, lineW * 0.35f), dc);
			});
			drawQueue_.push_back([=, this]() {
				drawSolidLineSegment(mx(cx), my(cy - s), mx(cx), my(cy + s), std::max(0.7f, lineW * 0.35f), dc);
			});
		}
	}

	if (traits_.showMuseumExhibit) {
		const float dimStroke = std::max(0.4f, lineW * 0.48f);
		ofColor dimCol = cDim;
		dimCol.a = 185;
		ofColor formCol = cMain;
		formCol.a = 245;

		auto lineAnnot = [dimStroke, mx, my](float x0, float y0, float x1, float y1, const ofColor& c) {
			drawSolidLineSegment(mx(x0), my(y0), mx(x1), my(y1), dimStroke, c);
		};

		auto pushEllipseOutline = [&](float ecx, float ecy, float rx, float ry, const ofColor& c, float sw) {
			const int segs = static_cast<int>(ofClamp((rx + ry) * 0.32f, 24.f, 64.f));
			for (int s = 0; s < segs; ++s) {
				const float t0 = (static_cast<float>(s) / static_cast<float>(segs)) * TWO_PI;
				const float t1 = (static_cast<float>(s + 1) / static_cast<float>(segs)) * TWO_PI;
				const float xa = ecx + std::cos(t0) * rx;
				const float ya = ecy + std::sin(t0) * ry;
				const float xb = ecx + std::cos(t1) * rx;
				const float yb = ecy + std::sin(t1) * ry;
				drawQueue_.push_back([=, this]() { drawSolidLineSegment(mx(xa), my(ya), mx(xb), my(yb), sw, c); });
			}
		};

		auto pushOpenCircle = [&](float ccx, float ccy, float rr) {
			const int n = 14;
			for (int s = 0; s < n; ++s) {
				const float t0 = (static_cast<float>(s) / static_cast<float>(n)) * TWO_PI;
				const float t1 = (static_cast<float>(s + 1) / static_cast<float>(n)) * TWO_PI;
				const float xa = ccx + std::cos(t0) * rr;
				const float ya = ccy + std::sin(t0) * rr;
				const float xb = ccx + std::cos(t1) * rr;
				const float yb = ccy + std::sin(t1) * rr;
				drawQueue_.push_back([=, this]() {
					drawSolidLineSegment(mx(xa), my(ya), mx(xb), my(yb), dimStroke * 0.9f, dimCol);
				});
			}
		};

		auto pushIsoRectSolid = [&](float Ax, float Ay, float w, float d, float hh) {
			const float a1 = ofDegToRad(30.f);
			const float a2 = ofDegToRad(150.f);
			const float Bx = Ax + std::cos(a1) * w;
			const float By = Ay + std::sin(a1) * w;
			const float Dx = Ax + std::cos(a2) * d;
			const float Dy = Ay + std::sin(a2) * d;
			const float Cx = Bx + (Dx - Ax);
			const float Cy = By + (Dy - Ay);
			const float tAy = Ay - hh;
			const float tBy = By - hh;
			const float tCy = Cy - hh;
			const float tDy = Dy - hh;
			const auto seg = [&](float xa, float ya, float xb, float yb) {
				drawQueue_.push_back([=, this]() {
					drawSolidLineSegment(mx(xa), my(ya), mx(xb), my(yb), dimStroke, formCol);
				});
			};
			seg(Ax, Ay, Bx, By);
			seg(Bx, By, Cx, Cy);
			seg(Cx, Cy, Dx, Dy);
			seg(Dx, Dy, Ax, Ay);
			seg(Ax, tAy, Bx, tBy);
			seg(Bx, tBy, Cx, tCy);
			seg(Cx, tCy, Dx, tDy);
			seg(Dx, tDy, Ax, tAy);
			seg(Ax, Ay, Ax, tAy);
			seg(Bx, By, Bx, tBy);
			seg(Cx, Cy, Cx, tCy);
			seg(Dx, Dy, Dx, tDy);
		};

		for (int f = 0; f < traits_.museumFormCount; ++f) {
			const int kind = rng_.randInt(0, 3);
			if (kind == 0) {
				const float cx = rng_.range(width_ * 0.18f, width_ * 0.82f);
				const float baseY = rng_.range(elevY - 160.f, elevY - 35.f);
				const float w0 = rng_.range(38.f, 95.f);
				const float d0 = rng_.range(32.f, 85.f);
				const int tiers = rng_.randInt(2, 5);
				for (int t = 0; t < tiers; ++t) {
					const float wi = std::max(18.f, static_cast<float>(w0 - static_cast<float>(t) * rng_.range(8.f, 16.f)));
					const float di = std::max(16.f, static_cast<float>(d0 - static_cast<float>(t) * rng_.range(7.f, 14.f)));
					const float x = cx - wi * 0.35f;
					const float y = baseY - static_cast<float>(t) * rng_.range(14.f, 22.f);
					const float th = rng_.range(10.f, 20.f);
					pushIsoRectSolid(x, y, wi, di, th);
				}
			} else if (kind == 1) {
				const float cx = rng_.range(width_ * 0.15f, width_ * 0.85f);
				const float cy = rng_.range(height_ * 0.2f, height_ * 0.82f);
				const float rx = rng_.range(22.f, 65.f);
				const float ry = rx * rng_.range(0.28f, 0.42f);
				const float h = rng_.range(35.f, 110.f);
				ofColor ec = formCol;
				ec.a = 230;
				pushEllipseOutline(cx, cy, rx, ry, ec, std::max(0.5f, lineW * 0.55f));
				pushEllipseOutline(cx, cy - h, rx, ry, ec, std::max(0.5f, lineW * 0.55f));
				const float swc = std::max(0.5f, lineW * 0.55f);
				drawQueue_.push_back([=, this]() {
					drawSolidLineSegment(mx(cx - rx), my(cy), mx(cx - rx), my(cy - h), swc, ec);
				});
				drawQueue_.push_back([=, this]() {
					drawSolidLineSegment(mx(cx + rx), my(cy), mx(cx + rx), my(cy - h), swc, ec);
				});
			} else if (kind == 2) {
				const float x = rng_.range(width_ * 0.2f, width_ * 0.75f);
				const float y = rng_.range(elevY - 140.f, elevY - 40.f);
				const float w = rng_.range(45.f, 120.f);
				const float d = rng_.range(40.f, 100.f);
				const float h = rng_.range(40.f, 95.f);
				pushIsoRectSolid(x, y, w, d, h);
			} else {
				const float x = rng_.range(width_ * 0.15f, width_ * 0.8f);
				const float sz = rng_.range(55.f, 130.f);
				const float y = elevY - sz * 0.85f;
				const float a1 = ofDegToRad(30.f);
				const float a2 = ofDegToRad(150.f);
				const float trx = x + std::cos(a1) * sz;
				const float trY = y + std::sin(a1) * sz;
				const float tlx = x + std::cos(a2) * sz;
				const float tly = y + std::sin(a2) * sz;
				const float frx = trx + std::cos(a2) * sz;
				const float fry = trY + std::sin(a2) * sz;
				ofColor vc = formCol;
				vc.a = 200;
				const float vw = dimStroke;
				drawQueue_.push_back([=, this]() {
					drawSolidLineSegment(mx(x), my(y), mx(trx), my(trY), vw, vc);
				});
				drawQueue_.push_back([=, this]() {
					drawSolidLineSegment(mx(x), my(y), mx(tlx), my(tly), vw, vc);
				});
				drawQueue_.push_back([=, this]() {
					drawSolidLineSegment(mx(tlx), my(tly), mx(frx), my(fry), vw, vc);
				});
				drawQueue_.push_back([=, this]() {
					drawSolidLineSegment(mx(trx), my(trY), mx(frx), my(fry), vw, vc);
				});
				drawQueue_.push_back([=, this]() {
					drawSolidLineSegment(mx(x), my(y), mx(x), my(y + sz * 0.35f), vw, vc);
				});
				drawQueue_.push_back([=, this]() {
					drawSolidLineSegment(mx(trx), my(trY), mx(trx), my(trY + sz * 0.35f), vw, vc);
				});
				drawQueue_.push_back([=, this]() {
					drawSolidLineSegment(mx(tlx), my(tly), mx(tlx), my(tly + sz * 0.35f), vw, vc);
				});
			}
		}

		for (int di = 0; di < traits_.museumDimensions; ++di) {
			const bool horiz = rng_.next() < 0.55f;
			if (horiz) {
				const float yFeat = rng_.range(height_ * 0.2f, height_ * 0.88f);
				float x0 = rng_.range(width_ * 0.12f, width_ * 0.55f);
				float x1 = x0 + rng_.range(55.f, 220.f);
				if (x1 > width_ * 0.92f) {
					x1 = width_ * 0.92f;
				}
				const float yDim = yFeat - rng_.range(38.f, 110.f);
				const float gap = rng_.range(10.f, 20.f);
				const float xm = (x0 + x1) * 0.5f;
				const float tick = rng_.range(4.f, 9.f);
				drawQueue_.push_back([=, this]() {
					lineAnnot(x0, yFeat, x0, yDim, dimCol);
				});
				drawQueue_.push_back([=, this]() {
					lineAnnot(x1, yFeat, x1, yDim, dimCol);
				});
				drawQueue_.push_back([=, this]() {
					lineAnnot(x0, yDim, xm - gap * 0.5f, yDim, dimCol);
				});
				drawQueue_.push_back([=, this]() {
					lineAnnot(xm + gap * 0.5f, yDim, x1, yDim, dimCol);
				});
				drawQueue_.push_back([=, this]() {
					lineAnnot(x0, yDim - tick, x0, yDim + tick, dimCol);
				});
				drawQueue_.push_back([=, this]() {
					lineAnnot(x1, yDim - tick, x1, yDim + tick, dimCol);
				});
			} else {
				const float xFeat = rng_.range(width_ * 0.15f, width_ * 0.85f);
				float y0 = rng_.range(height_ * 0.2f, height_ * 0.55f);
				float y1 = y0 + rng_.range(50.f, 180.f);
				if (y1 > height_ * 0.9f) {
					y1 = height_ * 0.9f;
				}
				const float xDim = xFeat - rng_.range(35.f, 100.f);
				const float gap = rng_.range(10.f, 18.f);
				const float ym = (y0 + y1) * 0.5f;
				const float tick = rng_.range(4.f, 9.f);
				drawQueue_.push_back([=, this]() {
					lineAnnot(xFeat, y0, xDim, y0, dimCol);
				});
				drawQueue_.push_back([=, this]() {
					lineAnnot(xFeat, y1, xDim, y1, dimCol);
				});
				drawQueue_.push_back([=, this]() {
					lineAnnot(xDim, y0, xDim, ym - gap * 0.5f, dimCol);
				});
				drawQueue_.push_back([=, this]() {
					lineAnnot(xDim, ym + gap * 0.5f, xDim, y1, dimCol);
				});
				drawQueue_.push_back([=, this]() {
					lineAnnot(xDim - tick, y0, xDim + tick, y0, dimCol);
				});
				drawQueue_.push_back([=, this]() {
					lineAnnot(xDim - tick, y1, xDim + tick, y1, dimCol);
				});
			}
		}

		for (int ti = 0; ti < traits_.museumTextPlates; ++ti) {
			const float px = rng_.range(width_ * 0.05f, width_ * 0.62f);
			const float py = rng_.range(height_ * 0.06f, height_ * 0.55f);
			const float boxW = rng_.range(148.f, 258.f);
			const float pad = 8.f;
			const float innerW = boxW - pad * 2.f;
			const std::string titleStr = greekingTitle(rng_);
			const std::string bodyStr = greekingBody(rng_);
			std::vector<std::string> titleLines;
			std::vector<std::string> bodyLines;
			if (labelFontReady_) {
				titleLines = wrapLinesTtf(labelFont_, titleStr, innerW);
				bodyLines = wrapLinesTtf(labelFont_, bodyStr, innerW);
			} else {
				titleLines = wrapLinesPlain(titleStr, 26);
				bodyLines = wrapLinesPlain(bodyStr, 34);
			}
			if (titleLines.empty()) {
				titleLines.push_back(titleStr);
			}
			if (bodyLines.empty()) {
				bodyLines.push_back(bodyStr);
			}
			const float lineStep = labelFontReady_ ? labelFont_.getLineHeight() : 12.f;
			const float titleH = static_cast<float>(titleLines.size()) * lineStep;
			const float bodyH = static_cast<float>(bodyLines.size()) * lineStep;
			const float boxH = pad + titleH + 6.f + bodyH + pad;
			const ofColor inkTitle = formCol;
			const ofColor inkBody = dimCol;
			const ofColor frameCol = dimCol;
			const float ruleStroke = std::max(0.45f, dimStroke * 0.95f);
			drawQueue_.push_back([=, this]() {
				ofPushStyle();
				ofColor fill = traits_.paperBackground;
				fill.a = 38;
				ofSetColor(fill);
				ofFill();
				ofDrawRectangle(mx(px), my(py), boxW, boxH);
				ofNoFill();
				ofSetColor(frameCol);
				ofSetLineWidth(std::max(0.8f, traits_.lineWeight * 0.38f));
				ofDrawRectangle(mx(px), my(py), boxW, boxH);
				ofSetLineWidth(std::max(0.5f, traits_.lineWeight * 0.28f));
				ofDrawRectangle(mx(px) + 2.f, my(py) + 2.f, boxW - 4.f, boxH - 4.f);
				float yLay = py + pad + lineStep * 0.88f;
				if (labelFontReady_) {
					ofSetColor(inkTitle);
					for (const auto& ln : titleLines) {
						labelFont_.drawString(ln, mx(px + pad), my(yLay));
						yLay += lineStep;
					}
					ofColor ib = inkBody;
					ib.a = 218;
					ofSetColor(ib);
					yLay += 4.f;
					for (const auto& ln : bodyLines) {
						labelFont_.drawString(ln, mx(px + pad), my(yLay));
						yLay += lineStep;
					}
				} else {
					ofSetColor(inkTitle);
					float yb = py + pad + 10.f;
					for (const auto& ln : titleLines) {
						ofDrawBitmapString(ln, mx(px + pad), my(yb));
						yb += 12.f;
					}
					yb += 4.f;
					ofColor ib = inkBody;
					ib.a = 218;
					ofSetColor(ib);
					for (const auto& ln : bodyLines) {
						ofDrawBitmapString(ln, mx(px + pad), my(yb));
						yb += 12.f;
					}
				}
				ofColor ru = dimCol;
				ru.a = 175;
				drawSolidLineSegment(mx(px + pad), my(py + pad + titleH + 3.f), mx(px + boxW - pad), my(py + pad + titleH + 3.f), ruleStroke, ru);
				ofPopStyle();
			});
		}

		for (int ci = 0; ci < traits_.museumCallouts; ++ci) {
			const float ax = rng_.range(width_ * 0.2f, width_ * 0.8f);
			const float ay = rng_.range(height_ * 0.18f, height_ * 0.82f);
			const float kneeX = ax + rng_.range(-90.f, 90.f);
			const float kneeY = ay - rng_.range(25.f, 120.f);
			const float ex = rng_.range(width_ * 0.06f, width_ * 0.94f);
			const float ey = rng_.range(height_ * 0.06f, height_ * 0.94f);
			const float anchorR = rng_.range(2.5f, 5.f);
			pushOpenCircle(ax, ay, anchorR);
			drawQueue_.push_back([=, this]() {
				lineAnnot(ax, ay, kneeX, kneeY, dimCol);
			});
			drawQueue_.push_back([=, this]() {
				lineAnnot(kneeX, kneeY, ex, ey, dimCol);
			});
			const float bubbleR = rng_.range(6.f, 11.f);
			pushOpenCircle(ex, ey, bubbleR);
			const std::string capStr = greekingPhrase(rng_, 4, 10);
			const float capMaxInner = 120.f;
			std::vector<std::string> capLines;
			float capLineStep = 12.f;
			if (labelFontReady_) {
				capLines = wrapLinesTtf(labelFont_, capStr, capMaxInner);
				capLineStep = labelFont_.getLineHeight();
			} else {
				capLines = wrapLinesPlain(capStr, 22);
			}
			if (capLines.empty()) {
				capLines.push_back(capStr);
			}
			float capW = capMaxInner + 12.f;
			if (labelFontReady_) {
				float mw = 0.f;
				for (const auto& ln : capLines) {
					mw = std::max(mw, labelFont_.stringWidth(ln));
				}
				capW = std::min(std::max(mw + 14.f, 92.f), 196.f);
			}
			const float capH = static_cast<float>(capLines.size()) * capLineStep + 8.f;
			float tcx = ex + bubbleR + 7.f;
			if (tcx + capW > width_ * 0.95f) {
				tcx = ex - capW - 7.f;
			}
			float tcy = ey - capH * 0.5f;
			if (tcy < height_ * 0.04f) {
				tcy = height_ * 0.04f;
			}
			if (tcy + capH > height_ * 0.94f) {
				tcy = height_ * 0.94f - capH;
			}
			const ofColor capInk = formCol;
			drawQueue_.push_back([=, this]() {
				ofPushStyle();
				ofColor fill = traits_.paperBackground;
				fill.a = 52;
				ofSetColor(fill);
				ofFill();
				ofDrawRectangle(mx(tcx), my(tcy), capW, capH);
				ofNoFill();
				ofSetColor(dimCol);
				ofSetLineWidth(std::max(0.55f, traits_.lineWeight * 0.34f));
				ofDrawRectangle(mx(tcx), my(tcy), capW, capH);
				float yL = tcy + capLineStep * 0.88f;
				if (labelFontReady_) {
					ofColor ci = capInk;
					ci.a = 238;
					ofSetColor(ci);
					for (const auto& ln : capLines) {
						labelFont_.drawString(ln, mx(tcx + 4.f), my(yL));
						yL += capLineStep;
					}
				} else {
					ofColor ci = capInk;
					ci.a = 238;
					ofSetColor(ci);
					for (const auto& ln : capLines) {
						ofDrawBitmapString(ln, mx(tcx + 4.f), my(yL));
						yL += 12.f;
					}
				}
				ofPopStyle();
			});
		}
	}
}

void ofApp::playStepSound(std::size_t lineIndex, bool reverseErase) {
	if (noteCycle_.empty() || lineIndex >= stepVelocity_.size()) return;

	const std::size_t n = noteCycle_.size();
	std::string const* notePtr = nullptr;
	if (!reverseErase) {
		notePtr = &noteCycle_[lineIndex % n];
	} else {
		const std::size_t rev = (n - 1) - (lineIndex % n);
		notePtr = &noteCycle_[rev];
	}
	const std::string& note = *notePtr;
	float vel = stepVelocity_[lineIndex];
	if (reverseErase) {
		vel *= 0.38f;
	}

	int velMidi = static_cast<int>(std::lround(vel * 127.f));
	velMidi = std::max(1, std::min(127, velMidi));

	if (useOscForAudio_) {
		{
			ofxOscMessage om;
			om.setAddress("/from-of");
			om.addIntArg(degreeForFiveNoteName(note));
			om.addIntArg(reverseErase ? std::max(1, velMidi * 2 / 3) : velMidi);
			oscSender_.sendMessage(om);
		}
		if (!reverseErase) {
			const std::string& harmonyNote = noteCycle_[(lineIndex + 2) % n];
			const float harmVel = vel * 0.36f;
			int harmMidi = static_cast<int>(std::lround(harmVel * 127.f));
			harmMidi = std::max(1, std::min(127, harmMidi));
			ofxOscMessage hm;
			hm.setAddress("/from-of-harm");
			hm.addIntArg(degreeForFiveNoteName(harmonyNote));
			hm.addIntArg(harmMidi);
			oscSender_.sendMessage(hm);
		}
	} else if (piano_.isReady()) {
		piano_.playNote(note, vel);
		if (!reverseErase) {
			const std::string& harmonyNote = noteCycle_[(lineIndex + 2) % n];
			piano_.playNote(harmonyNote, vel * 0.36f);
		}
	}
}

void ofApp::drawLifeline() {
	const float barH = 8.f;
	const float y = height_ - barH - 4.f;

	const ofColor& paper = traits_.paperBackground;
	ofColor track;
	track.r = static_cast<unsigned char>(std::max(0, paper.r - 35));
	track.g = static_cast<unsigned char>(std::max(0, paper.g - 35));
	track.b = static_cast<unsigned char>(std::max(0, paper.b - 35));
	ofSetColor(track);
	ofFill();
	ofDrawRectangle(0.f, y, width_, barH);

	float progress = 0.f;
	if (queueTotal_ > 0) {
		progress = static_cast<float>(visibleCount_) / static_cast<float>(queueTotal_);
	}
	if (holdingFull_) {
		progress = 1.f;
	}
	progress = std::max(0.f, std::min(1.f, progress));

	ofColor fill = traits_.inkMain;
	fill.a = 210;
	ofSetColor(fill);
	ofDrawRectangle(0.f, y, width_ * progress, barH);

	if (rmsFromPd_ > 0.03f) {
		float rms = std::max(0.f, std::min(1.f, rmsFromPd_));
		ofSetColor(255, 255, 255, 100);
		ofDrawRectangle(0.f, y - 2.f, width_ * rms * progress, 2.f);
	}

	ofNoFill();
	ofColor border = traits_.inkDim;
	border.a = 140;
	ofSetColor(border);
	ofDrawRectangle(0.f, y, width_, barH);
}

void ofApp::drawEmergentHud() {
	std::string phase;
	if (holdingFull_) {
		phase = "complete";
	} else if (forwardPhase_) {
		phase = "draw";
	} else {
		phase = "erase";
	}
	const std::string audio = useOscForAudio_ ? "OSC->Pd" : "local WAV";
	const std::string bed = bedTrackLoaded_ ? " +bed" : "";
	static const char* kPaper[] = {"beige", "blue", "pink", "white"};
	const char* pname = (traits_.paperKind >= 0 && traits_.paperKind < 4) ? kPaper[traits_.paperKind] : "?";
	std::string line = std::string("EmergentDNA seed ") + ofToString(seed_) + "  paper " + pname + "  style "
					   + ofToString(traits_.styleId) + "  |  " + phase + "  |  " + audio + bed;
	ofColor hudBg(255, 255, 255, 180);
	ofColor hudFg = traits_.inkMain;
	hudFg.a = 255;
	ofDrawBitmapStringHighlight(line, 10, 16, hudBg, hudFg);
}

void ofApp::draw() {
	ofBackground(traits_.paperBackground);
	if (!drawQueue_.empty()) {
		const std::size_t n = std::min(visibleCount_, drawQueue_.size());
		for (std::size_t i = 0; i < n; ++i) {
			drawQueue_[i]();
		}
	}
	drawLifeline();
	drawEmergentHud();
}

void ofApp::keyPressed(int key) {
	if (key == 'o' || key == 'O') {
		useOscForAudio_ = !useOscForAudio_;
		ofLogNotice("ofApp") << "Audio: " << (useOscForAudio_ ? "OSC -> Pd" : "local ofSoundPlayer");
		regenerate(seed_);
	} else if (key == ' ') {
		regenerate(static_cast<std::uint32_t>(ofRandom(1, 0x7fffffff)));
	} else if (key == 'r' || key == 'R') {
		regenerate(seed_);
	} else if (key == 'n' || key == 'N') {
		++seed_;
		regenerate(seed_);
	} else if (key == 'b' || key == 'B') {
		if (bedTrackLoaded_) {
			if (bedTrack_.isPlaying()) {
				bedTrack_.stop();
			} else {
				bedTrack_.play();
			}
		}
	}
}
