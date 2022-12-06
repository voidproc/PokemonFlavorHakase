#pragma once

enum class Difficulty : int
{
	Green = 0,
	Gold = 1,
	Diamond = 2,
	Expert = 3,
};

int difficultyCount();

int maxPokemonId(Difficulty difficulty);

Texture difficultyIcon(Difficulty difficulty, int size);

ColorF difficultyColor(Difficulty difficulty);

StringView difficultyName(Difficulty difficulty);
