# include "MainScene.h"
# include "ExpandingCircle.h"

MainScene::MainScene(const InitData& init)
	: IScene{ init }
{
	// 非同期データ取得開始
	beginFetchPokemonInfo_();

	fetchedJson_ = false;
	inputText_ = answer_ = U"";
	question_ = 1;

	// レイアウト

	errorMessageArea_ = RectF{ Arg::center = Scene::CenterF(), Scene::Size() - SizeF(128, 480) };
	flavorArea_ = RectF{ Arg::center = Scene::CenterF().movedBy(0, -128), SizeF(Scene::Width() - 128, Scene::Height() - 720) };
	nameArea_ = RectF{ Arg::center = Vec2(Scene::CenterF().x - 96, Scene::Height() - 640 / 2 / 2 - 128 / 2 - 48), SizeF(1080, 160)};
	logoArea_ = RectF{ 0, 0, Scene::Width(), 100 };
	pokemonImagePos_ = nameArea_.rightCenter().movedBy(80, -108);

	for (int i : step(5))
	{
		ballPosList_.push_front(logoArea_.rightCenter().movedBy(-48 - i * 28 * 2, 0));
	}

	for (int i = 0; i < difficultyCount(); i++)
	{
		difficultyIcons_.push_back(difficultyIcon(static_cast<Difficulty>(i), 80));
	}

	keyboardIcon_ = Texture{ 0xf11c_icon, 64 };

	swScene_.restart();

	// メインシーンは大きく4つのフェーズから構成する
	// Fetching       : PokeAPI などからデータを取ってきて検証する
	// QuestionNumber : "第〇問" と表示する（Fetching 中にも表示してるけど）。時間稼ぎ用
	// Thinking       : ユーザーが答えを入力する。maxRemainTime() 秒間
	// Judge          : 答えを表示して、合ってるか確かめる。
	//                  クリア条件を満たしてたらシーン移行、そうでなければ Fetching へ
	sw_.add(U"Fetching");
	sw_.add(U"QuestionNumber");
	sw_.add(U"Thinking");
	sw_.add(U"Judge");
	sw_.restart();
}

