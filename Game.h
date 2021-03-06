#ifndef GAME_H
#define GAME_H

#include "GlobalStatus.h"
#include "State.h"
#include "Enemy.h"
#include "Map.h"
#include "Tower.h"
#include "Projectile.h"
#include "Treasure.h"

class Game
{
	RenderWindow& window;
	GlobalStatus& status;

	Image imgBg;
	Sprite bg;

	Image imgFoe;
	std::vector<std::shared_ptr<Enemy>> enemies;

	Image imgTower;
	Tower* activeTower;
	std::vector<Tower> towers;

	std::vector<Projectile> projectiles;

	size_t lives;

	Map map;

public:
	Game(RenderWindow& win, GlobalStatus& gs);

	void Run();
	void Reset();

	bool IsRunning();
	State GetNextState();

private:
	bool running;

	std::string LEVEL(const char *what) const
	{
		return "data/levels/" + status.level + "/" + what;
	}

	void AddTower();
	void AddEnemy();

	void LooseLife();
};

#endif //GAME_H
