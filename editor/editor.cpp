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

#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QDebug>
#include <qscilexercpp.h>
#include "editor.h"
#include "fileiothread.h"
#include "findtextdialog.h"

namespace KScope
{

namespace Editor
{

/**
 * Class constructor.
 * @param  parent  Owner object
 */
Editor::Editor(QWidget* parent) : ViScintilla(parent),
	newFileIndex_(0),
	isLoading_(false),
	onLoadLine_(0),
	onLoadColumn_(0),
	onLoadFocus_(false)
{
}

/**
 * Class destructor.
 */
Editor::~Editor()
{
}

/**
 * Asynchronously loads the contents of the given file into the editor.
 * Launches a thread that reads the file contents. When done, the thread signals
 * the editor with the read text.
 * During the loading process, the editor widget is disabled. Any calls to
 * setCursorPosition() or setFocus() are delayed until loading finishes.
 * @param  path  The path of the file to load
 * @param  lexer Text formatter
 * @return true if the loading process started successfully, false otherwise
 */
bool Editor::load(const QString& path, QsciLexer* lexer)
{
	// Indicate that loading is in progress.
	isLoading_ = true;
	setEnabled(false);
	setText(tr("Loading..."));

	setLexer(lexer);

	// Create and start the loading thread.
	FileIoThread* thread = new FileIoThread(this);
	connect(thread, SIGNAL(done(const QString&)), this,
	        SLOT(loadDone(const QString&)));
	connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
	if (!thread->load(path)) {
		setText(tr("Loading failed"));
		return false;
	}

	// Store the path.
	path_ = path;
	return true;
}

/**
 * Writes the contents of the editor back to the file.
 * @note The current implementation is synchronous, which may cause KScope to
 *       hang if the saving process takes too long (e.g., for NFS-mounted
 *       files). This should be fixed at some point.
 * @return true if successful, false otherwise
 */
bool Editor::save()
{
	// Nothing to do if the contents did not change since the last save.
	if (!isModified())
		return true;

	// Prompt for a file name if the editor is holding a new file.
	QString path = path_;
	if (path.isEmpty()) {
		path = QFileDialog::getSaveFileName(this, tr("Save New File"));
		if (path.isEmpty())
			return false;
	}

	// TODO: Provide an asynchronous implementation.
	QFile file(path);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QString msg = tr("Failed to save '%1'").arg(path_);
		QMessageBox::critical(this, tr("File Error"), msg);
		return false;
	}

	// Save the file's contents.
	QTextStream strm(&file);
	strm << text();

	// Notify of a change in the file path, if necessary.
	if (path != path_) {
		QString oldTitle = title();
		path_ = path;
		emit titleChanged(oldTitle, title());
	}