void MainScene::update()
{
	effect_.update();

	ballFlash_.update();

	// ESCキーに関すること

	if (not processEscapeKey_())
	{
		return;
	}

	if (sw_.runningTag() == U"Fetching")
	{
		// ロード中

		if (fetchedJson_ && task3_.isReady() && task3_.getResponse().isOK())
		{
			// 画像ダウンロード終了、次のフェーズへ

			pokemonImage_ = Texture{ PokeAPI::PokemonImagePath };

			fetchedJson_ = false;
			inputText_ = answer_ = U"";
			sw_.next();
		}

		if (task1_.isReady() && task2_.isReady() && not fetchedJson_)
		{
			// JSON取得完了、画像取得へ

			pokemonInfo_ = PokeAPI::flavorInfo(task1_, task2_);
			fetchedJson_ = true;

			if (pokemonInfo_)
			{
				task3_ = PokeAPI::fetchPokemonImageAsync(pokemonInfo_->imageUrl);
			}
			else
			{
				// データ取得できなかったのでロード中止

				fetchedJson_ = false;
				inputText_ = answer_ = U"";
				sw_.next();
			}
		}
	}
	else
	{
		// ロード中じゃなくて…

		if (not pokemonInfo_)
		{
			// データ取得失敗してたらエラーメッセージ

			if (sw_.running().sF() > 3.0 && KeyEnter.down())
			{
				beginFetchPokemonInfo_();

				fetchedJson_ = false;
				inputText_ = answer_ = U"";
				sw_.restart();
			}

			return;
		}
	}

	if (sw_.runningTag() == U"QuestionNumber")
	{
		if (sw_.running().sF() > 1.3)
		{
			sw_.next();
			swFlavorText_.restart();
		}
	}

	if (sw_.runningTag() == U"Thinking")
	{
		// 名前入力を受け付け
		TextInput::UpdateText(inputText_, TextInputMode::AllowBackSpaceDelete);

		if (not inputText_.isEmpty() && TextInput::GetEditingText().isEmpty() && not swInputDecided_.isStarted())
		{
			// 名前入力を確定してからの経過時間を計測スタート
			swInputDecided_.restart();
		}
		else if (inputText_.isEmpty() || not TextInput::GetEditingText().isEmpty())
		{
			swInputDecided_.reset();
		}

		// ①名前入力を確定後、少し経過してからエンターが押されたので答え合わせへ
		//   （即答え合わせにならないための時間制限）
		// ②残り時間がなくなったので答え合わせへ

		if ((swInputDecided_.sF() > 0.3 && KeyEnter.down()) ||
			(maxRemainTime() - sw_.running().s() < 0))
		{
			// 考えタイム終了

			answer_ = inputText_;

			if ((answer_ == pokemonInfo_->name)
#ifdef DEBUGMODE
				|| (answer_ == U"あ")
#endif
				)
			{
				if (ball_ < 5)
				{
					ball_++;

					// ボール増えたエフェクト
					ballFlash_.start(ballPosList_[ball_ - 1], BallFlashType::Receive);

					correct_++;
				}
			}
			else
			{
				if (ball_ > 0)
				{
					ball_--;

					// ボール減ったエフェクト
					ballFlash_.start(ballPosList_[ball_], BallFlashType::Lose);
				}
			}

			sw_.next();

			// ポケモン画像の位置にエフェクト
			addPokemonImageEffect_();
		}
	}
	else if (sw_.runningTag() == U"Judge")
	{
		if (sw_.running().sF() > 1.0 && KeyEnter.down())
		{
			// 答え合わせが1秒経過したあとエンターが押されたので次の問題へ
			// ただしボールが5つ集まってたらリザルトシーンへ

			if (ball_ >= 5)
			{
				getData().questionCount = question_;
				getData().elapsedTime = swScene_.sF();

				changeScene(U"ResultScene", 2000ms);
			}
			else
			{
				// 次のポケモンのデータを非同期取得開始
				beginFetchPokemonInfo_();

				fetchedJson_ = false;
				inputText_ = answer_ = U"";
				question_++;
				sw_.next();
			}
		}
	}
}

void MainScene::draw() const
{
	const ColorF MainBgColor{ 0.96, 0.94, 0.96 };

	Scene::SetBackground(MainBgColor);

	drawBg_();

	// ロード中

	if (sw_.runningTag() == U"Fetching")
	{
		drawLoading_();

		drawQuestionNumber_();

		return;
	}
	else
	{
		// ちゃんとデータ取ってこれたのか？

		if (not pokemonInfo_)
		{
			drawError_();

			return;
		}
	}

	if (sw_.runningTag() == U"QuestionNumber")
	{
		drawQuestionNumber_();

		return;
	}

	drawFlavorArea_();

	drawTimer_();

	drawNameArea_();

	drawPokemonImage_();

	drawLogo_();

	ballFlash_.draw();

	drawEscapeWarning_();
}

int MainScene::maxRemainTime() const
{
	return 30;
}

int MainScene::randomPokemonId() const
{
	return Random(1, maxPokemonId(getData().difficulty));
}

void MainScene:: beginFetchPokemonInfo_()
{
	int id = randomPokemonId();

	// 直近の10個と近すぎるIDが出たら再抽選
	if (recentId_.includes_if([&id](int x) { return abs(id - x) < 3; }))
	{
		id = randomPokemonId();
	}

	recentId_.push_back(id);

	if (recentId_.size() > 10)
	{
		recentId_.pop_front();
	}

	// 非同期でPokeAPIのJSONを取得開始

	task1_.cancel();
	task2_.cancel();
	task3_.cancel();

	task1_ = PokeAPI::fetchPokemonSpeciesJsonAsync(id);
	task2_ = PokeAPI::fetchPokemonJsonAsync(id);
}

