# include "ExpandingCircle.h"

ExpandingCircle::ExpandingCircle(const Vec2& pos, const ColorF& color, double delay)
	: pos_{ pos }, color_{ color }, delay_{ delay }
{

}

bool ExpandingCircle::update(double t)
{
	if (t < delay_) return true;

	const auto s = (t - delay_) / 0.3;
	const double r = 300 + 1000 * EaseInCubic(s);
	Circle(Arg::center = pos_, r).drawFrame(96, color_);

	return (t < (0.3 + delay_));
}
