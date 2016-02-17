#ifndef GAME_AI_MCTS_H
#define GAME_AI_MCTS_H

#include <random>
#include <list>
#include <map>
#include <unordered_set>
#include "game-engine/game-engine.h"
#include "tree.h"
#include "board-node-map.h"

class MCTS
{
public:
	MCTS();
	~MCTS();
	MCTS(const MCTS&) = delete;
	MCTS &operator=(const MCTS&) = delete;
	MCTS(MCTS&&);
	MCTS &operator=(MCTS&&);

	bool operator==(const MCTS&) const;
	bool operator!=(const MCTS&) const;

public:
	void Initialize(unsigned int rand_seed, const GameEngine::Board &starting_board);
	void Iterate();

public:
	GameEngine::Board root_node_board;
	Tree tree;
	BoardNodeMap board_node_map;

private:
	void Select(TreeNode* & node, GameEngine::Board & board);
	bool Expand(TreeNode* & node, GameEngine::Board & board);
	bool Simulate(GameEngine::Board &board);
	void BackPropagate(bool is_win);

	void GetNextState(TreeNode *node, GameEngine::Move &move, GameEngine::Board &board, bool & introduced_random);
	int GetRandom();

private: // for internal use
	TreeNode *allocated_node;

	std::vector<TreeNode*> traversed_nodes;
};

#endif