	setModified(false);
	return true;
}

/**
 * Determines whether the editor can be safely closed.
 * This is the case if the contents are not modified, the user saves the changes
 * or the user decides that the contents should not be saved.
 * @return true if the editor can be closed, false otherwise
 */
bool Editor::canClose()
{
	if (isModified()) {
		// Prompt the user for unsaved changes.
		QString msg = tr("The file '%1' was modified.\n"
		                 "Would you like to save it?")
		              .arg(path_);
		QMessageBox::StandardButtons buttons = QMessageBox::Yes
		                                       | QMessageBox::No
		                                       | QMessageBox::Cancel;
		switch (QMessageBox::question(0, tr("Unsaved Changes"), msg,
		                              buttons)) {
		case QMessageBox::Yes:
			// Save the contents of the editor.
			if (!save())
				return false;
			break;

		case QMessageBox::No:
			// Ignore changes.
			setModified(false);
			break;

		default:
			// Stop the close operation.
			return false;
		}
	}

	return true;
}

/**
 * Moves the cursor to the requested position in the document.
 * This function translates 1-based line and column indexes into the 0-based
 * values used by Scintilla. We use 0 values as "don't cares", that do not
 * change the current value of the respective dimension (line or column).
 * @param  line    1-based line number, 0 to keep the current line
 * @param  column  1-based column number, 0 to keep the current column
 */
void Editor::moveCursor(uint line, uint column)
{
	// Wait for file loading to complete before setting a new position.
	if (isLoading_) {
		if (line)
			onLoadLine_ = line;

		if (column)
			onLoadColumn_ = column;

		return;
	}

	// Get current values.
	int curLine, curColumn;
	getCursorPosition(&curLine, &curColumn);

	// Determine the new line position.
	if (line == 0)
		line = curLine;
	else
		line--;

	// Determine the new column position.
	if (column == 0)
		column = curColumn;
	else
		column--;

	// Set the new cursor position.
	ViScintilla::setCursorPosition(line, column);
}

/**
 * Returns a symbol for automatic selection.
 * If any text is selected in the editor, it is returned. Otherwise, the method
 * returns the word on which the cursor is currently positioned.
 * @return The current text
 */
QString Editor::currentSymbol() const
{
	// Return any selected text.
	// TODO: Should we test for a valid symbol here to?
	if (hasSelectedText())
		return QsciScintilla::selectedText();

	// No selected text.
	// Get the boundaries of the word from the current cursor position.
	long pos, start, end;
	pos = SendScintilla(SCI_GETCURRENTPOS);
	start = SendScintilla(SCI_WORDSTARTPOSITION, pos, 0L);
	end = SendScintilla(SCI_WORDENDPOSITION, pos, 0L);

	// Return an empty string if no word is found.
	if (start >= end)
		return QString();

	// Extract the word's text using its position boundaries.
	QByteArray curText;
	curText.resize(end - start );
	SendScintilla(SCI_GETTEXTRANGE, start, end, curText.data());

	// NOTE: Scintilla's definition of a "word" does not correspond to a
	// "symbol". Make sure the result contains only alpha-numeric characters
	// or an underscore.
	QString symbol(curText);
	if (!QRegExp("\\w+").exactMatch(symbol))
		return QString();

	return symbol;
}

/**
 * Sets the input focus to the editor.
 * If the editor is currently loading a file, the focus will be set when the
 * process finishes.
 */
void Editor::setFocus()
{
	if (isLoading_)
		onLoadFocus_ = true;
	else
		QsciScintilla::setFocus();
}

/**
 * Fills a Location structure with information on the current file name, line
 * and column.
 * @param  loc  The structure to fill
 */
void Editor::getCurrentLocation(Core::Location& loc)
{
	int line, col;

	// Get the current cursor position.
	getCursorPosition(&line, &col);

	// Fill the structure.
	loc.file_ = path_;
	loc.line_ = line + 1;
	loc.column_ = col + 1;
	loc.text_ = text(line).trimmed();
}

/**
 * Generates a unique title for this editor.
 * The title is used both to identify the editor in the file map managed by the
 * container, as well as for display purposes.
 * @return The editor's title
 */
QString Editor::title() const
{
	if (path_.isEmpty())
		return tr("Untitled %1").arg(newFileIndex_);

	return path_;
}

/**
 * Searches for text inside the document.
 * Prompts the user for the text to find.
 */
void Editor::search()
{
	// Prompt for text to find.
	QString pattern = currentSymbol();
	SearchOptions options;
	if (FindTextDialog::promptPattern(pattern, options, this)
	    != QDialog::Accepted) {
		return;
	}

	// Find the first occurrence of the searched text.
	if (!QsciScintilla::findFirst(pattern, options.regExp_,
	                              options.caseSensitive_,
	                              options.wholeWordsOnly_,
	                              options.wrap_, !options.backward_)) {
		QString msg = tr("'%1' could not be found in the document")
		              .arg(pattern);
		QMessageBox::warning(this, tr("Pattern not found"), msg);
	}
}

/**
 * Searches for the next occurrence of the previously-selected text inside the
 * document.
 */
void Editor::searchNext()
{
	if (!QsciScintilla::findNext()) {
		QString msg = tr("No more matches.");
		QMessageBox::warning(this, tr("Pattern not found"), msg);
	}
}

/**
 * Prompts the user for a line number and moves the cursor to the selected line.
 */
void Editor::gotoLine()
{
	// Get the current line number.
	int line, column;
	getCursorPosition(&line, &column);

	// Prompt for a new line number.
	bool ok;
	line = QInputDialog::getInteger(this, tr("Enter Line Number"),
	                                tr("Line"), line + 1, 1, lines(), 1, &ok);
	if (ok)
		setCursorPosition(line - 1, 0);
}

/**
 * Moves the cursor to the beginning of the current block.
 */
void Editor::gotoBlockBegin()
{
	int line, column, newline;
	getCursorPosition(&line, &column);
	newline = SendScintilla(QsciScintillaBase::SCI_GETFOLDPARENT, line);
	setCursorPosition(newline, 0);
}

/**
 * Called before the editor window is closed.
 * Checks whether the editor can be closed, and if so, accepts the event.
 * @param  event  The close event
 */
void Editor::closeEvent(QCloseEvent* event)
{
	if (canClose()) {
		emit closed(title());
		event->accept();
	}
	else {
		event->ignore();
	}
}

/**
 * Called when the thread loading the file for the editor terminates.
 * Invokes any methods that were deferred while the file was loading.
 * @param  text  The contents of the file
 */
void Editor::loadDone(const QString& text)
{
	setText(text);
	setModified(false);
	isLoading_ = false;
	moveCursor(onLoadLine_, onLoadColumn_);
	setEnabled(true);

	if (onLoadFocus_) {
		setFocus();
		onLoadFocus_ = false;
	}
}

} // namespace Editor

} // namespace KScope
