#pragma once

// http://www.hearthpwn.com/cards?filter-set=2&filter-premium=1&filter-class=1&sort=-cost&display=1

namespace Cards
{
	class Card_CS2_189 : public MinionCardBase<Card_CS2_189>, MinionCardUtils
	{
	public:
		static constexpr int id = Cards::ID_CS2_189;

		Card_CS2_189()
		{
			battlecry_target_getter = [] (auto context) {
				SetBattlecryTarget(context, Targets().Targetable().Enemy(context));
				return true;
			};
			battlecry = [](auto context) {
				Damage(context).Target(context.flow_context_.battlecry_target_).Amount(1);
			};
		}
	};

	class Card_CS1_042 : public MinionCardBase<Card_CS1_042>, MinionCardUtils
	{
	public:
		static constexpr int id = Cards::ID_CS1_042;

		Card_CS1_042()
		{
			Taunt();
		}
	};

	class Card_CS2_168 : public MinionCardBase<Card_CS2_168>
	{
	public:
		static constexpr int id = Cards::ID_CS2_168;

		Card_CS2_168() {}
	};

	class Card_EX1_508_Enchant : public EnchantmentCardBase
	{
	public:
		static constexpr EnchantmentTiers tier = kEnchantmentAura;

		Card_EX1_508_Enchant()
		{
			apply_functor = [](auto& stats) {
				++stats.attack;
			};
		}
	};

	class Card_EX1_508 : public MinionCardBase<Card_EX1_508>, MinionCardUtils
	{
	public:
		static constexpr int id = Cards::ID_EX1_508;

		Card_EX1_508()
		{
			Aura<Card_EX1_508_Enchant>().Target([](auto& context, auto& targetor) {
				targetor
					.Ally(context).Minion().Murlocs() // friendly murlocs only
					.Exclude(context.card_ref_); // only apply on other murlocs
			});
		}
	};

	class Card_CS2_171 : public MinionCardBase<Card_CS2_171>, MinionCardUtils
	{
	public:
		static constexpr int id = Cards::ID_CS2_171;

		Card_CS2_171()
		{
			Charge();
		}
	};

	class Card_EX1_011 : public MinionCardBase<Card_EX1_011>, MinionCardUtils
	{
	public:
		static constexpr int id = Cards::ID_EX1_011;

		Card_EX1_011()
		{
			battlecry_target_getter = [](auto context) {
				SetBattlecryTarget(context, Targets().Targetable().Exclude(context.card_ref_));
				return true;
			};
			battlecry = [](auto context) {
				Heal(context).Target(context.flow_context_.battlecry_target_).Amount(2);
			};
		}
	};

	class Card_EX1_066 : public MinionCardBase<Card_EX1_066>, MinionCardUtils
	{
	public:
		static constexpr int id = Cards::ID_EX1_066;

		Card_EX1_066()
		{
			battlecry = [](auto context) {
				Manipulate(context).OpponentHero().DestroyWeapon();
			};
		}
	};

	class Card_CS2_172 : public MinionCardBase<Card_CS2_172>, MinionCardUtils
	{
	public:
		static constexpr int id = Cards::ID_CS2_172;
	};

	class Card_CS2_173 : public MinionCardBase<Card_CS2_173>, MinionCardUtils
	{
	public:
		static constexpr int id = Cards::ID_CS2_173;

		Card_CS2_173()
		{
			Charge();
		}
	};

	class Card_CS2_121 : public MinionCardBase<Card_CS2_121>, MinionCardUtils
	{
	public:
		static constexpr int id = Cards::ID_CS2_121;

		Card_CS2_121()
		{
			Taunt();
		}
	};

	class Card_CS2_142 : public MinionCardBase<Card_CS2_142>, MinionCardUtils
	{
	public:
		static constexpr int id = Cards::ID_CS2_142;

		Card_CS2_142()
		{
			SpellDamage(1);
		}
	};
}

REGISTER_MINION_CARD_CLASS(Cards::Card_CS2_189)
REGISTER_MINION_CARD_CLASS(Cards::Card_CS1_042)
REGISTER_MINION_CARD_CLASS(Cards::Card_CS2_168)
REGISTER_MINION_CARD_CLASS(Cards::Card_EX1_508)
REGISTER_MINION_CARD_CLASS(Cards::Card_CS2_171)
REGISTER_MINION_CARD_CLASS(Cards::Card_EX1_011)
REGISTER_MINION_CARD_CLASS(Cards::Card_EX1_066)
REGISTER_MINION_CARD_CLASS(Cards::Card_CS2_172)
REGISTER_MINION_CARD_CLASS(Cards::Card_CS2_173)
REGISTER_MINION_CARD_CLASS(Cards::Card_CS2_121)
REGISTER_MINION_CARD_CLASS(Cards::Card_CS2_142)
