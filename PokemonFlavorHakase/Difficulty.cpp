# include "Difficulty.h"

int difficultyCount()
{
	return 4;
}

int maxPokemonId(Difficulty difficulty)
{
	switch (difficulty)
	{
	case Difficulty::Green: return 151;
	case Difficulty::Gold: return 251;
	case Difficulty::Diamond: return 493;
	case Difficulty::Expert: return 898;
	}

	return 151;
}

Texture difficultyIcon(Difficulty difficulty, int size)
{
	switch (difficulty)
	{
	case Difficulty::Green:
		return Texture{ 0xf06c_icon, size };
	case Difficulty::Gold:
		return Texture{ 0xF0674_icon, (int)(size*1.1) };
	case Difficulty::Diamond:
		return Texture{ 0xf3a5_icon, size };
	case Difficulty::Expert:
		return Texture{ 0xf521_icon, size };
	}

	return Texture{ 0xf06c_icon, size };
}

ColorF difficultyColor(Difficulty difficulty)
{
	switch (difficulty)
	{
	case Difficulty::Green: return ColorF(U"#03a57a");
	case Difficulty::Gold: return ColorF(U"#cead08");
	case Difficulty::Diamond: return ColorF(U"#1c90dd");
	case Difficulty::Expert: return ColorF(U"#ea60b5");
	}

	return ColorF(U"#03a57a");
}

StringView difficultyName(Difficulty difficulty)
{
	switch (difficulty)
	{
	case Difficulty::Green: return U"グリーン";
	case Difficulty::Gold: return U"ゴールド";
	case Difficulty::Diamond: return U"ダイヤ";
	case Difficulty::Expert: return U"エキスパ";
	}

	return U"グリーン";
}
