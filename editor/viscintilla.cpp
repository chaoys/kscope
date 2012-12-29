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

#include <QDebug>
#include "viscintilla.h"

namespace KScope
{

namespace QtKey
{

/**
 * Defines a set of functions for comparing two keys.
 * @author Elad Lahav
 */
template<int Key1, int Key2>
struct Compare
{
	static const bool gt_ = (Key1 > Key2);
	static const bool lt_ = (Key1 < Key2);
	static const bool ge_ = (Key1 >= Key2);
	static const bool le_ = (Key1 <= Key2);
};

/**
 * Determines whether the template parameter is a letter key.
 */
template<int Key>
struct IsLetter
{
	static const bool result_
		= (Compare<Key, Qt::Key_A>::ge_) && (Compare<Key, Qt::Key_Z>::le_);
};

/**
 * Determines whether the template parameter is a digit key.
 * @author Elad Lahav
 */
template<int Key>
struct IsDigit
{
	static const bool result_
		= (Compare<Key, Qt::Key_0>::ge_) && (Compare<Key, Qt::Key_9>::le_);
};

/**
 * Provides compile- and run-time functions for translating a key into an ASCII
 * character.
 * @author Elad Lahav
 */
template<int Key = 0>
struct ToChar
{
	static const char char_ = (IsLetter<Key>::result_
	                           ? (Key - Qt::Key_A) + 'A'
	                           : (IsDigit<Key>::result_
	                              ? (Key - Qt::Key_0) + '0' : 0));
	static char transform(char c) {
		if (c >= Qt::Key_0 && c <= Qt::Key_9)
			return (c - Qt::Key_0) + '0';
		if (c >= Qt::Key_A && c <= Qt::Key_Z)
			return (c - Qt::Key_A) + 'A';
		return 0;
	}
};

/**
 * Provides compile- and run-time functions for translating a key into an ASCII
 * control character.
 * @author Elad Lahav
 */
template<int Key = 0>
struct ToControlChar
{
	static const char char_ = (IsLetter<Key>::result_
	                           ? (Key - Qt::Key_A) + 1 : 0);
	static const uchar uchar_ = static_cast<uchar>(char_);
	static char transform(char c) {
		if (c >= Qt::Key_A && c <= Qt::Key_Z)
			return (c - Qt::Key_A) + 1;
		return 0;
	}
};

} // namespace QtKey

namespace Editor
{

/**
 * Implements the empty (move) action for a move command.
 * @author Elad Lahav
 */
struct MoveAction
{
	static ViCommand::ProcessResult
	action(ViScintilla* editor) {
		(void)editor;
		return ViCommand::Done;
	}
};

/**
 * Implements the yank action for a move command.
 * @author Elad Lahav
 */
struct YankAction
{
	static ViCommand::ProcessResult
	action(ViScintilla* editor) {
		// Copy selected text.
		editor->copy();

		// Clear selection.
		int position
			= editor->SendScintilla(QsciScintillaBase::SCI_GETSELECTIONSTART);
		editor->SendScintilla(QsciScintillaBase::SCI_GOTOPOS, position);
		return ViCommand::Done;
	}
};

/**
 * Implements the cut action for a move command.
 * @author Elad Lahav
 */
struct CutAction
{
	static ViCommand::ProcessResult
	action(ViScintilla* editor) {
		// Cut selected text.
		editor->cut();
		return ViCommand::Done;
	}
};

/**
 * Implements the change (delete+exit normal mode) action for a move command.
 * @author Elad Lahav
 */
struct ChangeAction
{
	static ViCommand::ProcessResult
	action(ViScintilla* editor) {
		// Delete selected text and exit normal mode.
		editor->removeSelectedText();
		return ViCommand::Exit;
	}
};

/**
 * Handles cursor movement commands.
 * @author Elad Lahav
 */
template<bool Select = false, class ActionT = MoveAction>
struct MoveCommand : public ViCommand
{
	ProcessResult processKey(char key, ViScintilla* editor,
	                         const CharSequence& seq) {
		bool relative;
		uint message;

		switch (key) {
		case 'l':
			// Go right <NUM> characters.
			relative = true;
			message = Select ? QsciScintillaBase::SCI_CHARRIGHTEXTEND
			                 : QsciScintillaBase::SCI_CHARRIGHT;
			break;

		case 'h':
			// Go left <NUM> characters.
			relative = true;
			message = Select ? QsciScintillaBase::SCI_CHARLEFTEXTEND
			                 : QsciScintillaBase::SCI_CHARLEFT;
			break;

		case 'k':
			// Go up <NUM> lines.
			relative = true;
			message = Select ? QsciScintillaBase::SCI_LINEUPEXTEND
			                 : QsciScintillaBase::SCI_LINEUP;
			break;

		case 'j':
			// Go down <NUM> lines.
			relative = true;
			message = Select ? QsciScintillaBase::SCI_LINEDOWNEXTEND
			                 : QsciScintillaBase::SCI_LINEDOWN;
			break;

		case 'w':
			// Go left <NUM> words
			relative = true;
			message = Select ? QsciScintillaBase::SCI_WORDRIGHTEXTEND
			                 : QsciScintillaBase::SCI_WORDRIGHT;
			break;

		case '0':
			// Go to the beginning of the line.
			relative = false;
			message = Select ? QsciScintillaBase::SCI_HOMEEXTEND
			                 : QsciScintillaBase::SCI_HOME;
			break;

		case '$':
			// Go to the end of the line.
			relative = false;
			message = Select ? QsciScintillaBase::SCI_LINEENDEXTEND
			                 : QsciScintillaBase::SCI_LINEEND;
			break;

		case 'g':
			// Go to the beginning of the file.
			relative = false;
			message = Select ? QsciScintillaBase::SCI_DOCUMENTSTARTEXTEND
			                 : QsciScintillaBase::SCI_DOCUMENTSTART;
			break;

		case 'G':
			// Go to the end of the file.
			relative = false;
			message = Select ? QsciScintillaBase::SCI_DOCUMENTENDEXTEND
			                 : QsciScintillaBase::SCI_DOCUMENTEND;
			break;

		default:
			return NotHandled;
		}

		// Compute the multiplier value.
		int multiplier = 1;
		if (relative && seq.length() > 1) {
			bool ok;
			multiplier = CharSequence(seq, 0, -1).toUInt(&ok);
			if (!ok)
				return NotHandled;
		}

		// Move the cursor.
		for (int i = 0; i < multiplier; i++)
			editor->SendScintilla(message);

		return ActionT::action(editor);
	}
};

/**
 * Common handler for commands that take the form <CHAR> <ARG> <CHAR>. Examples
 * include yank, cut and change.
 * Supported variants include:
 * - <CHAR><CHAR>: do action on a the current line,
 * - <CHAR><NUMBER><CHAR>: do action on <NUMBER> lines,
 * - <CHAR><MOVE_COMMAND>: do action on a range starting from the current cursor
 *                         position to the one resulting from the move command.
 * @author Elad Lahav
 */
template<char C, class ActionT>
struct XyXCommand : public ViCommand
{
	ProcessResult processKey(char key, ViScintilla* editor,
	                         const CharSequence& seq) {
		switch (key) {
		case C:
			// Check if this is the first occurrence of C.
			if (seq.length() == 1)
				return Continue;
			break;

		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			return Continue;

		case '0':
			if (seq.length() > 2)
				return Continue;
			// Fall through...

		default:
			{
				MoveCommand<true, ActionT> moveCmd;
				return moveCmd.processKey(key, editor, CharSequence(seq, 1, 0));
			}
		}

		// Got here on a second occurrence of C.
		// Perform the action on a number of whole lines.
		int multiplier = 1;
		if (seq.length() > 2) {
			bool ok;
			multiplier = CharSequence(seq, 1, -1).toUInt(&ok);
			if (!ok)
				return NotHandled;
		}

		// Select the given number of lines.
		editor->SendScintilla(QsciScintillaBase::SCI_VCHOME);
		for (int i = 0; i < multiplier; i++)
			editor->SendScintilla(QsciScintillaBase::SCI_LINEDOWNEXTEND);

		// Perform the action on the selected lines.
		return ActionT::action(editor);
	}
};

/**
 * Pastes text from the clipboard.
 * @author Elad Lahav
 */
struct PasteCommand : public ViCommand
{
	ProcessResult processKey(char key, ViScintilla* editor,
	                         const CharSequence& seq) {
		(void)seq;

		switch (key) {
		case 'P':
			// Paste here.
			editor->paste();
			return Done;

		case 'p':
			break;

		default:
			return NotHandled;
		}

		// Get the current cursor position.
		int line, column;
		editor->getCursorPosition(&line, &column);

		// TODO: Is this the right behaviour?
		if (column == 0)
			line++;
		else
			column++;

		// Move to the new position and paste.
		editor->setCursorPosition(line, column);
		editor->paste();
		return Done;
	}
};

/**
 * Aborts Vi mode.
 * Different keys place the cursor in different locations:
 * - i: Current position
 * - a: Next column
 * - I: Beginning of current line
 * - A: End of current line
 * @author Elad Lahav
 */
struct InsertCommand : public ViCommand
{
	ProcessResult processKey(char key, ViScintilla* editor,
	                         const CharSequence& seq) {
		(void)seq;

		// Get the current cursor position.
		int line, column;
		editor->getCursorPosition(&line, &column);

		switch (key) {
		case 'i':
			break;

		case 'a':
			column++;
			break;

		case 'I':
			column = 0;
			break;

		case 'A':
			column = editor->lineLength(line) - 1;
			break;

		default:
			return NotHandled;
		}

		// Move to the new position and exit normal mode.
		editor->setCursorPosition(line, column);
		return Exit;
	}
};

/**
 * Performs the undo (u) and redo (Ctrl+r) actions.
 * @author Elad Lahav
 */
struct UndoRedoCommand : public ViCommand
{
	ProcessResult processKey(char key, ViScintilla* editor,
	                         const CharSequence& seq) {
		int multiplier = 1;
		if (seq.length() > 1) {
			bool ok;
			multiplier = CharSequence(seq, 0, -1).toUInt(&ok);
			if (!ok)
				return NotHandled;
		}

		// Get the current cursor position.
		switch (key) {
		case 'u':
			for (int i = 0; i < multiplier; i++)
				editor->undo();
			break;

		case QtKey::ToControlChar<Qt::Key_R>::char_:
			for (int i = 0; i < multiplier; i++)
				editor->redo();
			break;

		default:
			return NotHandled;
		}

		return Done;
	}
};

/**
 * Creates a new line, either above or below the current one, and exits normal
 * mode.
 */
struct OpenLineCommand : public ViCommand
{
	ProcessResult processKey(char key, ViScintilla* editor,
	                         const CharSequence& seq) {
		(void)seq;

		// Get the current cursor position.
		int line, column;
		editor->getCursorPosition(&line, &column);

		switch (key) {
		case 'o':
			line++;
			break;

		case 'O':
			break;

		default:
			return NotHandled;
		}

		// Add a line at the new position and exit normal mode.
		editor->setCursorPosition(line, 0);
		editor->insert("\n");
		return Exit;
	}
};

/**
 * Handles all commands that start with a number (other than 0).
 * Currently supports movement, undo and redo only. Yank, delete, change, etc.
 * must start with a character (e.g., d2d rather than 2dd).
 * @author Elad Lahav
 */
struct NumPrefixCommand : public ViCommand
{
	ProcessResult processKey(char key, ViScintilla* editor,
	                         const CharSequence& seq) {
		// Consume digits.
		if (key >= '0' && key <= '9')
			return Continue;

		switch (key) {
		case 'l':
		case 'h':
		case 'k':
		case 'j':
		case 'w':
			return MoveCommand<>().processKey(key, editor, seq);

		case 'u':
		case QtKey::ToControlChar<Qt::Key_R>::char_:
			return UndoRedoCommand().processKey(key, editor, seq);
		}

		return NotHandled;
	}
};

ViScintilla::CommandMap ViScintilla::commandMap_;

/**
 * Class constructor.
 * @param  parent Parent widget
 */
ViScintilla::ViScintilla(QWidget* parent)
	: QsciScintilla(parent), mode_(Disabled), curCommand_(NULL)
{
}

/**
 * Class destructor.
 */
ViScintilla::~ViScintilla()
{
}

/**
 * Changes the edit mode.
 * @param  mode The new mode to set
 */
void ViScintilla::setEditMode(EditMode mode)
{
	if (mode != mode_) {
		// Set the new mode.
		mode_ = mode;

		// Change the caret style.
		// TODO: Handle overwrite in disabled/insert mode.
		switch (mode_) {
		case Disabled:
		case InsertMode:
			SendScintilla(SCI_SETCARETSTYLE, CARETSTYLE_LINE);
			break;

		case NormalMode:
		case VisualMode:
			SendScintilla(SCI_SETCARETSTYLE, CARETSTYLE_BLOCK);
			break;
		}

		// Notify of the change.
		emit editModeChanged(mode_);
	}
}

/**
 * Handles a key press.
 * If Vi compatibility is disabled or the current editing mode is insert, the
 * event is handled by Scintilla. Otherwise, the key is intercepted and
 * interpreted by the Vi command structure.
 * @param  event The key event
 */
void ViScintilla::keyPressEvent(QKeyEvent* event)
{
	switch (mode_) {
	case Disabled:
		// Let Scintilla handle all events.
		QsciScintilla::keyPressEvent(event);
		return;

	case InsertMode:
		// Intercept the ESC key for entering normal mode.
		if ((event->key() == Qt::Key_Escape)
		    && (event->modifiers() == Qt::NoModifier)) {
			setEditMode(NormalMode);
			event->setAccepted(true);
		}
		else {
			// Anything but ESC.
			QsciScintilla::keyPressEvent(event);
		}
		return;

	case NormalMode:
	case VisualMode:
		break;
	}

	event->setAccepted(false);

	// Translate keys, so that we only deal with ASCII characters.
	char key, dispKey = 0;
	switch (event->modifiers()) {
	case Qt::ControlModifier:
		key = QtKey::ToControlChar<>::transform(event->key());
		dispKey = QtKey::ToChar<>::transform(event->key());
		break;

	case Qt::NoModifier:
		switch (event->key()) {
		case Qt::Key_Right:
			key = 'l';
			break;

		case Qt::Key_Left:
			key = 'h';
			break;

		case Qt::Key_Up:
			key = 'k';
			break;

		case Qt::Key_Down:
			key = 'j';
			break;

		case Qt::Key_Escape:
			// Abort the current command.
			if (curCommand_) {
				emit message(QString("Aborted: ") + cmdString_, 1000);
				curCommand_ = NULL;
				cmdSequence_.clear();
				cmdString_ = "";
			}
			event->setAccepted(true);
			return;

		default:
			key = event->text()[0].toAscii();
		}
		break;

	case Qt::ShiftModifier:
		key = event->text()[0].toAscii();
		break;

	default:
		return;
	}

	if (key == 0)
		return;

	// Add to the current command sequence.
	cmdSequence_.append(key);
	if (event->modifiers() & Qt::ControlModifier)
		cmdString_.append("(Ctrl)");
	cmdString_.append(dispKey ? dispKey : key);

	// Process the key event.
	ViCommand::ProcessResult result;

	if (curCommand_) {
		// If a command is in progress, let it process the key.
		result = curCommand_->processKey(key, this, cmdSequence_);
	}
	else {
		// This is the first key in a new command.
		// Check which command is associated with this key.
		curCommand_ = commandMap_.command(key);
		if (curCommand_)
			result = curCommand_->processKey(key, this, cmdSequence_);
		else
			result = ViCommand::NotHandled;
	}

	switch (result) {
	case ViCommand::Continue:
		// A command is in progress.
		emit message(cmdString_, 1000);
		break;

	case ViCommand::Exit:
		// Return to insert mode.
		setEditMode(InsertMode);
		// Fall through...

	case ViCommand::Done:
		curCommand_ = NULL;
		// Report multi-character commands.
		if (cmdSequence_.length() > 1)
			emit message(cmdString_, 1000);
		cmdSequence_.clear();
		cmdString_ = "";
		break;

	case ViCommand::NotHandled:
		// Unrecognised command sequence.
		curCommand_ = NULL;
		emit message(tr("Bad sequence: %1").arg(cmdString_), 2000);
		cmdSequence_.clear();
		cmdString_ = "";
		return;
	}

	event->setAccepted(true);
}

/**
 * Class constructor.
 * Creates the command objects, and puts them in the key-to-command map.
 */
ViScintilla::CommandMap::CommandMap()
{
	ViCommand* moveCmd = new MoveCommand<>;
	ViCommand* yankCmd = new XyXCommand<'y', YankAction>;
	ViCommand* cutCmd = new XyXCommand<'d', CutAction>;
	ViCommand* changeCmd = new XyXCommand<'c', ChangeAction>;
	ViCommand* pasteCmd = new PasteCommand;
	ViCommand* insCmd = new InsertCommand;
	ViCommand* undoCmd = new UndoRedoCommand;
	ViCommand* numCmd = new NumPrefixCommand;
	ViCommand* openCmd = new OpenLineCommand;

	// Add all commands to a list, so they can be deleted when the map is
	// destroyed.
	cmdList_ << moveCmd << yankCmd << cutCmd << changeCmd << pasteCmd << insCmd
	         << undoCmd << numCmd;

	// Clear the map.
	memset(cmdMap_, 0, sizeof(cmdMap_));

	// Insert commands to the map.
	cmdMap_['0'] = moveCmd;
	cmdMap_['1'] = numCmd;
	cmdMap_['2'] = numCmd;
	cmdMap_['3'] = numCmd;
	cmdMap_['4'] = numCmd;
	cmdMap_['5'] = numCmd;
	cmdMap_['6'] = numCmd;
	cmdMap_['7'] = numCmd;
	cmdMap_['8'] = numCmd;
	cmdMap_['9'] = numCmd;
	cmdMap_['l'] = moveCmd;
	cmdMap_['h'] = moveCmd;
	cmdMap_['k'] = moveCmd;
	cmdMap_['j'] = moveCmd;
	cmdMap_['w'] = moveCmd;
	cmdMap_['$'] = moveCmd;
	cmdMap_['g'] = moveCmd;
	cmdMap_['G'] = moveCmd;
	cmdMap_['y'] = yankCmd;
	cmdMap_['d'] = cutCmd;
	cmdMap_['c'] = changeCmd;
	cmdMap_['p'] = pasteCmd;
	cmdMap_['P'] = pasteCmd;
	cmdMap_['i'] = insCmd;
	cmdMap_['a'] = insCmd;
	cmdMap_['I'] = insCmd;
	cmdMap_['A'] = insCmd;
	cmdMap_['u'] = undoCmd;
	cmdMap_[QtKey::ToControlChar<Qt::Key_R>::uchar_] = undoCmd;
	cmdMap_['o'] = openCmd;
	cmdMap_['O'] = openCmd;
}

/**
 * Class destructor.
 * Deletes the command objects.
 */
ViScintilla::CommandMap::~CommandMap()
{
	foreach (ViCommand* cmd, cmdList_)
		delete cmd;
}

/**
 * Returns the command handling the given character.
 * @param  c The character to map
 * @return The handling command, NULL if no command is assigned to this
 *         character.
 */
ViCommand* ViScintilla::CommandMap::command(char c)
{
	int index = static_cast<uint>(c);
	if (index < 0 || index >= 0x100)
		return NULL;

	return cmdMap_[index];
}

} // namespace Editor

} // namespace KScope
