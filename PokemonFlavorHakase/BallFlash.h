#pragma once

enum class BallFlashType
{
	Receive,
	Lose,
};

struct BallFlash
{
	Vec2 pos_{};
	Stopwatch sw_{};
	BallFlashType type_{};

	void start(const Vec2& pos, BallFlashType type);

	void update();

	void draw() const;
};
