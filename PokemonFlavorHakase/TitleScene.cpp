# include "TitleScene.h"

TitleScene::TitleScene(const InitData& init)
	: IScene{ init }
{
	for (int i = 0; i < difficultyCount(); i++)
	{
		difficultyIcons_.push_back(difficultyIcon(static_cast<Difficulty>(i), 108));
	}
}

void TitleScene::update()
{
	if (swScene_.sF() > 3.0)
	{
		// 難易度選択

		if (KeyRight.down())
		{
			difficulty_ = (difficulty_ + 1) % difficultyCount();

			swChangeDifficulty_.restart();
		}
		else if (KeyLeft.down())
		{
			difficulty_ = (difficulty_ - 1 + difficultyCount()) % difficultyCount();

			swChangeDifficulty_.restart();
		}

		// 難易度決定、メインシーンへ

		if (KeyEnter.down())
		{
			getData().difficulty = static_cast<Difficulty>(difficulty_);

			changeScene(U"MainScene", 1.2s);
		}
	}
}

void TitleScene::draw() const
{
	const auto t = swScene_.sF();
	const auto color1 = Color(30);
	const auto color2 = ColorF{ 0.96, 0.94, 0.96 };

	ColorF bgColor;

	if (t < 2.2)
	{
		bgColor = color1;
	}
	else if (t > 3.0)
	{
		bgColor = color2;
	}
	else
	{
		bgColor = color1.lerp(color2, (t - 2.2) / 0.8);
	}

	Scene::SetBackground(bgColor);

	drawLogo_();

	if (swScene_.sF() > 3.0)
	{
		drawCopy_();

		drawMenu_();

		drawDifficulty_();
	}
}

void TitleScene::drawLogo_() const
{
	const String logoText = U"ポケモンフレーバーはかせ";
	const auto logoRegion = FontAsset(U"title")(logoText).region();

	const auto scaleX = 0.78;
	const auto scaleY = 1.1;
	const auto posY = (swScene_.sF() < 2.2) ? 0 : Max(-300.0, -300.0 * EaseInOutCubic((swScene_.sF() - 2.2) / 0.8));
	auto penPos = Scene::CenterF().movedBy(0, posY) - SizeF(logoRegion.w * scaleX, logoRegion.h) / 2;

	for (const auto& glyph : FontAsset(U"title").getGlyphs(logoText))
	{
		const auto shadowOffset = Vec2(4, 4);
		ColorF shadowColor = Alpha(0);
		ColorF logoColor;

		if (swScene_.sF() < 0.8)
		{
			logoColor = Alpha(0);
		}
		else if (swScene_.sF() < 2.5)
		{
			logoColor = ColorF(Palette::Whitesmoke, EaseInOutSine((swScene_.sF() - 0.8) / 1.7));
		}
		else if (swScene_.sF() > 3.0)
		{
			logoColor = Palette::Blueviolet;
			shadowColor = ColorF(0.5, 0, 0.5, 0.08);
		}
		else
		{
			logoColor = Palette::Whitesmoke.lerp(Palette::Blueviolet, (swScene_.sF() - 2.5) / 0.5);
			shadowColor = ColorF(0.5, 0, 0.5, 0.08);
		}

		glyph.texture.scaled(scaleX, scaleY).draw(penPos + glyph.getOffset() + shadowOffset, shadowColor);
		glyph.texture.scaled(scaleX, scaleY).draw(penPos + glyph.getOffset(), logoColor);
		penPos.x += glyph.xAdvance * scaleX;
	}

	// バージョン

	if (swScene_.sF() > 3.0)
	{
		const auto versionText = U"v{}"_fmt(appVersion());
		const auto versionFontSize = 48;
		const auto versionRegion = FontAsset(U"sys")(versionText).region();
		const auto versionCenter = Scene::CenterF().movedBy(0, -300).movedBy(logoRegion.w * scaleX / 2, logoRegion.h / 2).movedBy(-versionRegion.w / 2 - 16, 32);
		const auto versionColor = Palette::Blueviolet;

		FontAsset(U"sys")(versionText).drawAt(versionFontSize, versionCenter, versionColor);
	}
}

