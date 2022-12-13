#pragma once

# include "BallFlash.h"


class MainScene : public App::Scene
{
public:

	MainScene(const InitData& init);

	void update() override;

	void draw() const override;

private:
	// レイアウト
	RectF errorMessageArea_;
	RectF flavorArea_;
	RectF nameArea_;
	Vec2 pokemonImagePos_;
	RectF logoArea_;
	Array<Vec2> ballPosList_;

	// 難易度ごとのアイコン
	Array<Texture> difficultyIcons_;

	// キーボードアイコン
	Texture keyboardIcon_;

	// 直近に取得したポケモンID
	Array<int> recentId_;

	// APIからの取得データ
	Optional<PokeAPI::PokemonInfo> pokemonInfo_;
	Texture pokemonImage_;

	// JSON取得済み？(済みなら画像取得へ)
	bool fetchedJson_ = false;

	// 非同期ロード用タスク
	AsyncHTTPTask taskFetchJson1_;
	AsyncHTTPTask taskFetchJson2_;
	AsyncHTTPTask taskFetchImage_;

	// 入力データ
	String inputText_ = U"";
	String answer_ = U"";

	// 正解数
	int correct_ = 0;

	// 問題数
	int question_ = 1;

	// モンスターボールの数
	// 正解で１つ付与、間違うと１つ没収
	int ball_ = 0;

	// ボール点滅エフェクト
	BallFlash ballFlash_;

	// 入力確定タイマー
	Stopwatch swInputDecided_;

	// ゲームスタートからの経過時間
	Stopwatch swScene_;

	// ESCが一瞬押されたときにタイトルへ戻れる案内を表示
	Stopwatch swEscDown_;

	// フレーバーテキスト表示用
	Stopwatch swFlavorText_;

	// シーン全体の時間管理
	StopwatchSequence sw_;

	Effect effect_;

	int maxRemainTime_() const;
	int randomPokemonId_();
	void beginFetchPokemonInfo_();

	bool processEscapeKey_();
	void processFetching_();
	void nextPhase_();
	void resetPhase_();
	void processShowQuestionNumber_();
	void checkInputDecided_();
	void exitThinking_();
	void changeToResultScene_();
	void nextQuestion_();

	void addPokemonImageEffect_();

	void drawBg_() const;
	void drawLoading_() const;
	void drawError_() const;
	void drawQuestionNumber_() const;
	void drawMonsterBall_(double r, const Vec2& pos) const;
	void drawHeader_() const;
	void drawFlavorArea_() const;
	void drawNameFrame_() const;
	void drawInputName_() const;
	void drawKeyboardIcon_() const;
	void drawStamp_() const;
	void drawAnswerName_() const;
	void drawTimer_() const;
	void drawPokemonImage_() const;
	void drawNextLabel_(StringView text) const;
	void drawEscapeWarning_() const;
};
