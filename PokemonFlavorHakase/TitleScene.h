#pragma once

class TitleScene : public App::Scene
{
public:

	TitleScene(const InitData& init);

	void update() override;

	void draw() const override;


private:
	Stopwatch swScene_{ StartImmediately::Yes };

	// 選択中の難易度
	int difficulty_ = 0;

	// 難易度ごとのアイコン
	Array<Texture> difficultyIcons_;

	// 難易度を切り替えるときにアイコンを少し拡縮したりする用
	Stopwatch swChangeDifficulty_{ StartImmediately::Yes };

	void drawHeader_() const;

	void drawCopy_() const;

	void drawMenu_() const;

	void drawDifficulty_() const;

};
