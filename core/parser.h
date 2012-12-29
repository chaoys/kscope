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

#ifndef __PARSER_PARSER_H__
#define __PARSER_PARSER_H__

#include <QString>
#include <QVariant>

namespace KScope
{

namespace Parser
{

/**
 * Possible results when matching input to a parser object.
 */
enum ParseResult
{
	/** Input cannot be matched by the parser. */
	NoMatch,
	/** Potential match, but more input is required. */
	PartialMatch,
	/** Input was matched by the parser. */
	FullMatch
};

/**
 * A list of values captured during parsing.
 * The use of a QVector container allows us to pre-allocate the list when the
 * number of captured values is known (which is true in most cases, the
 * exception being parsers that include a Kleene-star expression).
 */
struct CapList
{
	/**
	 * The actual storage.
	 */
	QVector<QVariant> caps_;

	/**
	 * The number of used positions in the vector.
	 */
	int used_;

	/**
	 * Default constructor.
	 * Used when the number of captured values is not known in advance.
	 */
	CapList() : caps_(), used_(0) {}

	/**
	 * Constructor.
	 * Used when the number of captured values is known in advance.
	 * @param  size  The number of values that can be captured by the parser
	 */
	CapList(int size) : caps_(size), used_(0) {}

	/**
	 * Appends a value to the vector.
	 * If the vector was pre-allocated, the value is set at the next available
	 * position (rather than growing the vector, which is the default).
	 * @param  var  The value to append
	 * @return A reference to this object
	 */
	CapList& operator<<(const QVariant& var) {
		if (used_ < caps_.size())
			caps_[used_++] = var;
		else
			caps_ << var;

		return *this;
	}

	/**
	 * Provides random access to the values in the vector.
	 * @param  pos  The position to get
	 * @return The value at the given position
	 */
	const QVariant& operator[](int pos) const {
		return caps_[pos];
	}

	/**
	 * @return The size of the vector
	 */
	int size() const { return caps_.size(); }
};

/**
 * Specialisation for known sizes.
 */
template<int S>
struct SizedCapList : public CapList
{
	 SizedCapList() : CapList(S) {}
};

/**
 * Specialisation for unknown sizes (represented by -1).
 */
template<>
struct SizedCapList<-1> : public CapList
{
	 SizedCapList() : CapList() {}
};

/**
 * Combines two numbers, each representing the number of captured values in a
 * parser class.
 * If both values are non-negative, the result is the sum of the two numbers.
 * Otherwise, the result is -1, representing an "unknown" value, which is the
 * case in a parser with a Kleene-star operator.
 */
template<int A, int B>
struct AddCapCount
{
	static const int result_ = A + B;
};

/**
 * Specialisation for the case where the first number is negative.
 */
template<int B>
struct AddCapCount<-1, B>
{
	static const int result_ = -1;
};

/**
 * Specialisation for the case where the first number is negative.
 */
template<int A>
struct AddCapCount<A, -1>
{
	static const int result_ = -1;
};

template<class Exp1T, class Exp2T>
struct Concat;

template<class ExpT>
struct Kleene;

/**
 * Syntactic-sugar operators for building parsers out of the basic blocks.
 * Each parser class T should inherit from Operators<T>.
 */
template<class ExpT>
struct Operators
{
	template<class Exp2T>
	Concat<ExpT, Exp2T> operator<<(const Exp2T&) const;

	Kleene<ExpT> operator*() const;
};

/**
 * Matches a fixed-string.
 */
struct Literal : public Operators<Literal>
{
	/**
	 * Class constructor.
	 * @param  str  The string to match.
	 */
	Literal(const QString& str) : str_(str) {}

	/**
	 * Matches the object's string with a prefix of the input.
	 * @param   input  The input string
	 * @param   pos    The current position in the input string
	 * @param   caps   An ordered list of captured values
	 * @return  true if the input has a mathcing prefix, false otherwise
	 */
	ParseResult match(const QString& input, int& pos, CapList& caps) const {
		(void)caps;

#ifdef DEBUG_PARSER
		qDebug() << "Literal::match" << input.mid(pos) << str_;
#endif

		// If the input is shorter than the expected string, that it can be at
		// most a partial match.
		if (input.length() < str_.length())
			return str_.startsWith(input) ? PartialMatch : NoMatch;

		// Input is longer than expected string, so it is either a full match or
		// no match.
		if (input.mid(pos, str_.length()) == str_) {
#ifdef DEBUG_PARSER
			qDebug() << str_;
#endif
			pos += str_.length();
			return FullMatch;
		}

		return NoMatch;
	}

