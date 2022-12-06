#pragma once

class StopwatchSequence
{
public:
	StopwatchSequence();

	void add(const String tag);

	void restart();

	void next();

	StringView runningTag() const;

	Stopwatch& running();

	const Stopwatch& running() const;

	void drawDebug() const;

private:
	struct StopwatchInfo
	{
		Stopwatch sw;
		String tag;
	};

	Array<StopwatchInfo> swInfoList_;
	int runningIndex_ = 0;
};
