#pragma once
#include "FlowControl/FlowController.h"

#include "FlowControl/Manipulate-impl.h"

namespace FlowControl
{
	inline Result FlowController::PlayCard(int hand_idx)
	{
		flow_context_.ResetActionParameter();
		PlayCardInternal(hand_idx);

		assert(flow_context_.Empty());
		return flow_context_.GetResult();
	}

	inline Result FlowController::EndTurn()
	{
		flow_context_.ResetActionParameter();
		EndTurnInternal();

		assert(flow_context_.Empty());
		return flow_context_.GetResult();
	}

	inline Result FlowController::Attack(state::CardRef attacker, state::CardRef defender)
	{
		flow_context_.ResetActionParameter();
		AttackInternal(attacker, defender);

		assert(flow_context_.Empty());
		return flow_context_.GetResult();
	}

	inline Result FlowController::Resolve()
	{
		Helpers::Resolver(state_, flow_context_).Resolve();
		return flow_context_.GetResult();
	}

	inline void FlowController::PlayCardInternal(int hand_idx)
	{
		if (!PlayCardPhase(hand_idx)) return;
		if (!Helpers::Resolver(state_, flow_context_).Resolve()) return;
	}

	inline bool FlowController::PlayCardPhase(int hand_idx)
	{
		state::CardRef card_ref = state_.GetCurrentPlayer().hand_.Get(hand_idx);
		state::Cards::Card const& card = state_.GetCardsManager().Get(card_ref);

		if (!card.GetRawData().battlecry_handler.PrepareBattlecryTarget(state_, flow_context_, card_ref, card)) {
			return SetResult(kResultInvalid);
		}

		state_.GetCurrentPlayer().GetResource().Cost(card.GetCost());

		switch (card.GetCardType())
		{
		case state::kCardTypeMinion:
			return PlayMinionCardPhase(hand_idx, card_ref, card);
		case state::kCardTypeWeapon:
			return PlayWeaponCardPhase(hand_idx, card_ref, card);
		default:
			return SetResult(kResultInvalid);
		}
	}

	inline bool FlowController::PlayMinionCardPhase(int hand_idx, state::CardRef card_ref, state::Cards::Card const& card)
	{
		state_.TriggerEvent<state::Events::EventTypes::BeforeMinionSummoned>(
			state::Events::EventTypes::BeforeMinionSummoned::Context{ state_, card_ref, card });

		if (state_.GetCurrentPlayer().minions_.Full()) {
			return SetResult(kResultInvalid);
		}

		int total_minions = (int)state_.GetCurrentPlayer().minions_.Size();
		int put_position = flow_context_.GetMinionPutLocation(0, total_minions);

		state_.GetZoneChanger<state::kCardZoneHand>(flow_context_.GetRandom(), card_ref)
			.ChangeTo<state::kCardZonePlay>(state_.GetCurrentPlayerId(), put_position);

		state_.TriggerEvent<state::Events::EventTypes::OnMinionPlay>(card);

		assert(card.GetPlayerIdentifier() == state_.GetCurrentPlayerId());
		card.GetRawData().battlecry_handler.DoBattlecry(state_, flow_context_, card_ref, card);

		state_.TriggerEvent<state::Events::EventTypes::AfterMinionPlayed>(card);

		state_.TriggerEvent<state::Events::EventTypes::AfterMinionSummoned>();
		Manipulate(state_, flow_context_).Minion(card_ref).AfterSummoned();

		return true;
	}

	inline bool FlowController::PlayWeaponCardPhase(int hand_idx, state::CardRef card_ref, state::Cards::Card const& card)
	{
		Manipulate(state_, flow_context_).CurrentHero().EquipWeapon<state::kCardZoneHand>(card_ref);

		assert(card.GetPlayerIdentifier() == state_.GetCurrentPlayerId());
		card.GetRawData().battlecry_handler.DoBattlecry(state_, flow_context_, card_ref, card);

		return true;
	}

	inline void FlowController::AttackInternal(state::CardRef attacker, state::CardRef defender)
	{
		if (!AttackPhase(attacker, defender)) return;
		if (!Helpers::Resolver(state_, flow_context_).Resolve()) return;
	}

