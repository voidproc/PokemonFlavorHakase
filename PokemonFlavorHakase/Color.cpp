# include "Color.h"

ColorF blinkColor(const ColorF& color1, const ColorF& color2, const Duration& period)
{
	return Periodic::Square0_1(period) > 0.5 ? color1 : color2;
}
