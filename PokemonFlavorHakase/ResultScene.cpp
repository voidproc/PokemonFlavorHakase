# include "ResultScene.h"

ResultScene::ResultScene(const InitData& init)
	: IScene{ init }
{
}

void ResultScene::update()
{
	if (KeyEnter.down() && swScene_.sF() > 3.0)
	{
		changeScene(U"TitleScene", 1000ms);
	}

#ifdef DEBUGMODE
	if (KeyLeft.down())
	{
		getData().difficulty = static_cast<Difficulty>((static_cast<int>(getData().difficulty) - 1 + difficultyCount()) % difficultyCount());
	}

	if (KeyRight.down())
	{
		getData().difficulty = static_cast<Difficulty>((static_cast<int>(getData().difficulty) + 1) % difficultyCount());
	}
#endif
}

void ResultScene::draw() const
{
	drawBg_();

	const auto diffColor = difficultyColor(getData().difficulty);
	const auto shadowColor = ColorF(0, 0.1);

	const auto congratsText = U"おめでとう!";
	const auto congratsDt = FontAsset(U"title")(congratsText);
	const auto congratsPos = Scene::CenterF().movedBy(0, -300);
	const auto congratsRegion = congratsDt.regionAt(congratsPos);
	const auto congratsBgColor = Palette::Whitesmoke;
	const auto congratsColor = diffColor;
	const auto congratsFontSize = 150;

	RectF{ 0, congratsRegion.topY(), Scene::Width(), congratsRegion.h }.draw(congratsBgColor);
	congratsDt.drawAt(congratsFontSize, congratsPos.movedBy(3, 3), shadowColor);
	congratsDt.drawAt(congratsFontSize, congratsPos, ColorF(congratsColor, 1.0 - 0.1 * Periodic::Square0_1(150ms)));

	const Array<String> youareText {
		U"きみは　今日から",
		U"ポケモンフレーバーはかせ【{}】"_fmt(difficultyName(getData().difficulty)),
		U"を　名乗っていいぞ!",
	};

	for (auto [index, text] : Indexed(youareText))
	{
		const auto youareDt = FontAsset(U"title")(text);
		const auto youarePos = congratsPos.movedBy(0, 280);
		const auto youareLineHeight = 96;
		const auto youareColor = Palette::Whitesmoke;
		const auto youareFontSize = 64;

		youareDt.drawAt(youareFontSize, youarePos.movedBy(0, youareLineHeight * index).movedBy(2, 2), shadowColor);
		youareDt.drawAt(youareFontSize, youarePos.movedBy(0, youareLineHeight * index), youareColor);
	}

	const auto resultText = U"もんだい数 {}　かかった時間 {:.1f} 秒"_fmt(getData().questionCount, getData().elapsedTime);
	const auto resultDt = FontAsset(U"sys")(resultText);
	const auto resultPos = Scene::Rect().bottomCenter().movedBy(0, -200);
	const auto resultFontSize = 48;
	const auto resultRegion = resultDt.regionAt(resultFontSize, resultPos);
	const auto resultBgColor = ColorF(Palette::Whitesmoke, 0.93);
	const auto resultColor = diffColor;

	RoundRect{ resultRegion.stretched(36, 24), resultRegion.h+32 }.draw(resultBgColor);
	resultDt.drawAt(resultFontSize, resultPos.movedBy(2, 2), shadowColor);
	resultDt.drawAt(resultFontSize, resultPos, resultColor);

	// アプリ名、日付など

	const auto appInfoText = U"{}.{}.{} - PokemonFlavorHakase v{}"_fmt(Date::Today().year, Date::Today().month, Date::Today().day, appVersion());
	const auto appInfoFontSize = 24;
	const auto appInfoDt = FontAsset(U"sys")(appInfoText);
	const auto appInfoRegion = appInfoDt.region(appInfoFontSize);

	appInfoDt.draw(appInfoFontSize, Scene::Rect().br().movedBy(-appInfoRegion.w-16, -appInfoRegion.h-4), ColorF(Palette::Whitesmoke, 0.5));

}

void ResultScene::drawBg_() const
{
	const auto diffColor = difficultyColor(getData().difficulty);

	Scene::SetBackground(diffColor);

	Scene::Rect().stretched(0, -Scene::Height() * 0.25).movedBy(0, -Scene::Height() * 0.25).draw(Arg::top = ColorF(1, 0.1), Arg::bottom = ColorF(1, 0));
	Scene::Rect().stretched(0, -Scene::Height() * 0.25).movedBy(0, Scene::Height() * 0.25).draw(Arg::top = ColorF(0, 0), Arg::bottom = ColorF(0, 0.2));

	// 動く背景

	const auto NX = 8;
	const auto NY = 6;

	for (int iy = -1; iy <= NY; iy++)
	{
		for (int ix = -1; ix <= NX; ix++)
		{
			const auto x = (1 - Periodic::Sawtooth0_1(1s)) * (Scene::Width() / NX) + ix * Scene::Width() / NX;
			const auto y = (Periodic::Sawtooth0_1(1s)) * (Scene::Height() / NY) + iy * Scene::Height() / NY;

			if ((ix + iy) % 2 == 0)
			{
				const auto s = Periodic::Sine0_1(1s);
				Shape2D::Star(56, Vec2(x, y), Scene::Time() * 0.1).draw(ColorF(0.95, 0.1 + 0.2 * s));
			}
			else
			{
				const auto s = 1.0 - Periodic::Sine0_1(1s);
				Shape2D::Star(42, Vec2(x, y), Scene::Time() * 0.07).draw(ColorF(0.95, 0.1 + 0.2 * s));
			}
		}
	}
}
