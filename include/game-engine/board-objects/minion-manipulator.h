#pragma once

#include <memory>
#include "object-base.h"
#include "game-engine/board-objects/minion-data.h"

namespace GameEngine {

class Board;

namespace BoardObjects {

	class Minions;

	// Manipulator is designed to modify data members for MinionData structure,
	// and will trigger related hooks on-the-fly
	class Minion : public ObjectBase
	{
		friend class Minions; // only class 'Minions' can create/copy/move

	private: // copy semantics should only be used in Minions
		Minion(Minion const& rhs) = delete;

		Minion(Board & board, Minions & minions, MinionData const& minion)
			: board(board), minions(minions), minion(minion)
		{}

		Minion(Board & board, Minions & minions, MinionData && minion)
			: board(board), minions(minions), minion(std::move(minion))
		{}

		Minion(Board & board, Minions & minions, Minion const& minion)
			: board(board), minions(minions), minion(minion.minion)
		{}

		Minion(Board & board, Minions & minions, Minion && minion)
			: board(board), minions(minions), minion(std::move(minion.minion))
		{}

	public:
		// move semantics should only be used by container (i.e., std::list<Minion>)
		Minion(Minion && rhs)
			: board(rhs.board), minions(rhs.minions), minion(std::move(rhs.minion))
		{}

		Minion & operator=(Minion const& rhs) = delete;
		Minion & operator=(Minion && rhs) = delete;

	public:
		bool operator==(Minion const& rhs) const { return this->GetMinion() == rhs.GetMinion(); }
		bool operator!=(Minion const& rhs) const { return this->GetMinion() != rhs.GetMinion(); }

		Board & GetBoard() const { return this->board; }
		Minions & GetMinions() const { return this->minions; }
		MinionData const& GetMinion() const { return this->minion; }

	public: // basic operations on minion (including interface to ObjectBase)
		int GetHP() const;
		int GetMaxHP() const;
		int GetAttack() const;

		bool Attackable() const;
		void AttackedOnce();

		void TakeDamage(int damage, bool poisonous);
		bool IsForgetful() const;

		void SetFreezeAttacker(bool freeze);
		void SetFreezed(bool freezed);
		bool IsFreezeAttacker() const;
		bool IsFreezed() const;

		bool IsPoisonous() const;

	public:
		void AddAttack(int val);
		void IncreaseCurrentAndMaxHP(int val);
		void DecreaseMaxHP(int val);

		void AddOnDeathTrigger(MinionData::OnDeathTrigger func);
		std::list<MinionData::OnDeathTrigger> GetAndClearOnDeathTriggers();

		void SetMinionStatFlag(MinionStat::Flag flag, bool val);

	public: // auras
		void AddAura(std::unique_ptr<Aura> && aura);
		void ClearAuras();

	public: // enchantments
		void AddEnchantment(std::unique_ptr<Enchantment<Minion>> && enchantment, EnchantmentOwner * owner);
		void RemoveEnchantment(Enchantment<Minion> * enchantment);
		void ClearEnchantments();

	public: // hooks
		void TurnStart(bool owner_turn);
		void TurnEnd(bool owner_turn);

	public: // triggering hooks
		void HookAfterMinionAdded(Minion & added_minion);
		void HookMinionCheckEnraged();

	public:
		bool IsPlayerSide() const;
		bool IsOpponentSide() const;

	private:
		Board & board;
		Minions & minions;
		MinionData minion;
	};

} // BoardObjects
} // GameEngine