#include "pch.h"
#include "Game.h"
#include "Utility.h"
#include "Tower.h"
#include "DataPaths.h"

#include "json_spirit/json_spirit.h"

namespace fs = boost::filesystem;
namespace js = json_spirit;

Game::Game(RenderWindow& win, GlobalStatus& gs)
: window(win), globalStatus(gs), userInterface(window, theme, globalStatus, gameStatus), activeTower(0), running(true)
{ }

void Game::Reset()
{
	enemies.clear();
	towers.clear();
	projectiles.clear();

	LoadLevel(globalStatus.level);

	LoadFromFile(map, levelInfo.map);
	LoadFromFile(imgFoe, "data/models/test.png");
	LoadFromFile(imgTower, "data/models/archer_level1.png");

	theme.LoadTheme(levelInfo.theme);
	userInterface.Reset(levelInfo);

	running = true;
	gameStatus.Reset(globalStatus);
}

// Compare towers by their y position, to ensure lower towers (= higher y pos) are drawn
// later, so the overlap is displayed correctly.
static bool CompTowerY(const Tower& a, const Tower& b)
{
	return a.GetPosition().y < b.GetPosition().y;
}

static bool ShouldRemoveEnemy(const std::shared_ptr<Enemy>& e)
{
	return e->IsIrrelevant();
}

void Game::Run()
{
	Event event;
	while (window.GetEvent(event)) {
		if (DefaultHandleEvent(window, event))
			continue;
		if (activeTower && activeTower->HandleEvent(event)) {
			if (activeTower->IsPlaced()) {
				map.PlaceTower(map.PostionToTowerPos(activeTower->GetPosition()));
				activeTower = 0;
				boost::sort(towers, CompTowerY);
			}
			else if (activeTower->StopPlace()) {
				towers.pop_back();
				activeTower = 0;
			}
			continue;
		}

		if (event.Type == Event::KeyReleased) {
			switch (event.Key.Code) {
			case Key::G:
				AddEnemy();
				break;
			case Key::T:
				AddTower();
				break;
			case Key::F2:
				map.ToggleOverlay();
				break;
			case Key::F3:
				map.DebugToggleTowersAnywhere();
				break;
			}
		}
	}

	float elapsed = window.GetFrameTime();

	for (auto it = enemies.begin(); it != enemies.end(); ++it) {
		std::shared_ptr<Enemy> e = *it;
		e->Update(elapsed);
		if (e->IsAtTarget() && !e->DidStrike()) {
			e->Strike();
			LooseLife();
		}
	}
	for (auto it = projectiles.begin(); it != projectiles.end(); ++it)
		it->Update(elapsed);
	for (auto it = towers.begin(); it != towers.end(); ++it)
		it->Update(elapsed);

	projectiles.erase(boost::remove_if(projectiles, boost::bind(&Projectile::DidHit, _1)), projectiles.end());
	enemies.erase(boost::remove_if(enemies, ShouldRemoveEnemy), enemies.end());

	userInterface.Update();

	window.Clear();
	map.Draw(window);
	for (auto it = towers.begin(); it != towers.end(); ++it) {
		it->DrawRangeCircle(window);
		window.Draw(*it);
	}
	for (auto it = enemies.begin(); it != enemies.end(); ++it) {
		(*it)->DrawHpBar(window);
		window.Draw(*(*it));
	}
	
	for (auto it = projectiles.begin(); it != projectiles.end(); ++it)
		window.Draw(*it);

	userInterface.Draw();

	window.Display();
}

void Game::LooseLife()
{
	gameStatus.lives--;
	if (gameStatus.lives == 0) {
		running = false;
	}
}

bool Game::IsRunning()
{
	return running;
}

State Game::GetNextState()
{
	return ST_QUIT;
}

void Game::LoadLevel(const std::string& level)
{
	fs::path levelPath = GetLevelPath(level);
	fs::path levelDef = levelPath / LevelDefinitionFile;

	std::ifstream in(levelDef.string());
	js::mValue rootValue;
	try {
		js::read_or_throw(in, rootValue);
	}
	catch (js::Error_position err) {
		throw GameError() << ErrorInfo::Desc("Invalid json file") << ErrorInfo::Note(err.reason_) << boost::errinfo_at_line(err.line_) << boost::errinfo_file_name(levelDef.string());
	}

	if (rootValue.type() != js::obj_type)
		throw GameError() << ErrorInfo::Desc("Root value is not an object");

	js::mObject rootObj = rootValue.get_obj();

	levelInfo.name = rootObj["name"].get_str();
	levelInfo.map = rootObj["map"].get_str();
	levelInfo.theme = rootObj["theme"].get_str();
}

void Game::AddTower()
{
	if (activeTower)
		return;

	Tower t(&map, &enemies, &projectiles);
	t.SetImage(imgTower);

	const Input& input = window.GetInput();
	t.SetPosition(static_cast<float>(input.GetMouseX()), static_cast<float>(input.GetMouseY()));
	t.SetSize(imgTower.GetWidth(), imgTower.GetHeight());

	towers.push_back(t);
	activeTower = &towers.back();
}

void Game::AddEnemy()
{
	std::shared_ptr<Enemy>e(new Enemy(&map));
	e->SetImage(imgFoe);
	e->SetOffset(1);
	e->SetSize(50, 50);
	e->SetFrameTime(.2f);
	e->SetNumFrames(4);

	e->SetPosition(map.BlockToPosition(Vector2i(0, 7)));

	e->SetSpeed(50);
	e->SetTarget(Vector2i(24, 17));

	enemies.push_back(e);
}
