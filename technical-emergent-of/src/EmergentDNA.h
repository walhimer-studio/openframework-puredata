#pragma once
/**
 * Canonical SeedRng matching walhimer-studio/EmergentDNA emergent-dna-core.js:
 *   state: (seed >>> 0) || 1 - state kept as double and incremented like JS Number
 *   next:  sin(s++) * 10000; return x - floor(x)
 */
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

#include "ofColor.h"

class SeedRng {
public:
	explicit SeedRng(std::uint32_t seed) {
		std::uint32_t u = seed & 0xFFFFFFFFu;
		double s = static_cast<double>(u);
		if (s == 0.0) s = 1.0;
		state_ = s;
	}

	double next() {
		double x = std::sin(state_++) * 10000.0;
		return x - std::floor(x);
	}

	double range(double a, double b) { return a + next() * (b - a); }

	template<typename T>
	T pick(const std::vector<T>& arr) {
		if (arr.empty()) return T{};
		int idx = static_cast<int>(std::floor(next() * static_cast<double>(arr.size())));
		if (idx >= static_cast<int>(arr.size())) idx = static_cast<int>(arr.size()) - 1;
		return arr[static_cast<size_t>(idx)];
	}

	template<typename T>
	void shuffle(std::vector<T>& a) {
		for (int i = static_cast<int>(a.size()) - 1; i > 0; --i) {
			int j = static_cast<int>(std::floor(next() * static_cast<double>(i + 1)));
			std::swap(a[static_cast<size_t>(i)], a[static_cast<size_t>(j)]);
		}
	}

	int randInt(int minInclusive, int maxInclusive) {
		return minInclusive + static_cast<int>(std::floor(next() * static_cast<double>(maxInclusive - minInclusive + 1)));
	}

private:
	double state_{1.0};
};

struct TechnicalTraits {
	float lineWeight{1.5f};
	int complexity{24};
	int detailPasses{2};
	int microTickCount{40};
	bool showGrid{true};
	bool showElevation{true};
	bool showPrisms{true};
	bool showZigzags{true};
	bool showArrows{true};
	bool showDotMarks{true};
	/// Museum / exhibit sheet: axonometric masses, dimension chains, callout leaders, greeking plates (fake Latin-like filler).
	bool showMuseumExhibit{true};
	int museumFormCount{12};
	int museumDimensions{24};
	int museumCallouts{28};
	/// Exhibit plates: framed title + greeking body (fake Latin-like filler, not real text).
	int museumTextPlates{6};
	float elevationYFactor{0.7f};
	/// 0 beige, 1 light blue, 2 pink, 3 white - drives paper + ink (like drafting on colored stock).
	int paperKind{0};
	ofColor paperBackground{250, 245, 235};
	/// 0-7: primitive mix weights (line / circle / rect / ...).
	int styleId{0};
	float layoutScale{1.f};
	float layoutShiftX{0.f};
	float layoutShiftY{0.f};
	ofColor inkMain{40, 38, 36};
	ofColor inkDim{100, 92, 84};
	ofColor inkGrid{140, 130, 120, 85};
	float wLine{1.f};
	float wCircle{1.f};
	float wRect{1.f};
	float wRadial{1.f};
	float wArc{1.f};
	float wDimTicks{1.f};
	float gridDivMin{7.f};
	float gridDivMax{18.f};
};

inline void applyPaperPalette(TechnicalTraits& t, SeedRng& rng) {
	t.paperKind = rng.randInt(0, 3);
	switch (t.paperKind) {
	case 0: {
		const int r = static_cast<int>(rng.range(236.f, 252.f));
		const int g = static_cast<int>(rng.range(228.f, 245.f));
		const int b = static_cast<int>(rng.range(208.f, 232.f));
		t.paperBackground.set(r, g, b);
		t.inkMain.set(48, 42, 36);
		t.inkDim.set(110, 98, 86);
		t.inkGrid.set(150, 135, 118, 90);
		break;
	}
	case 1: {
		const int r = static_cast<int>(rng.range(210.f, 232.f));
		const int g = static_cast<int>(rng.range(232.f, 248.f));
		const int b = static_cast<int>(rng.range(248.f, 255.f));
		t.paperBackground.set(r, g, b);
		t.inkMain.set(32, 48, 68);
		t.inkDim.set(85, 110, 140);
		t.inkGrid.set(120, 150, 185, 85);
		break;
	}
	case 2: {
		const int r = static_cast<int>(rng.range(252.f, 255.f));
		const int g = static_cast<int>(rng.range(218.f, 238.f));
		const int b = static_cast<int>(rng.range(228.f, 245.f));
		t.paperBackground.set(r, g, b);
		t.inkMain.set(78, 44, 58);
		t.inkDim.set(140, 95, 118);
		t.inkGrid.set(190, 140, 160, 80);
		break;
	}
	default: {
		const int v = static_cast<int>(rng.range(246.f, 255.f));
		t.paperBackground.set(v, v, static_cast<int>(rng.range(248.f, 255.f)));
		t.inkMain.set(28, 28, 32);
		t.inkDim.set(95, 95, 102);
		t.inkGrid.set(165, 165, 172, 70);
		break;
	}
	}
}