bool MainScene::processEscapeKey_()
{
	if ((sw_.runningTag() == U"Thinking" || sw_.runningTag() == U"Judge"))
	{
		// ESC長押し2sでタイトルへ戻れる

		if (KeyEscape.pressedDuration() >= (2s + 0.4s + 0.1s))
		{
			changeScene(U"TitleScene", 0s);

			return false; // このフレームでの、以降のupdate処理を中断
		}

		// ESCキーが一瞬押されたとき、タイトルに戻れる案内を小さく表示

		if (KeyEscape.down())
		{
			swEscDown_.restart();
		}
	}

	return true;
}

void MainScene::addPokemonImageEffect_()
{
	// ポケモン画像の位置にエフェクト

	const auto effectColor = (answer_ == pokemonInfo_->name) ? ColorF(U"#03b289") : ColorF(U"#d34026");

	for (int i : step(3))
	{
		effect_.add<ExpandingCircle>(pokemonImagePos_, effectColor, i * 0.12);
	}
}

void MainScene::drawBg_() const
{
	// 動く背景

	const auto NX = 16;
	const auto NY = 12;

	for (int iy = 0; iy <= NY; iy++)
	{
		for (int ix = 0; ix <= NX; ix++)
		{
			const auto x = Periodic::Sawtooth0_1(1s) * (Scene::Width() / NX) + ix * Scene::Width() / NX;
			const auto y = Periodic::Sawtooth0_1(1s) * (Scene::Height() / NY) + iy * Scene::Height() / NY;

			if ((ix + iy) % 2 == 0)
				RectF(Arg::center = Vec2(x, y), SizeF(48, 48)).rotated(45_deg).draw(ColorF(0.6, 0.3, 0.6, 0.1));
			else
				Circle(Arg::center = Vec2(x, y), 12).draw(ColorF(0.6, 0.3, 0.6, 0.1));
		}
	}
}

void MainScene::drawQuestionNumber_() const
{
	Scene::Rect().draw(ColorF(Palette::Blueviolet, 0.1));

	const auto text = U"第 {} 問"_fmt(question_);
	const auto textFontSize = 140;
	const ColorF textColor = Palette::Blueviolet;

	const auto t = (sw_.runningTag() == U"Fetching") ? 0.0 : sw_.running().sF();
	const auto alpha = (t < 0.9) ? 1.0 : 1.0 - EaseInCubic((t - 0.9) / 0.4);

	const auto region = FontAsset(U"sys")(text).regionAt(textFontSize, Scene::CenterF());

	region.stretched(64, 24)
		.draw(ColorF(Palette::Whitesmoke, 0.3))
		.drawFrame(18, ColorF(textColor, alpha));

	FontAsset(U"sys")(text).drawAt(textFontSize, Scene::CenterF(), ColorF(textColor, alpha));
}

void MainScene::drawLoading_() const
{
	const auto text = U"ロード中…";
	const auto textFontSize = 80;
	const ColorF textColor = blinkColor(ColorF(Palette::Blueviolet, 0.5), ColorF(Palette::Blueviolet, 0.3), 200ms);

	FontAsset(U"sys")(text).drawAt(textFontSize, Scene::CenterF().movedBy(40, 240), textColor);
}

void MainScene::drawError_() const
{
	if (sw_.running().sF() > 3.0)
	{
		const auto msgText = U"なんか　失敗したみたい。\nエンターで　リトライ";
		const auto msgFontSize = 80;
		const auto msgPadding = 32;
		const auto errorColor = Palette::Orangered;

		RoundRect{ errorMessageArea_, 16.0 }.drawFrame(16, errorColor);

		FontAsset(U"flavor")(msgText).draw(msgFontSize, errorMessageArea_.stretched(-msgPadding), errorColor);
	}
}