	static const int capCount_ = 0;

private:
	/** The string to match. */
	const QString str_;
};

/**
 * Captures a base-10 numeric value.
 */
struct Number : public Operators<Number>
{
	/**
	 * Matches a non-empty sequence of digits, up to the first non-digit
	 * character (or the end of the input).
	 * @param   input  The input string
	 * @param   pos    The current position in the input string
	 * @param   caps   An ordered list of captured values
	 * @return  true if matched a number, false otherwise
	 */
	ParseResult match(const QString& input, int& pos, CapList& caps) const {
		int digit, number = 0;
		bool foundNumber = false;

#ifdef DEBUG_PARSER
		qDebug() << "Number::match" << input.mid(pos);
#endif

		 // Iterate to the end of the input.
		while (pos < input.size()) {
			// Stop if a non-digit character is found.
		    if ((digit = input[pos].digitValue()) == -1) {
		    	// Check if any input was consumed.
		    	if (!foundNumber)
		    		return NoMatch;

		    	// Found a number.
#ifdef DEBUG_PARSER
				qDebug() << number;
#endif
		    	caps << number;
		    	return FullMatch;
		    }

			// At least one digit.
			// Update the captured numeric value, the position and indicate that
			// a number has been found.
			number = (number * 10) + digit;
			pos++;
			foundNumber = true;
		}

		// Ran out of input characters.
		return PartialMatch;
	}

	static const int capCount_ = 1;
};

/**
 * Captures a string delimited by a single character.
 * The default delimiter causes the string to match to the end of the input.
 */
template<class DelimT = QChar, bool AllowEmpty = false>
struct String : public Operators< String<DelimT, AllowEmpty> >
{
	String(DelimT delim) : delim_(delim) {}

	/**
	 * Matches a string up to the object's delimiter.
	 * @param   input  The input string
	 * @param   pos    The current position in the input string
	 * @param   caps   An ordered list of captured values
	 * @return  true if matched a non-empty string, false otherwise
	 */
	ParseResult match(const QString& input, int& pos, CapList& caps) const {
#ifdef DEBUG_PARSER
		qDebug() << "String::match" << input.mid(pos);
#endif

		if (input.isEmpty())
			return PartialMatch;

		// Find an occurrence of the delimiter.
		int delimPos = input.indexOf(delim_, pos);
		if (delimPos == -1)
			return PartialMatch;

		// Check for empty strings.
		if (!AllowEmpty && delimPos == pos)
			return NoMatch;

		// Update position and captured values list.
#ifdef DEBUG_PARSER
		qDebug() << input.mid(pos, delimPos - pos);
#endif
		caps << input.mid(pos, delimPos - pos);
		pos = delimPos;
		return FullMatch;
	}

	static const int capCount_ = 1;

private:
	DelimT delim_;
};

/**
 * Swallows whitespace.
 */
struct Whitespace : public Operators<Whitespace>
{
	/**
	 * Matches a (possibly empty) sequence of any space characters.
	 * @param   input  The input string
	 * @param   pos    The current position in the input string
	 * @param   caps   An ordered list of captured values
	 * @return  Always true
	 */
	ParseResult match(const QString& input, int& pos, CapList& caps) const {
		(void)caps;

#ifdef DEBUG_PARSER
		qDebug() << "Whitespace::match" << input;
#endif

		while ((pos < input.size()) && (input[pos].isSpace()))
			pos++;

		return FullMatch;
	}

	static const int capCount_ = 0;
};

/**
 * Concatenates two parsers.
 * Matches input that is matched first by one parser, and then by the other.
 */
template<class Exp1T, class Exp2T>
struct Concat : public Operators< Concat<Exp1T, Exp2T> >
{
	Concat(Exp1T exp1, Exp2T exp2) : exp1_(exp1), exp2_(exp2) {}

	ParseResult match(const QString& input, int& pos, CapList& caps) const {
		ParseResult result = exp1_.match(input, pos, caps);
		if (result == FullMatch)
			return exp2_.match(input, pos, caps);

		return result;
	}

	static const int capCount_
		= AddCapCount<Exp1T::capCount_, Exp2T::capCount_>::result_;

private:
	Exp1T exp1_;
	Exp2T exp2_;
};

/**
 * A Kleene-star closure.
 * Matches input matched by zero or more instances of a parser.
 */
template<class ExpT>
struct Kleene : public Operators< Kleene<ExpT> >
{
	Kleene(ExpT exp) : exp_(exp) {}

	ParseResult match(const QString& input, int& pos, CapList& caps) const {
		ParseResult result;
		while ((result = exp_.match(input, pos, caps)) == FullMatch)
			;

		if (result == PartialMatch)
			return PartialMatch;

		return FullMatch;
	}

private:
	ExpT exp_;
};

/**
 * Implements the concatenation operator (<<) for building parsers.
 */
template<class ExpT>
template<class Exp2T>
Concat<ExpT, Exp2T> Operators<ExpT>::operator<<(const Exp2T& exp2) const {
	return Concat<ExpT, Exp2T>(*static_cast<ExpT const*>(this), exp2);
}

/**
 * Implements the Kleene-star operator (*) for building parsers.
 */
template<class ExpT>
Kleene<ExpT> Operators<ExpT>::operator*() const {
	return Kleene<ExpT>(*static_cast<ExpT const*>(this));
}

} // namespace Parser

} // namespace KScope

#endif // __PARSER_PARSER_H__