	inline bool FlowController::AttackPhase(state::CardRef attacker, state::CardRef defender)
	{
		if (!attacker.IsValid()) return SetResult(kResultInvalid);
		if (!defender.IsValid()) return SetResult(kResultInvalid);

		// check if attacker is attackable
		if (!IsAttackable(attacker)) return SetResult(kResultInvalid);

		while (true) {
			state::CardRef origin_defender = defender;
			state_.TriggerEvent<state::Events::EventTypes::BeforeAttack>(attacker, state_, defender);
			state_.TriggerCategorizedEvent<state::Events::EventTypes::BeforeAttack>(attacker, state_, defender);

			if (!attacker.IsValid()) return kResultNotDetermined;
			if (state_.GetCardsManager().Get(attacker).GetHP() <= 0) return kResultNotDetermined;
			if (state_.GetCardsManager().Get(attacker).GetZone() != state::kCardZonePlay) return kResultNotDetermined;

			if (!defender.IsValid()) return kResultNotDetermined;
			if (state_.GetCardsManager().Get(defender).GetHP() <= 0) return kResultNotDetermined;
			if (state_.GetCardsManager().Get(defender).GetZone() != state::kCardZonePlay) return kResultNotDetermined;

			if (origin_defender == defender) break;
		}

		state_.TriggerEvent<state::Events::EventTypes::OnAttack>(
			attacker, state::Events::EventTypes::OnAttack::Context{ state_, flow_context_, defender });
		// TODO: attacker lose stealth

		Manipulate(state_, flow_context_).Character(defender).Damage(GetAttackValue(attacker));
		Manipulate(state_, flow_context_).Character(attacker).Damage(GetAttackValue(defender));

		Manipulate(state_, flow_context_).Character(attacker).AfterAttack();

		state_.TriggerEvent<state::Events::EventTypes::AfterAttack>(attacker, state_, defender);
		state_.TriggerCategorizedEvent<state::Events::EventTypes::AfterAttack>(attacker, state_, defender);

		{
			const state::Cards::Card & attacker_card = state_.GetCardsManager().Get(attacker);
			if (attacker_card.GetCardType() == state::kCardTypeHero) {
				state::CardRef weapon_ref = attacker_card.GetRawData().weapon_ref;
				if (weapon_ref.IsValid()) {
					Manipulate(state_, flow_context_).Weapon(weapon_ref).Damage(1);
				}
			}
		}

		return true;
	}

	inline bool FlowController::IsAttackable(state::CardRef attacker)
	{
		state::Cards::Card const& card = state_.GetCardsManager().Get(attacker);

		if (card.GetCardType() == state::kCardTypeMinion) {
			if (card.GetRawData().enchanted_states.charge == false && card.GetRawData().just_played) return false;
		}

		if (card.GetRawData().num_attacks_this_turn >= 1) return false; // TODO: windfury, etc. An attribute 'max_attacks'?

		return true;
	}

	inline int FlowController::GetAttackValue(state::CardRef ref)
	{
		int attack = 0;

		state::Cards::Card const& card = state_.GetCardsManager().Get(ref);

		int v1 = card.GetAttack();
		if (v1 > 0) attack += v1;

		state::CardRef weapon_ref = card.GetRawData().weapon_ref;
		if (weapon_ref.IsValid()) {
			assert(card.GetCardType() == state::kCardTypeHero);

			state::Cards::Card const& weapon = state_.GetCardsManager().Get(weapon_ref);
			int v2 = weapon.GetAttack();
			if (v2 > 0) attack += v2;
		}

		return attack;
	}

	inline void FlowController::EndTurnInternal()
	{
		if (state_.GetTurn() >= 89) {
			flow_context_.SetResult(kResultDraw);
			return;
		}
		state_.IncreaseTurn();

		flow_context_.ResetActionParameter();
		EndTurnPhase();
		if (!Helpers::Resolver(state_, flow_context_).Resolve()) return;

		state_.GetMutableCurrentPlayerId().ChangeSide();

		flow_context_.ResetActionParameter();
		StartTurnPhase();
		if (!Helpers::Resolver(state_, flow_context_).Resolve()) return;

		flow_context_.ResetActionParameter();
		DrawCardPhase();
		if (!Helpers::Resolver(state_, flow_context_).Resolve()) return;
	}

	inline void FlowController::EndTurnPhase()
	{
		state_.TriggerEvent<state::Events::EventTypes::OnTurnEnd>(
			state::Events::EventTypes::OnTurnEnd::Context{ state_, flow_context_ });
	}

	inline void FlowController::StartTurnPhase()
	{
		state_.GetCurrentPlayer().GetResource().IncreaseTotal();
		state_.GetCurrentPlayer().GetResource().Refill();
		// TODO: overload

		state_.TriggerEvent<state::Events::EventTypes::OnTurnStart>();

		for (state::CardRef minion : state_.GetCurrentPlayer().minions_.Get()) {
			Manipulate(state_, flow_context_).Minion(minion).TurnStart();
		}
		Manipulate(state_, flow_context_).CurrentHero().TurnStart();
	}

	inline void FlowController::DrawCardPhase()
	{
		Manipulate(state_, flow_context_).CurrentHero().DrawCard();
	}

	inline int FlowController::CalculateSpellDamage(state::board::Player const& player)
	{
		int spell_damage = 0;

		state::Cards::Card const& hero = state_.GetCardsManager().Get(player.GetHeroRef());
		spell_damage += hero.GetSpellDamage();
		spell_damage += state_.GetCardsManager().Get(hero.GetRawData().weapon_ref).GetSpellDamage();

		for (state::CardRef const& minion_ref : player.minions_.Get()) {
			spell_damage += state_.GetCardsManager().Get(minion_ref).GetSpellDamage();
		}

		return spell_damage;
	}

	inline bool FlowController::SetResult(Result result)
	{
		assert(result != kResultNotDetermined);
		flow_context_.SetResult(result);
		return false;
	}

}