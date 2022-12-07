# include <Siv3D.hpp> // OpenSiv3D v0.6.6

# include "TitleScene.h"
# include "MainScene.h"
# include "ResultScene.h"

void Main()
{
	Window::SetTitle(U"ポケモンフレーバーはかせ v{}"_fmt(appVersion()));

	// ESCキーで終了しない
	System::SetTerminationTriggers(UserAction::CloseButtonClicked);

	// シーンのサイズを 1600x1200 に設定 / 手動リサイズ可能

	Window::SetStyle(WindowStyle::Sizable);
	Scene::SetResizeMode(ResizeMode::Keep);

	const auto sceneSize = Size(1600, 1200);
	Scene::Resize(sceneSize);

	// ウィンドウがディスプレイからはみ出す場合は適当に小さくする

	const auto workAreaSize = System::GetCurrentMonitor().workArea.size;

	if (sceneSize.y > workAreaSize.y)
	{
		const auto scale = 0.9 * workAreaSize.y / sceneSize.y;
		Window::ResizeActual(sceneSize.x * scale, sceneSize.y * scale);
	}
	else if (sceneSize.x > workAreaSize.x)
	{
		const auto scale = 0.9 * workAreaSize.x / sceneSize.x;
		Window::ResizeActual(sceneSize.x * scale, sceneSize.y * scale);
	}
	else
	{
		Window::ResizeActual(sceneSize);
	}


	// アセット

	FontAsset::Register(U"title", 150, Typeface::Black);
	FontAsset::Register(U"titleSmall", 64, Typeface::Black);
	FontAsset::Register(U"flavor", FontMethod::MSDF, 40, Typeface::Medium);
	FontAsset::Register(U"pokemonName", 96, Typeface::Bold);
	FontAsset::Register(U"sys", FontMethod::MSDF, 40, Typeface::Heavy);
	FontAsset::Register(U"timer", FontMethod::MSDF, 64, Typeface::Black, FontStyle::Italic);

#ifndef DEBUGMODE
	// フレーバーテキストのフォント生成に時間がかかるので、英数字ひらカタと教育漢字のうち小２まで（適当）をプリロードしておく

	const String preloadText = U"０１２３４５６７８９ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺぁあぃいぅうぇえぉおかがきぎくぐけげこごさざしじすずせぜそぞただちぢっつづてでとどなにぬねのはばぱひびぴふぶぷへべぺほぼぽまみむめもゃやゅゆょよらりるれろわをんァアィイゥウェエォオカガキギクグケゲコゴサザシジスズセゼソゾタダチヂッツヅテデトドナニヌネノハバパヒビピフブプヘベペホボポマミムメモャヤュユョヨラリルレロワンヴ一右雨円王音下火花貝学気九休玉金空月犬見五口校左三山子四糸字耳七車手十出女小上森人水正生青夕石赤千川先早草足村大男竹中虫町天田土二日入年白八百文木本名目立力林六引羽雲園遠何科夏家歌画回会海絵外角楽活間丸岩顔汽記帰弓牛魚京強教近兄形計元言原戸古午後語工公広交光考行高黄合谷国黒今才細作算止市矢姉思紙寺自時室社弱首秋週春書少場色食心新親図数西声星晴切雪船線前組走多太体台地池知茶昼長鳥朝直通弟店点電刀冬当東答頭同道読内南肉馬売買麦半番父風分聞米歩母方北毎妹万明鳴毛門夜野友用曜来里理話";
	FontAsset::LoadAsync(U"flavor", preloadText);

	// ロード中

	Scene::SetBackground(Color(30));

	while (System::Update())
	{
		FontAsset(U"sys")(U"ロード中…").drawAt(Scene::CenterF(), ColorF(1, 1, 1, 0.7 + 0.2 * Periodic::Square0_1(300ms)));

		if (FontAsset::IsReady(U"flavor"))
		{
			break;
		}
	}
#endif

	// シーン

	App manager;
	manager.add<TitleScene>(U"TitleScene");
	manager.add<MainScene>(U"MainScene");
	manager.add<ResultScene>(U"ResultScene");
	manager.setFadeColor(Palette::Whitesmoke);
	manager.changeScene(U"TitleScene", 0ms);

	while (System::Update())
	{
		if (not manager.update())
		{
			break;
		}
	}
}
