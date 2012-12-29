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

#include <QFileInfo>
#include "config.h"
#include "editor.h"
#include "lexerstylemodel.h"

namespace KScope
{

namespace Editor
{

/**
 * A special lexer used to create a common default style for all other lexers.
 * This class serves two purposes:
 * 1. Allows style properties to be inherited by all lexers (e.g., for setting
 *    a single, common font);
 * 2. Provides a default lexer for files that are not handled by any of the
 *    pre-defined lexers.
 * @author Elad Lahav
 */
class CommonLexer : public QsciLexer
{
public:
	/**
	 * Class constructor.
	 * @param  parent Parent object
	 */
	CommonLexer(QObject* parent) : QsciLexer(parent) {}

	/**
	 * Class destructor.
	 */
	~CommonLexer() {}

	/**
	 * @return A string identifying the language handled by the lexer
	 */
	const char* language() const { return tr("Common").toLatin1(); }

	/**
	 * @return A string identifying the lexer
	 */
	const char* lexer() const { return "common"; }

	/**
	 * Provides a name for the given style ID.
	 * @param  style The style ID
	 * @return The name of the style, or an empty string if the style does not
	 *         exist
	 */
	QString description(int style) const {
		if (style == 0)
			return tr("Default");

		return QString();
	}

	/**
	 * @return The ID of the default style for this lexer
	 */
	int defaultStyle() const { return 0; }
};

/**
 * The sole purpose of this class is to provide a fix a couple of bugs in
 * QsciLexerCPP:
 * 1. The description of the UUID style is not handled, causing the style
 *    detection mechanism to stop before all styles are found.;
 * 2. The default text colour is set to grey, rather than black
 * @author Elad Lahav
 */
class CPPLexer : public QsciLexerCPP
{
public:
	/**
	 * Class constructor.
	 * @param  parent Parent object
	 */
	CPPLexer(QObject* parent) : QsciLexerCPP(parent) {}

	/**
	 * Class destructor.
	 */
	~CPPLexer() {}

	/**
	 * Provides a description string for each style.
	 * Fixes the bug in QsciLexerCPP::description().
	 * @param  style The style ID
	 * @return The style description or an empty string if the style does not
	 *         exist
	 */
	QString description(int style) const {
		if (style == UUID)
			return "UUID";

		return QsciLexerCPP::description(style);
	}

	/**
	 * Returns the default text colour for the given style.
	 * @param  style The requested style
	 * @return The style's default colour
	 */
	QColor defaultColor(int style) const {
		if (style == Default)
			return QColor(0, 0, 0);

		return QsciLexerCPP::defaultColor(style);
	}
};

/**
 * Class constructor.
 * @param  parent Parent object
 */
Config::Config(QObject* parent) : QObject(parent)
{
	// Create the lexers.
	commonLexer_ = new CommonLexer(this);
	cppLexer_ = new CPPLexer(this);
	makeLexer_ = new QsciLexerMakefile(this);
	bashLexer_ = new QsciLexerBash(this);
	lexers_ << commonLexer_ << cppLexer_ << makeLexer_ << bashLexer_;

	// Create the lexer style model.
	styleModel_ = new LexerStyleModel(lexers_, this);
}

/**
 * Class destructor.
 */
Config::~Config()
{
}

/**
 * Reads editor configuration parameters from a QSettings object.
 * @param  settings The object to read from
 */
void Config::load(QSettings& settings)
{
	// Get the current (default) configuration.
	Editor editor;

	// Read values from the settings object.
	loadValue(settings, hlCurLine_, "HighlightCurrentLine", false);
	loadValue(settings, visibleWSpace_, "VisibleWhitespace", false);
	loadValue(settings, marginLineNumbers_, "LineNumbersInMargin", false);
	loadValue(settings, eolMarkerColumn_, "EOLMarkerColumn", 0);
	loadValue(settings, indentTabs_, "IndentWithTabs",
	          editor.indentationsUseTabs());
	loadValue(settings, tabWidth_, "TabWidth", editor.tabWidth());

	uint viMode;
	loadValue(settings, viMode, "ViMode", (uint)ViScintilla::Disabled);
	viDefaultMode_ = static_cast<ViScintilla::EditMode>(viMode);

	// Load the C lexer parameters.
	// Ignore the exception thrown by load(), which is the result of not
	// finding the style magic key in the settings file. This happens if styles
	// were never modified by the user, and is thus benign.
	styleModel_->load(settings, true);
	styleModel_->updateLexers();

	// Create the file to lexer map.
	// TODO: Make this configurable.
	lexerMap_.clear();
	lexerMap_["*.c"] = cppLexer_;
	lexerMap_["*.cc"] = cppLexer_;
	lexerMap_["*.cpp"] = cppLexer_;
	lexerMap_["*.h"] = cppLexer_;
	lexerMap_["*.hpp"] = cppLexer_;
	lexerMap_["Makefile*"] = makeLexer_;
	lexerMap_["*.sh"] = bashLexer_;
}

/**
 * Writes the current configuration parameters to a QSettings object.
 * @param  settings The object to write to
 */
void Config::store(QSettings& settings) const
{
	// Store editor configuration.
	settings.setValue("HighlightCurrentLine", hlCurLine_);
	settings.setValue("VisibleWhitespace", visibleWSpace_);
	settings.setValue("LineNumbersInMargin", marginLineNumbers_);
	settings.setValue("EOLMarkerColumn", eolMarkerColumn_);
	settings.setValue("IndentWithTabs", indentTabs_);
	settings.setValue("TabWidth", tabWidth_);
	settings.setValue("ViMode", viDefaultMode_);
	styleModel_->store(settings, true);
}

/**
 * Updates the settings of the current editor to reflect the current
 * configuration parameters.
 * @param  editor The editor to which to apply the configuration
 */
void Config::apply(Editor* editor) const
{
	editor->setCaretLineVisible(hlCurLine_);
	editor->setWhitespaceVisibility
		(visibleWSpace_ ? QsciScintilla::WsVisible
		                : QsciScintilla::WsInvisible);
	if (marginLineNumbers_) {
		editor->setMarginLineNumbers(2, true);
		QFont font = commonLexer_->font(0);
		int width = QFontMetrics(font).width("8888");
		editor->setMarginsFont(font);
		editor->setMarginWidth(2, width);
	}
	else {
		editor->setMarginLineNumbers(2, false);
		editor->setMarginWidth(2, 0);
	}
	if (eolMarkerColumn_ > 0) {
		editor->setEdgeMode(QsciScintilla::EdgeLine);
		editor->setEdgeColumn(eolMarkerColumn_);
	}
	else {
		editor->setEdgeMode(QsciScintilla::EdgeNone);
	}
	editor->setIndentationsUseTabs(indentTabs_);
	editor->setTabWidth(tabWidth_);
	editor->setFont(commonLexer_->font(0));
	editor->setColor(commonLexer_->color(0));
	editor->setPaper(commonLexer_->paper(0));
	editor->setEditMode(viDefaultMode_);

	if (editor->lexer())
		editor->lexer()->refreshProperties();
}

/**
 * Provides a lexer for the given file.
 * @param  path The path to the file
 * @return The appropriate lexer
 */
QsciLexer* Config::lexer(const QString& path) const
{
	QString name = QFileInfo(path).fileName();
	QsciLexer* lexer = lexerMap_.find(path);
	return lexer == NULL ? commonLexer_ : lexer;
}

} // namespace Editor

} // namespace KScope
