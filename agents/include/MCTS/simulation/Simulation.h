#pragma once

#include "MCTS/Config.h"
#include "MCTS/policy/Simulation.h"

namespace mcts
{
	namespace simulation
	{
		class Simulation
		{
		public:
			Simulation(state::PlayerSide side, std::mt19937 & rand) :
				random_(rand), select_(side, rand)
			{}

			void StartNewAction() {
			}

			FlowControl::Result CutoffCheck(board::Board const& board)
			{
				using Policy = std::decay_t<decltype(select_)>;

				if constexpr (Policy::kEnableCutoff) {
					return select_.GetCutoffResult(board);
				}
				else {
					return FlowControl::kResultNotDetermined;
				}
			}

			int ChooseAction(
				board::Board const& board,
				FlowControl::ValidActionAnalyzer const& action_analyzer,
				FlowControl::ActionType action_type,
				FlowControl::ActionChoices const& action_choices)
			{
				assert(!action_choices.Empty());

				int choices = action_choices.Size();

				if (choices == 1) {
					return 0;
				}

				if (action_type.IsChosenRandomly()) {
					int rnd = random_.GetRandom(choices);
					return action_choices.Get(rnd);
				}

				assert(action_type.IsChosenManually());

				int choice = select_.GetChoice(
					board, action_analyzer, action_type,
					policy::simulation::ChoiceGetter(choices)
				);
				assert(choice >= 0); // always return a valid choice

				return action_choices.Get(choice);
			}

		private:
			StaticConfigs::SimulationPhaseRandomActionPolicy random_;
			StaticConfigs::SimulationPhaseSelectActionPolicy select_;
		};
	}
}