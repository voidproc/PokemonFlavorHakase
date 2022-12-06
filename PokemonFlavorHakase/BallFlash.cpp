#include "stdafx.h"
#include "BallFlash.h"


void BallFlash::start(const Vec2& pos, BallFlashType type)
{
	pos_ = pos;
	type_ = type;

	sw_.restart();
}

void BallFlash::update()
{
}

void BallFlash::draw() const
{
	const auto t = sw_.sF();

	if (type_ == BallFlashType::Receive)
	{
		if (sw_.isStarted() && t < 1.0)
		{
			// ボール点滅
			const auto ballColor = ColorF(0.9, 0.9, 0, 0.7 * (1.0 - t) * Periodic::Square0_1(200ms));
			Circle{ Arg::center = pos_, 24 }.draw(ballColor);

			// 星
			for (int i : step(8))
			{
				const auto scale = 0.01 + 1.0 - t;
				const auto dist = pos_ + Circular(24 + EaseOutCubic(t) * 100, 360_deg / 8 * i);
				const auto angle = t * 50_deg + i * 30_deg;
				const auto alpha = 1.0 - EaseInCubic(t);
				const auto color = blinkColor(ColorF(Palette::Yellow, alpha), ColorF(Palette::Orange, alpha), 80ms);
				Shape2D::NStar(4, 32 * scale, 12 * scale, dist, angle).draw(color);
			}
		}
	}
	else if (type_ == BallFlashType::Lose)
	{
		if (sw_.isStarted() && t < 1.2)
		{
			// ボール点滅
			const auto ballColor = ColorF(1, 0, 0, 0.8 * (1.2 - t) * Periodic::Square0_1(200ms));
			Circle{ Arg::center = pos_, 24 }.draw(ballColor);
		}
	}
}
