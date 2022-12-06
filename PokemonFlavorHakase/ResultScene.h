#pragma once

class ResultScene : public App::Scene
{
public:

	ResultScene(const InitData& init);

	void update() override;

	void draw() const override;

private:

	Stopwatch swScene_{ StartImmediately::Yes };

	void drawBg_() const;

};
