#pragma once

struct ExpandingCircle : IEffect
{
	Vec2 pos_;
	ColorF color_;
	double delay_;

	ExpandingCircle(const Vec2& pos, const ColorF& color, double delay);

	bool update(double t) override;
};
