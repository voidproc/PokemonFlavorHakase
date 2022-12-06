# include "StopwatchSequence.h"

StopwatchSequence::StopwatchSequence()
{

}

void StopwatchSequence::add(const String tag)
{
	swInfoList_.push_back(StopwatchInfo{ Stopwatch{}, tag });
}

void StopwatchSequence::restart()
{
	for (auto& swInfo : swInfoList_)
	{
		swInfo.sw.reset();
	}

	if (not swInfoList_.isEmpty())
	{
		swInfoList_.front().sw.restart();
	}

	runningIndex_ = 0;
}

void StopwatchSequence::next()
{
	int running = runningIndex_;
	int next = (runningIndex_ + 1) % swInfoList_.size();

	swInfoList_[running].sw.reset();
	swInfoList_[next].sw.restart();

	runningIndex_ = next;
}

StringView StopwatchSequence::runningTag() const
{
	return swInfoList_[runningIndex_].tag;
}

Stopwatch& StopwatchSequence::running()
{
	return swInfoList_[runningIndex_].sw;
}

const Stopwatch& StopwatchSequence::running() const
{
	return swInfoList_[runningIndex_].sw;
}

void StopwatchSequence::drawDebug() const
{
#ifdef DEBUGMODE
	for (auto [index, swInfo] : Indexed(swInfoList_))
	{
		FontAsset(U"sys")(U"{:.2f} ({})"_fmt(swInfo.sw.sF(), swInfo.tag)).draw(24, Vec2(8, 8 + index * 24), Palette::Blue);
	}
#endif
}