void MainScene::drawMonsterBall_(double r, const Vec2& pos) const
{
	// LineとCircleでがんばってモンスターボールを描く…

	// 基本サイズ: r=24
	const double scale = r / 24.0;

	Circle(Arg::center = pos, r).draw(Palette::Red);
	Circle(Arg::center = pos, r).drawPie(90_deg, 180_deg, Palette::White);

	const auto glowShift = -13 * scale;
	const auto glowR = 4 * scale;
	Circle(Arg::center = pos.movedBy(glowShift, glowShift), glowR).draw(ColorF(1, 1));

	const auto centerLineWidth = 4 * scale;
	const auto lineColor = ColorF(0.2);
	Line(pos.movedBy(-r + centerLineWidth / 2, 0), pos.movedBy(r - centerLineWidth / 2, 0)).draw(centerLineWidth, lineColor);

	const auto centerCircleR = 10 * scale;
	Circle(Arg::center = pos.movedBy(0, 4), centerCircleR).draw(ColorF(0, 0.1)); //shadow

	const auto centerCircleLineWidth = 4 * scale;
	Circle(Arg::center = pos, centerCircleR)
		.draw(Palette::White)
		.drawFrame(centerCircleLineWidth, lineColor);

	const auto centerCircleR2 = 6 * scale;
	const auto centerCircleLineWidth2 = 1.5 * scale;
	Circle(Arg::center = pos, centerCircleR2)
		.draw(Palette::White)
		.drawFrame(centerCircleLineWidth2, ColorF(0.6));

	const auto outlineFrameWidth = 3 * scale;
	Circle(Arg::center = pos, r).drawFrame(outlineFrameWidth, lineColor);
}

void MainScene::drawLogo_() const
{
	const auto logoText = U"ポケモンフレーバーはかせ";
	const auto logoRegion = FontAsset(U"titleSmall")(logoText).region();
	const auto logoTextColor = Palette::Whitesmoke;
	const auto logoAreaColor = Palette::Blueviolet;
	logoArea_.draw(logoAreaColor);

	const auto scaleX = 0.78;
	auto penPos = Scene::Rect().topCenter().movedBy(0, 48) - SizeF(logoRegion.w * scaleX, logoRegion.h) / 2;

	for (const auto& glyph : FontAsset(U"titleSmall").getGlyphs(logoText))
	{
		glyph.texture.scaled(scaleX, 1.1).draw(penPos + glyph.getOffset() + Vec2(4, 4), ColorF(0, 0, 0, 0.1));
		glyph.texture.scaled(scaleX, 1.1).draw(penPos + glyph.getOffset(), logoTextColor);
		penPos.x += glyph.xAdvance * scaleX;
	}

	// 難易度表示

	const auto diffColor = difficultyColor(getData().difficulty);
	const auto iconPos = logoArea_.leftCenter().movedBy(56, 0);
	const auto iconColor = Palette::White;

	Circle(Arg::center = iconPos, 40)
		.draw(diffColor)
		.drawFrame(4.0, Palette::White);
	Circle(Arg::center = iconPos.movedBy(4, 4), 40).draw(ColorF(0, 0, 0, 0.1));

	difficultyIcons_[static_cast<int>(getData().difficulty)].resized(54).drawAt(iconPos, Palette::White);


	// モンスターボール５つ分の枠

	RoundRect{ Arg::center = ballPosList_[2], SizeF(ballPosList_[4].x - ballPosList_[0].x + 2 * 24/*R*/ + 2 * 16/*Pad*/, 2 * 24/*R*/ + 16/*Pad*/), 48}
		.draw(Arg::top = ColorF(0, 0.2), Arg::bottom = ColorF(0, 0.1));

	for (const auto& ballPos : ballPosList_)
	{
		Circle(Arg::center = ballPos, 24).draw(ColorF(0.15, 0.1, 0.15));
	}

	// 持ってるモンスターボール

	for (int i : step(ball_))
	{
		drawMonsterBall_(24, ballPosList_[i]);
	}


	// 正答数など

	if (false)
	{
		if (sw_.runningTag() == U"Judge")
		{
			const auto statusText = U"成績：{} / {}"_fmt(correct_, question_);
			const auto statusRegion = FontAsset(U"sys")(statusText).region();
			const auto statusFontSize = 36;
			const auto statusCenter = logoArea_.rightCenter() - Vec2(statusRegion.w / 2, 0);
			const auto statusColor = Palette::Whitesmoke;

			FontAsset(U"sys")(statusText).drawAt(statusFontSize, statusCenter + Vec2(4, 4), ColorF(0, 0, 0, 0.1));
			FontAsset(U"sys")(statusText).drawAt(statusFontSize, statusCenter, statusColor);
		}
	}
}

