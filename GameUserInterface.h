#ifndef GAME_USER_INTERFACE_H
#define GAME_USER_INTERFACE_H

#include "GlobalStatus.h"
#include "GameStatus.h"
#include "LevelMetaInfo.h"
#include "Theme.h"

class GameUserInterface
{
	const Theme& theme;
	RenderWindow& window;

	GlobalStatus& globalStatus;
	GameStatus& gameStatus;

	Sprite topPanel;

	sf::String levelName;
	sf::String lives;
	sf::String countdown;

	bool showCountdown;

	sf::Clock textUpdateClock;

public:
	GameUserInterface(RenderWindow& window, const Theme& theme, GlobalStatus& globalStatus, GameStatus& gameStatus);

	void Update();
	void Draw();
	void Reset(const LevelMetaInfo& metaInfo);

private:
	void UpdateText();
};

#endif //GAME_USER_INTERFACE_H