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

#ifndef __EDITOR_CONFIG_H__
#define __EDITOR_CONFIG_H__

#include <QObject>
#include <QSettings>
#include <qscilexercpp.h>
#include <qscilexermakefile.h>
#include <qscilexerbash.h>
#include "viscintilla.h"

namespace KScope
{

namespace Editor
{

class Editor;
class LexerStyleModel;
class CommonLexer;

/**
 * Manages editor configuration.
 * @author Elad Lahav
 */
class Config : public QObject
{
	Q_OBJECT

public:
	Config(QObject* parent = NULL);
	~Config();

	void load(QSettings&);
	void store(QSettings&) const;
	void apply(Editor*) const;
	QsciLexer* lexer(const QString&) const;

	typedef QList<QsciLexer*> LexerList;

private:
	/**
	 * Whether to highlight the current line.
	 */
	bool hlCurLine_;

	/**
	 * Whether whitespace markers should be shown.
	 */
	bool visibleWSpace_;

	/**
	 * Whether to show line numbers in the margin.
	 */
	bool marginLineNumbers_;

	/**
	 * The column in which to place the end-of-line marker (0 to disable).
	 */
	int eolMarkerColumn_;

	/**
	 * Whether to use tabs for indentation.
	 */
	bool indentTabs_;

	/**
	 * The tab width, in characters.
	 */
	int tabWidth_;

	/**
	 * The Vi mode in which editors should be started.
	 */
	ViScintilla::EditMode viDefaultMode_;

	/**
	 * The common defaults lexers.
	 */
	CommonLexer* commonLexer_;

	/**
	 * C/C++ lexer.
	 */
	QsciLexerCPP* cppLexer_;

	/**
	 * Makefile lexer.
	 */
	QsciLexerMakefile* makeLexer_;

	/**
	 * Shell script lexer.
	 */
	QsciLexerBash* bashLexer_;

	/**
	 * A list of the above lexers for batch operations.
	 */
	LexerList lexers_;

	/**
	 * Used to configure lexer styles.
	 */
	LexerStyleModel* styleModel_;

	/**
	 * Maps file name patterns to lexers.
	 */
	struct {
		struct Pair {
			QRegExp re_;
			QsciLexer* lexer_;

			Pair(const QString& pattern)
				: re_(pattern, Qt::CaseSensitive, QRegExp::Wildcard),
				  lexer_(NULL) {}

			void operator=(QsciLexer* lexer) { lexer_ = lexer; }
		};

		QList<Pair> map_;

		Pair& operator[](const QString& pattern) {
			map_.append(Pair(pattern));
			return map_.last();
		}

		QsciLexer* find(const QString& text) const {
			QList<Pair>::ConstIterator itr;
			for (itr = map_.begin(); itr != map_.end(); ++itr) {
				if ((*itr).re_.exactMatch(text))
					return (*itr).lexer_;
			}
			return NULL;
		}

		void clear() { map_.clear(); }
	} lexerMap_;

	friend class ConfigDialog;

	void fromEditor(Editor* editor = NULL);

	template<class T>
	static inline void loadValue(const QSettings& settings, T& val,
	                      const QString& key, const T& defVal) {
		QVariant var = settings.value(key, defVal);
		val = var.value<T>();
	}
};

} // namespace Editor

} // namespace KScope

#endif // __EDITOR_CONFIG_H__