void MainScene::drawFlavorArea_() const
{
	const auto flavorBgColor = ColorF(1, 0.97, 1);
	const auto flavorFrameWidth = 10;
	const auto flavorFrameTopColor = ColorF(Palette::Blueviolet, 0.6);
	const auto flavorFrameBottomColor = ColorF(Palette::Blueviolet, 0.8);

	const RoundRect flavorArea(Arg::center = flavorArea_.center(), flavorArea_.size, 16.0);

	flavorArea
		.draw(flavorBgColor)
		.drawFrame(flavorFrameWidth, Arg::top = flavorFrameTopColor, Arg::bottom = flavorFrameBottomColor)
		.drawFrame(2, Palette::Blueviolet);

	const auto flavorLength = (int)(swFlavorText_.msF() * 0.03);
	const auto flavor = pokemonInfo_->flavor.replaced(pokemonInfo_->name, U"■■■■");
	String flavorText = flavor.substr(0, flavorLength);
	if (flavorLength < flavor.length())
	{
		flavorText += U"■";
	}

	const auto flavorShadow = TextStyle::Shadow(Vec2(2, 2), ColorF(0, 0, 0, 0.1));
	const auto flavorFontSize = 64;
	const auto flavorPadding = 24;
	const auto flavorColor = ColorF(0.2, 0.2, 0.2);

	FontAsset(U"flavor")(U"▶").draw(flavorShadow, flavorFontSize, flavorArea.rect.stretched(-flavorPadding), Palette::Blueviolet);
	FontAsset(U"flavor")(U"　" + flavorText).draw(flavorShadow, flavorFontSize, flavorArea.rect.stretched(-flavorPadding), flavorColor);

}

