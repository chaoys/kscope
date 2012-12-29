/***************************************************************************
 *   Copyright (C) 2007-2009 by Elad Lahav
 *   elad_lahav@users.sourceforge.net
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 ***************************************************************************/

#ifndef __PARSER_STATEMACHINE_H__
#define __PARSER_STATEMACHINE_H__

#include <QDebug>
#include "parser.h"

namespace KScope
{

namespace Parser
{

/**
 * My attempt at a generic, fancy-looking, state-machine.
 * The main goal is to be able to describe a state machine implementation as
 * simply and elegantly as possible.
 * The machine is a set of states and a transition function. Given an input
 * string, the current state is checked for all outgoing edges, which hold
 * statically built parser objects. If the input string is matched by the
 * parser, that edge's in-vertex is set as the current state.
 * @author Elad Lahav
 */
class StateMachine
{
public:
	struct TransitionBase;

	/**
	 * A single state in the machine.
	 * The entire logic of the state machine is implemented in the list of
	 * Transition objects held by each state.
	 */
	struct State
	{
		State(QString name = "") : name_(name) {}
		State(const State& other) : name_(other.name_),
			transList_(other.transList_) {}

		bool isError() const { return transList_.isEmpty(); }

		QString name_;
		QList<TransitionBase*> transList_;
	};

	/**
	 * Default action type for matching transitions.
	 * Does nothing.
	 */
	struct NoAction
	{
		void operator()(const CapList& caps) const {
			(void)caps;
		}
	};

	/**
	 * Abstract base class for transitions.
	 * Since transition objects are created at compile time, we need this
	 * base class in order to be able to specify a list of pointers to
	 * transitions in the State class.
	 */
	struct TransitionBase
	{
		TransitionBase(const State& nextState) : nextState_(nextState) {}

		virtual int matches(const QString& input, int pos) const = 0;

		const State& nextState_;
	};

	/**
	 * A transition rule in a state machine.
	 * Transitions are associated with states, and each has the form of
	 * <parser,action,next_state>. If an input is matched by the parser, then
	 * the action is taken and the state machine should advance to the next
	 * state.
	 */
	template<class ParserT, class ActionT = NoAction>
	struct Transition : public TransitionBase
	{
		Transition(const State& nextState, const ParserT& parser)
			: TransitionBase(nextState), parser_(parser) {}
		Transition(const State& nextState, const ParserT& parser,
		           ActionT action)
			: TransitionBase(nextState), parser_(parser), action_(action) {}

		/**
		 * Determines if a transition should be taken.
		 * @param  input  The input to match against
		 * @return The number of characters matched by the parser if the input
		 *         matches, -1 if a partial match was found, -2 on a parse
		 *         error
		 */
		int matches(const QString& input, int pos) const {
			SizedCapList<ParserT::capCount_> caps;
			switch (parser_.match(input, pos, caps)) {
			case NoMatch:
				return -2;

			case PartialMatch:
				return -1;

			case FullMatch:
				action_(caps);
				return pos;
			}

			return 0;
		}

		/**
		 * The parser used to match input.
		 */
		ParserT parser_;

		/**
		 * The action to take if input matches.
		 */
		ActionT action_;
	};

	/**
	 * Class constructor.
	 */
	StateMachine() : curState_(&initState_) {}

	/**
	 * Class destructor.
	 */
	~StateMachine() {
		while (!transList_.isEmpty())
			delete transList_.takeFirst();
	}

	/**
	 * Parses the given input using the state machine.
	 * When the method returns, the input string is adjusted to contain only
	 * the part of the input that was not parsed (in case of a partial parse
	 * match).
	 * @param  input  The input to parse
	 * @return true if parsing was successful, false otherwise
	 */
	bool parse(QString& input) {
		// Return immediately if in an error state.
		if (curState_->isError()) {
			qDebug() << "Error state!";
			return false;
		}

		int pos = 0;
		while (pos < input.length()) {
			ParseResult result = NoMatch;

			// Iterate over the list of transitions.
			QList<TransitionBase*>::ConstIterator itr;
			for (itr = curState_->transList_.begin();
				 itr != curState_->transList_.end();
				 ++itr) {
				// Match the input using the transition's parser.
				int newPos = (*itr)->matches(input, pos);
				if (newPos >= 0) {
					// Match, consume input and move to the next state.
					pos = newPos;
					curState_ = &(*itr)->nextState_;
					result = FullMatch;
#ifdef DEBUG_PARSER
					qDebug() << "Parse match, next state is"
					         << curState_->name_;
#endif
					break;
				}
				else if (newPos == -1) {
					// Partial match, try other rules for a full match.
					result = PartialMatch;
				}
			}

			// Abort if no rule matched.
			if (result == NoMatch) {
				qDebug() << "Parse error!" << curState_->name_
				         << input.mid(pos);
				curState_ = &errorState_;
				return false;
			}

			// Stop if only a partial match was found.
			if (result == PartialMatch)
				break;
		}

		// Wait for more input.
		input = input.mid(pos);
		return true;
	}

	/**
	 * Sets the current state of the machine.
	 * @param  state  The new state
	 */
	void setState(const State& state) { curState_ = &state; }

	/**
	 * Sets the default state as the current one.
	 */
	void reset() { curState_ = &initState_; }

	template<class ParserT, class ActionT>
	void addRule(State& from, const ParserT& parser, const State& to,
	             const ActionT& action) {
		typedef Transition<ParserT, ActionT> TransT;
		TransT* trans = new TransT(to, parser, action);
		from.transList_.append(trans);
		transList_.append(trans);
	}

	template<class ParserT>
	void addRule(State& from, const ParserT& parser, const State& to) {
		typedef Transition<ParserT> TransT;
		TransT* trans = new TransT(to, parser);
		from.transList_.append(trans);
		transList_.append(trans);
	}

protected:
	State initState_;

private:
	const State* curState_;
	State errorState_;
	QList<TransitionBase*> transList_;
};

} // namespace Parser

} // namespace KScope

#endif // __PARSER_STATEMACHINE_H__
