#pragma once

class PokeAPI
{
public:

	static constexpr StringView ApiBaseUrl = U"https://pokeapi.co/api/v2/";
	static constexpr StringView SpeciesJsonPath = U"pokemon-species.json";
	static constexpr StringView PokemonJsonPath = U"pokemon.json";
	static constexpr StringView PokemonImagePath = U"image";

	struct PokemonInfo
	{
		String name;
		String flavor;
		String imageUrl;
	};

	static AsyncHTTPTask fetchPokemonSpeciesJsonAsync(int id)
	{
		const URL url = U"{}pokemon-species/{}/"_fmt(ApiBaseUrl, id);

		auto task = SimpleHTTP::SaveAsync(url, SpeciesJsonPath);
		return task;
	}

	static AsyncHTTPTask fetchPokemonJsonAsync(int id)
	{
		const URL url = U"{}pokemon/{}/"_fmt(ApiBaseUrl, id);

		auto task = SimpleHTTP::SaveAsync(url, PokemonJsonPath);
		return task;
	}

	static AsyncHTTPTask fetchPokemonImageAsync(const String imageUrl)
	{
		auto task = SimpleHTTP::SaveAsync(imageUrl, PokemonImagePath);
		return task;
	}

	static Optional<PokemonInfo> flavorInfo(AsyncHTTPTask speciesJsonTask, AsyncHTTPTask pokemonJsonTask)
	{

		if (not speciesJsonTask.isReady() || not speciesJsonTask.getResponse().isOK())
		{
			return none;
		}

		if (not pokemonJsonTask.isReady() || not pokemonJsonTask.getResponse().isOK())
		{
			return none;
		}

		if (not FileSystem::Exists(SpeciesJsonPath) || FileSystem::FileSize(SpeciesJsonPath) == 0)
		{
			return none;
		}

		if (not FileSystem::Exists(PokemonJsonPath) || FileSystem::FileSize(PokemonJsonPath) == 0)
		{
			return none;
		}

		const auto speciesJsonText = TextReader{ SpeciesJsonPath }.readAll();

		const auto speciesJson = JSON::Parse(speciesJsonText);

		if (speciesJson.Invalid())
		{
			return none;
		}

		const auto pokemonJsonText = TextReader{ PokemonJsonPath }.readAll();

		const auto pokemonJson = JSON::Parse(pokemonJsonText);

		if (pokemonJson.Invalid())
		{
			return none;
		}

		Array<String> flavors;

		for (const auto& jsonFlavor : speciesJson[U"flavor_text_entries"].arrayView())
		{
			if (const auto langName = jsonFlavor[U"language"][U"name"].getString(); langName == U"ja")
			{
				auto flavorText = jsonFlavor[U"flavor_text"].getString().replace(U"\n", U"　");
				flavors.push_back(flavorText);
			}
		}

		if (flavors.isEmpty())
		{
			return none;
		}

		String name = U"";

		for (const auto& jsonName : speciesJson[U"names"].arrayView())
		{
			if (const auto langName = jsonName[U"language"][U"name"].getString(); langName == U"ja")
			{
				name = jsonName[U"name"].getString();
			}
		}

		if (not pokemonJson[U"sprites"][U"other"].hasElement(U"official-artwork"))
		{
			return none;
		}

		const auto imageUrl = pokemonJson[U"sprites"][U"other"][U"official-artwork"][U"front_default"].getString();

		return PokemonInfo{ name, Sample(flavors), imageUrl };
	}

private:

	using HttpHeaders = HashTable<String, String>;

};