void MainScene::drawNameArea_() const
{
	// 枠線

	const auto nameBgColor = ColorF(1.0, 0.97, 1.0);
	const auto nameFrameWidth = 10;
	const auto nameFrameTopColor = ColorF(Palette::Blueviolet, 0.7);
	const auto nameFrameBottomColor = ColorF(Palette::Blueviolet, 0.8);

	const RoundRect nameArea(Arg::center = nameArea_.center(), nameArea_.size, 16.0);

	nameArea
		.draw(nameBgColor)
		.drawFrame(nameFrameWidth, Arg::top = nameFrameTopColor, Arg::bottom = nameFrameBottomColor)
		.drawFrame(2, Palette::Blueviolet);


	// 入力中の名前
	const auto inputName = inputText_ + TextInput::GetEditingText();

	if (sw_.runningTag() == U"Thinking")
	{
		if (inputName.isEmpty())
		{
			// テキストボックスが空

			const auto text = U"ポケモンのなまえは？";
			const auto textFontSize = 96;
			const auto textCenter = nameArea.center();
			const auto textColor = ColorF(0.4, 0.4, 0.4, 0.2);

			FontAsset(U"pokemonName")(text).drawAt(textFontSize, textCenter, textColor);

			// カーソル

			const auto cursorPos = nameArea.center();
			const auto cursorColor = blinkColor(ColorF(0.4, 0.4, 0.4), ColorF(0,0,0,0), 800ms);

			Line(cursorPos + Vec2(0, -48), cursorPos + Vec2(0, 48)).draw(6.0, cursorColor);
		}
		else
		{
			// 入力中……
			// 確定済みと、入力中を色分けして表示する

			const auto region = FontAsset(U"pokemonName")(inputName).region();
			auto penPos = nameArea.center() - region.size / 2;

			for (auto [index, glyph] : Indexed(FontAsset(U"pokemonName").getGlyphs(inputName)))
			{
				const auto charColor = (index >= inputText_.length()) ? ColorF(0.5, 0.5, 0.5) : ColorF(0.12, 0.13, 0.10);

				glyph.texture.draw(penPos + glyph.getOffset(), charColor);
				penPos.x += glyph.xAdvance;
			}

			// カーソル

			const auto cursorPos = nameArea.center().movedBy(region.w / 2 + 8, 0);
			const auto cursorColor = blinkColor(ColorF(0.4, 0.4, 0.4), ColorF(0, 0, 0, 0), 800ms);

			Line(cursorPos + Vec2(0, -48), cursorPos + Vec2(0, 48)).draw(6.0, cursorColor);

			// 確定済みの文字のみだったら「エンターで答え合わせ」と表示

			if (TextInput::GetEditingText().isEmpty() && swInputDecided_.sF() > 0.3)
			{
				drawNextLabel_(U"エンターで　答えあわせ");
			}
		}

		// キーボードアイコン
		const auto kbdIconPos = nameArea_.pos.movedBy(16, 16);
		const auto kbdIconSize = 56 - 8 * Periodic::Sine0_1(400ms);
		const auto kbdIconColor = ColorF(Palette::Whitesmoke, 1.0 - 0.1 * Periodic::Square0_1(400ms));

		Circle(Arg::center = kbdIconPos, kbdIconSize).draw(Palette::Blueviolet);
		keyboardIcon_.scaled(1 - 0.08 * Periodic::Sine0_1(400ms)).drawAt(kbdIconPos, kbdIconColor);


	}
	else if (sw_.runningTag() == U"Judge")
	{
		// 「せいかい！」or「ざんねん！」のスタンプ

		const auto stampPos = nameArea.rect.leftCenter();
		const bool isCorrect = (answer_ == pokemonInfo_->name);
		const ColorF stampColor{ isCorrect ? U"#03b289" : U"#d34026" };
		const auto stampText = isCorrect ? U"せいかい!" : U"ざんねん!\n正解は…";
		const auto stampFontSize = 40;
		const auto stampTextColor = Palette::Whitesmoke;

		Shape2D::NStar(12, 144, 108, stampPos, 0).draw(stampColor);
		FontAsset(U"sys")(stampText).drawAt(stampFontSize, stampPos, stampTextColor);


		// 正解の名前

		const auto name = pokemonInfo_->name;
		const auto nameFontSize = 96;
		const auto namePos = nameArea.center();
		const ColorF nameColor{ 0.12, 0.13, 0.10 };

		FontAsset(U"pokemonName")(name).drawAt(nameFontSize, namePos, nameColor);

		// 1秒後にメッセージ出す

		if (sw_.running().sF() > 1.0)
		{
			if (ball_ >= 5)
			{
				drawNextLabel_(U"おつかれさま!　エンターで　つぎへ");
			}
			else
			{
				drawNextLabel_(U"エンターで　つぎの問題");
			}
		}
	}
}

