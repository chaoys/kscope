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

#ifndef __CORE_FILEFILTER_H__
#define __CORE_FILEFILTER_H__

#include <QList>

namespace KScope
{

namespace Core
{

/**
 * An advanced include/exclude file filter.
 * The filter is represented as a list of rules, each can classified as an
 * inclusion or exclusion rule. When matching a path name, the filter compares
 * the path with each of the entries in order, returning a decision based on
 * the first matching rule.
 * A filter can be created from a string, formatted as following:
 * - Rules are separated by semicolons
 * - Each rule is given as a simplified (shell-style) regular expression
 * - By default a rule is classified as an inclusion one
 * - To create an exclusion rule, prefix the expression with a minus sign (-)
 * @author Elad Lahav
 */
class FileFilter
{
public:
	/**
	 * Default constructor.
	 */
	FileFilter() {}

	/**
	 * Class constructor.
	 * Creates a filter out of the given semicolon-delimited string.
	 * @param  filter    The filter string
	 * @return
	 */
	FileFilter(const QString& filter) {
		parse(filter);
	}

	/**
	 * Determines whether the given path is matched by the filter.
	 * Iterates over the list of rules. The first rule whose pattern matches
	 * the path name is used to determine whether the path is accepted
	 * (including rule) or rejected (excluding rule).
	 * @param  path           The path name to check
	 * @param  noMatchResult  The value to return in case no rule matches the
	 *                        path
	 * @return true if the path is accepted by the filter, false otherwise
	 */
	bool match(const QString& path, bool noMatchResult) const {
		QList<Rule>::ConstIterator itr;

		for (itr = ruleList_.begin(); itr != ruleList_.end(); ++itr) {
			if ((*itr).exp_.exactMatch(path))
				return (((*itr).type_ == Rule::Include) ? true : false);
		}

		return noMatchResult;
	}

	/**
	 * Creates a semicolon delimited representation of the filter.
	 * @return The filter as a string
	 */
	QString toString() const {
		QString result;
		QList<Rule>::ConstIterator itr;

		for (itr = ruleList_.begin(); itr != ruleList_.end(); ++itr) {
			if ((*itr).type_ == Rule::Exclude)
				result += "-";
			result += (*itr).exp_.pattern() + ";";
		}

		return result;
	}

private:
	/**
	 * Represents a single rule in the filter.
	 */
	struct Rule {
		enum Type { Include, Exclude };

		/**
		 * Struct constructor.
		 * Creates a simplified regular expression out of the given pattern,
		 * and determines whether it is an inclusion or exclusion pattern based
		 * on the existence of a "-" prefix.
		 * @param  pattern  Simplified regular expression pattern
		 */
		Rule(const QString& pattern) {
			exp_.setPatternSyntax(QRegExp::Wildcard);
			if (pattern.startsWith("-")) {
				exp_.setPattern(pattern.mid(1));
				type_ = Exclude;
			}
			else {
				exp_.setPattern(pattern);
				type_ = Include;
			}
		}

		/**
		 * The simplified regular expression to match against.
		 */
		QRegExp exp_;

		/**
		 * Whether this rule is for including or excluding files.
		 */
		Type type_;
	};

	/**
	 * The filter, represented as an ordered list of rules.
	 */
	QList<Rule> ruleList_;

	/**
	 * Converts a semicolon-delimited string into a list of rules.
	 * @param  filter  The filter string to parse
	 */
	void parse(const QString& filter) {
		// Split the filter string to get a list of patterns.
		QStringList patterns = filter.split(';', QString::SkipEmptyParts);

		// Create a rule for each pattern.
		QStringList::Iterator itr;
		for (itr = patterns.begin(); itr != patterns.end(); ++itr)
			ruleList_.append(Rule(*itr));
	}
};

}

}

#endif // __CORE_FILEFILTER_H__
