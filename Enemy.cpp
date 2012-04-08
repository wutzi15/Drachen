#include "pch.h"
#include "Enemy.h"

float norm(const Vector2f& v)
{
	return sqrt(v.x*v.x + v.y*v.y);
}

void Enemy::Update(float elapsed)
{
	AnimSprite::Update(elapsed);

	if (!path.empty()) {
		Vector2f tgt = path.top();
		Vector2f dist = tgt - GetPosition();

		if (norm(dist) < 5)
			path.pop();
	}
	if (!path.empty()) {
		Vector2f tgt = path.top();
		Vector2f dir = tgt - GetPosition();

		float r = norm(dir);
		dir /= r;

		Move(dir * speed * elapsed);
	}
}

void Enemy::SetTarget(size_t x, size_t y)
{
	FindPath(x, y);
}

struct Node
{
	size_t x, y;
	float g, f;

	Node *parent;

	Node(Vector2i pos)
	: x(pos.x), y(pos.y), g(.0f), f(.0f), parent(0)
	{ }

	Node(size_t x, size_t y)
	: x(x), y(y), g(.0f), f(.0f), parent(0)
	{ }

	bool operator == (const Node& rhs)
	{
		return x == rhs.x && y == rhs.y;
	}

	bool operator != (const Node& rhs)
	{
		return !(*this == rhs);
	}

	float CalcH(const Node& goal)
	{
		return 1.0f * (std::abs(static_cast<int>(x - goal.x)) + std::abs(static_cast<int>(y - goal.y)));
	}
};

struct NodePtrComp
{
	bool operator()(const Node* lhs, const Node *rhs) const
	{
		return lhs->f > rhs->f;
	}
};

void Enemy::FindPath(size_t tgtX, size_t tgtY)
{
	while (!path.empty())
		path.pop();

	NodePtrComp comp;
	std::vector<Node*> allNodes;
	std::vector<Node*> open, closed;

	Node *goal = new Node(tgtX, tgtY);
	allNodes.push_back(goal);

	Node *start = new Node(map->PositionToBlock(GetPosition()));
	allNodes.push_back(start);

	open.push_back(start);
	boost::push_heap(open, comp);

	while (!open.empty() && *open.front() != *goal) {
		Node *cur = open.front();
		boost::pop_heap(open, comp);
		open.pop_back();

		for (int i=0; i < 4; ++i) {
			int x, y;
			switch (i) {
			case 0:
				x = cur->x-1; y = cur->y;
				break;
			case 1:
				x = cur->x+1; y = cur->y;
				break;
			case 2: 
				x = cur->x; y = cur->y-1;
				break;
			case 3:
				x = cur->x; y = cur->y+1;
				break;
			}
			if (x < 0 || y < 0 || (size_t)x >= map->GetWidth() || (size_t)y >= map->GetHeight() || (x == cur->x && y == cur->y) || !map->GetGrid()[x][y])
				continue;

			Node *suc = new Node(x, y);
			allNodes.push_back(suc);

			float newg = cur->g + 1.f;

			std::vector<Node*>::iterator oit, cit;
			for (oit = open.begin(); oit != open.end(); ++oit) {
				if (**oit == *suc)
					break;
			}
			for (cit = closed.begin(); cit != closed.end(); ++cit) {
				if (**cit == *suc)
					break;
			}

			if (oit != open.end()) {
				if ((*oit)->g <= newg)
					continue;
			}
			if (cit != closed.end()) {
				if ((*cit)->g <= newg)
					continue;
			}

			suc->parent = cur;
			suc->g = newg;
			//suc->h = suc->CalcH(*goal);
			suc->f = suc->g + suc->CalcH(*goal);

			if (cit != closed.end()) {
				closed.erase(cit);
			}
			if (oit != open.end()) {
				open.erase(oit);
				boost::make_heap(open, comp);
			}

			open.push_back(suc);
			boost::push_heap(open, comp);
		}

		closed.push_back(cur);
	}

	if (open.empty())
		return;

	Node *n = open.front();

	std::ofstream out("path.txt");

	while (n) {
		out << n->x << "\t" << n->y << "\n";
		path.push(map->BlockToPosition(Vector2i(n->x, n->y)));
		n = n->parent;
	}
	out.close();

	for (size_t i=0; i < allNodes.size(); ++i) {
		delete allNodes[i];
	}
}
