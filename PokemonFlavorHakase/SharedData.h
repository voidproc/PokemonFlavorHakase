#pragma once

// シーン間の共有データ

struct SharedData
{
	// 選択中の難易度
	Difficulty difficulty;

	// 問題数
	int questionCount;

	// かかった時間
	double elapsedTime;
};