inline TechnicalTraits expressTechnicalTraits(SeedRng& rng) {
	TechnicalTraits t;
	applyPaperPalette(t, rng);

	t.styleId = rng.randInt(0, 7);

	t.lineWeight = static_cast<float>(rng.range(0.85f, 3.2f));
	t.elevationYFactor = static_cast<float>(rng.range(0.42f, 0.88f));
	t.layoutScale = static_cast<float>(rng.range(0.5f, 1.14f));
	t.layoutShiftX = static_cast<float>(rng.range(-0.14f, 0.14f));
	t.layoutShiftY = static_cast<float>(rng.range(-0.11f, 0.11f));

	t.showGrid = rng.next() < 0.86f;
	t.showElevation = rng.next() < 0.9f;
	t.showPrisms = rng.next() < 0.82f;
	t.showZigzags = rng.next() < 0.9f;
	t.showArrows = rng.next() < 0.88f;
	t.showDotMarks = rng.next() < 0.92f;
	t.showMuseumExhibit = rng.next() < 0.94f;
	t.museumFormCount = rng.randInt(8, 26);
	t.museumDimensions = rng.randInt(14, 48);
	t.museumCallouts = rng.randInt(18, 58);
	t.museumTextPlates = rng.randInt(3, 12);

	t.complexity = rng.randInt(28, 78);
	t.detailPasses = rng.randInt(2, 5);
	t.microTickCount = rng.randInt(40, 130);

	t.gridDivMin = static_cast<float>(rng.range(5.f, 12.f));
	t.gridDivMax = std::max<float>(t.gridDivMin + 2.f, static_cast<float>(rng.range(14.0, 30.0)));

	switch (t.styleId) {
	case 0:
		t.wLine = 2.2f;
		t.wCircle = 0.6f;
		t.wRect = 1.4f;
		t.wRadial = 1.f;
		t.wArc = 1.2f;
		t.wDimTicks = 0.8f;
		t.complexity = rng.randInt(40, 78);
		break;
	case 1:
		t.wLine = 0.7f;
		t.wCircle = 2.5f;
		t.wRect = 0.8f;
		t.wRadial = 1.8f;
		t.wArc = 2.f;
		t.wDimTicks = 0.5f;
		break;
	case 2:
		t.wLine = 1.8f;
		t.wCircle = 0.9f;
		t.wRect = 2.2f;
		t.wRadial = 1.5f;
		t.wArc = 0.7f;
		t.wDimTicks = 1.6f;
		break;
	case 3:
		t.wLine = 1.2f;
		t.wCircle = 1.2f;
		t.wRect = 1.2f;
		t.wRadial = 1.2f;
		t.wArc = 1.2f;
		t.wDimTicks = 1.2f;
		t.detailPasses = rng.randInt(3, 5);
		break;
	case 4:
		t.wLine = 2.5f;
		t.wCircle = 2.f;
		t.wRect = 0.5f;
		t.wRadial = 0.6f;
		t.wArc = 1.8f;
		t.wDimTicks = 2.f;
		t.showGrid = rng.next() < 0.42f;
		break;
	case 5:
		t.wLine = 1.f;
		t.wCircle = 1.f;
		t.wRect = 2.8f;
		t.wRadial = 1.4f;
		t.wArc = 1.4f;
		t.wDimTicks = 1.4f;
		break;
	case 6:
		t.wLine = 1.5f;
		t.wCircle = 1.5f;
		t.wRect = 1.f;
		t.wRadial = 2.5f;
		t.wArc = 2.2f;
		t.wDimTicks = 0.9f;
		break;
	default:
		t.wLine = 1.f;
		t.wCircle = 1.f;
		t.wRect = 1.f;
		t.wRadial = 1.f;
		t.wArc = 1.f;
		t.wDimTicks = 1.f;
		break;
	}

	if (rng.next() < 0.12f) {
		t.showGrid = false;
	}
	if (rng.next() < 0.06f) {
		t.showElevation = false;
	}

	return t;
}