void TitleScene::drawCopy_() const
{
	const auto poweredText = U"Powered by OpenSiv3D & PokéAPI.";
	const auto poweredFontSize = 32;
	const auto poweredCenter = Scene::Rect().bottomCenter().movedBy(0, -80);
	const auto poweredColor = Palette::Blueviolet;

	FontAsset(U"sys")(poweredText).drawAt(poweredFontSize, poweredCenter, poweredColor);

	const auto copyText = U"©2022 voidProc / Pokémon: ©Pokémon. ©Nintendo, Creatures Inc., GAME FREAK inc.";
	const auto copyFontSize = 24;
	const auto copyCenter = Scene::Rect().bottomCenter().movedBy(0, -32);
	const auto copyColor = Palette::Blueviolet;

	FontAsset(U"sys")(copyText).drawAt(copyFontSize, copyCenter, copyColor);
}

void TitleScene::drawMenu_() const
{
	const auto menuLabelColor = Palette::Blueviolet;
	const auto menuLabelText = ((int)(Scene::Time()) % 2) == 0 ? U"エンターで　スタート" : U"矢印キーで　レベル設定";
	const auto enterTextRegion = FontAsset(U"sys")(menuLabelText).region();
	const auto menuCenter = Scene::CenterF().movedBy(0, 320);
	const auto menuLabelSize = SizeF(enterTextRegion.w * 1.2, enterTextRegion.h);
	const auto menuFontSize = 44;
	const auto menuTextColor = Palette::Whitesmoke;

	RoundRect{ Arg::center = menuCenter, menuLabelSize, enterTextRegion.h }.draw(menuLabelColor);
	FontAsset(U"sys")(menuLabelText).drawAt(menuFontSize, menuCenter, menuTextColor);
}

void TitleScene::drawDifficulty_() const
{
	const auto iconPos = Scene::CenterF().movedBy(0, 48);
	const auto difficulty = static_cast<Difficulty>(difficulty_);
	const auto color = difficultyColor(difficulty);

	// 難易度切り替えした瞬間に現れるエフェクト

	const auto expandCircleScale = (swChangeDifficulty_.sF() > 0.4) ? 1.0 : 1.0 + 0.3 * EaseOutCubic(swChangeDifficulty_.sF() / 0.4);
	const auto expandCircleAlpha = (swChangeDifficulty_.sF() > 0.4) ? 0.0 : 0.5 * (1.0 - swChangeDifficulty_.sF() / 0.4);

	Circle(Arg::center = iconPos, 108 * expandCircleScale).draw(ColorF(color, expandCircleAlpha));

	// 難易度アイコン背景の円

	const auto iconScale = swChangeDifficulty_.sF() < 0.1 ? 1.0 + 0.2 * Periodic::Jump0_1(0.1s, swChangeDifficulty_.sF()) : 1.0;
	const auto alpha = 1.00 - 0.25 * Periodic::Square0_1(250ms);

	Circle(Arg::center = iconPos, 108 * iconScale).draw(ColorF(color, alpha));

	// 難易度アイコン

	const auto iconColor = Palette::White;

	difficultyIcons_[difficulty_].scaled(iconScale).drawAt(iconPos.movedBy(0, -8), iconColor);

	// 難易度ラベル

	const auto labelFontSize = 32;
	const auto labelColor = ColorF(Palette::White, 0.65);

	FontAsset(U"sys")(difficultyName(difficulty)).drawAt(labelFontSize, iconPos.movedBy(0, 68), labelColor);

	// 左右に移動できることを示す三角形

	const auto nextColor = ColorF(difficultyColor(static_cast<Difficulty>((difficulty_ + 1) % difficultyCount())), alpha);

	Triangle(iconPos.movedBy(240, 0), 48, 90_deg).draw(nextColor);

	const auto prevColor = ColorF(difficultyColor(static_cast<Difficulty>((difficulty_ - 1 + difficultyCount()) % difficultyCount())), alpha);

	Triangle(iconPos.movedBy(-240, 0), 48, -90_deg).draw(prevColor);
}