void MainScene::drawTimer_() const
{
	if (sw_.runningTag() == U"Thinking")
	{
		const auto remainTime = Max(0, maxRemainTime() - sw_.running().s());
		const auto timerFontSize = (remainTime >= 10) ? 256 : ((remainTime > 3) ? 290 : 320);
		const auto timerColor = (remainTime > 5) ? ColorF(U"#ab6ce2") : ColorF(U"#e56083");
		const auto timerText = U"{}"_fmt(remainTime);
		const auto timerPos = nameArea_.rightCenter().movedBy(128, 0);
		const auto outline = TextStyle::Outline(0.4, Palette::White);

		const auto t = Periodic::Sawtooth0_1(1s, sw_.running().sF());
		const auto angle = EaseInOutCubic(t) * Math::TwoPiF;
		const auto angleOffset = sw_.running().sF() * Math::TwoPiF / (maxRemainTime() + 1.0);
		Circle{ Arg::center = timerPos.movedBy(16,0), 180 }.drawArc(angle + angleOffset, Math::TwoPiF - angle, 64, 0, ColorF(timerColor, 0.5));
		Circle{ Arg::center = timerPos.movedBy(16,0), 180 }.drawArc(0, Math::TwoPiF, 64, 0, ColorF(timerColor, 0.5*t));

		FontAsset(U"timer")(timerText).drawAt(outline, timerFontSize, timerPos, timerColor);
	}
}

void MainScene::drawPokemonImage_() const
{
	if (sw_.runningTag() == U"Judge")
	{
		// ポケモンの画像

		const auto scale = (sw_.running().sF() < 0.2) ? 1.0 + 0.2 * Periodic::Jump0_1(0.2s, sw_.running().sF()) : 1.0;
		const auto alpha = Min(1.0, sw_.running().sF() * 4);
		const Vec2 pokemonImageSize{ 640 * scale, 640 * scale };

		pokemonImage_.resized(pokemonImageSize).drawAt(pokemonImagePos_, AlphaF(alpha));
	}
}

void MainScene::drawNextLabel_(StringView text) const
{
	const auto textColor = blinkColor(Palette::Blueviolet, ColorF(Palette::Blueviolet, 0.5), 300ms);
	const auto labelText = text;
	const auto enterTextRegion = FontAsset(U"sys")(labelText).region();
	const auto labelFontSize = 44;
	const auto labelCenter = nameArea_.bottomCenter().movedBy(0, 56);
	const auto labelSize = SizeF(enterTextRegion.w * 1.2, enterTextRegion.h);
	const auto labelTextColor = Palette::Whitesmoke;

	RoundRect{ Arg::center = labelCenter, labelSize, enterTextRegion.h }.draw(textColor);
	FontAsset(U"sys")(labelText).drawAt(labelFontSize, labelCenter, labelTextColor);
}

void MainScene::drawEscapeWarning_() const
{
	// ①ESCキーを長押ししたときにタイトルへ戻る旨と残り時間を表示する

	// ESCキーが押されている時間 (s)
	const auto pressedSec = SecondsF(KeyEscape.pressedDuration()).count();

	// 押されてから猶予時間(0.4s)経ったら表示し始める
	if (pressedSec > 0.4)
	{
		Scene::Rect().draw(ColorF(U"#300a5e").withAlpha(0.7));

		RoundRect{ Scene::Rect().scaled(0.8), 24 }.draw(ColorF(0, 0.4));

		FontAsset(U"sys")(U"タイトルがめんに　戻ります…").drawAt(72, Scene::CenterF(), Palette::Whitesmoke);

		Circle(Arg::center = Scene::CenterF(), 240)
			.drawArc(0_deg, 360_deg * Min(1.0, (pressedSec - 0.4) / 2.0), 80, 0, ColorF(Palette::Whitesmoke, 0.3 - 0.07 * Periodic::Square0_1(150ms)));
	}

	// ②ESCキーを一瞬押ししたときにタイトルへ戻れる旨を案内

	if (swEscDown_.isStarted() && swEscDown_.sF() < 3.0)
	{
		const auto guidanceText = U"エスケープキー長押しで　タイトルに戻れます";
		const auto guidanceFontSize = 28;
		const auto guidancePos = Scene::Rect().bottomCenter().movedBy(0, -32);
		const auto guidanceRegion = FontAsset(U"sys")(guidanceText).regionAt(guidanceFontSize, guidancePos);

		guidanceRegion.draw(ColorF(Palette::Blueviolet, 0.15));
		FontAsset(U"sys")(guidanceText).drawAt(guidanceFontSize, guidancePos, Palette::Blueviolet);
	}
}
