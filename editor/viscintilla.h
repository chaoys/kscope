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

#ifndef __EDITOR_VISCINTILLA_H__
#define __EDITOR_VISCINTILLA_H__

#include <QKeyEvent>
#include <QMultiHash>
#include <qsciscintilla.h>

namespace KScope
{

namespace Editor
{

/**
 * A sequence of characters with read-only sharing.
 * Used to pass around the Vi command sequence without creating unnecessary
 * copies.
 * @author Elad Lahav
 */
class CharSequence
{
public:
	/**
	 * Default constructor.
	 * Creates the sequence.
	 */
	CharSequence() : isCopy_(false), first_(0), last_(-1) {
		buffer_ = new QList<char>();
	}

	/**
	 * Copy constructor.
	 * Creates a sub-sequence that references the same buffer as the one held
	 * by the source sequence.
	 * @param  source The source sequence
	 * @param  first  The beginning of the sub-sequence (relative to the source)
	 * @param  last   The end of the sub-sequence (relative to the source)
	 */
	CharSequence(const CharSequence& source, int first, int last)
		: buffer_(source.buffer_), isCopy_(true) {
		if (first >= 0)
			first_ = source.first_ + first;
		else
			first_ = source.last_ + first;

		if (last > 0)
			last_ = source.first_ + last;
		else
			last_ = source.last_ + last;
	}

	/**
	 * Class destructor.
	 * Destroys only the main copy of the buffer.
	 */
	~CharSequence() {
		if (!isCopy_)
			delete buffer_;
	}

	/**
	 * Adds a character to the sequence.
	 * Can only be used with the master copy.
	 * @param  c The character to add.
	 */
	void append(char c) {
		if (!isCopy_) {
			buffer_->append(c);
			last_++;
		}
	}

	/**
	 * Removes all characters from the sequence.
	 * Can only be used with the master copy.
	 */
	void clear() {
		first_ = 0;
		last_ = -1;
		if (!isCopy_)
			buffer_->clear();
	}

	/**
	 * @return true if the sequence is empty, false otherwise
	 */
	bool isEmpty() const {
		return first_ > last_;
	}

	/**
	 * @return The number of characters in this sequence
	 */
	int length() const {
		return last_ - first_ + 1;
	}

	/**
	 * Converts the sequence into a number.
	 * @param  ok If not-NULL, used to return a value indicating the success of
	 *            the conversion
	 * @return The numeric value of the sequence, 0 if conversion failed
	 */
	uint toUInt(bool* ok = NULL) const {
		uint result = 0;
		for (int i = first_; i <= last_; i++) {
			char c = (*buffer_)[i];
			if (c < '0' || c > '9') {
				if (ok)
					*ok = false;
				return 0;
			}

			result = (result * 10) + (c - '0');
		}

		if (ok)
			*ok = true;

		return result;
	}

private:
	/**
	 * A pointer to the master copy of the buffer.
	 */
	QList<char>* buffer_;

	/**
	 * Whether this sequence is a copy of the master sequence.
	 */
	bool isCopy_;

	/**
	 * The index of the first character in this sub-sequence.
	 */
	int first_;

	/**
	 * The index of the last character in this sub-sequence.
	 */
	int last_;
};

class ViScintilla;

/**
 * Used to process a Vi command.
 * Each command is associated with the character that invokes it.
 */
struct ViCommand
{
	/**
	 * Possible results for processKey().
	 */
	enum ProcessResult {
		/**
		 * A multi-character command is in progress.
		 */
		Continue,
		/**
		 * The command has finished.
		 */
		Done,
		/**
		 * The command has finished, and the editor should exit Vi normal mode.
		 */
		Exit,
		/**
		 * Key is not recognised by this command.
		 */
		NotHandled
	};

	/**
	 * Handles a key press event.
	 * @param  key    The pressed key, translated into an ASCII code
	 * @param  editor The editor to manipulate
	 * @param  seq    The current command character sequence
	 * @return Result code (@see ProcessResult)
	 */
	virtual ProcessResult processKey(char key, ViScintilla* editor,
	                                 const CharSequence& seq) = 0;
};

/**
 * An implementation of a Vi-style commands on top of Scintilla.
 * @author Elad Lahav
 */
class ViScintilla : public QsciScintilla
{
	Q_OBJECT

public:
	ViScintilla(QWidget*);
	~ViScintilla();

	/**
	 * Editing modes.
	 */
	enum EditMode
	{
		/** Vi compatibility is disabled. */
		Disabled,
		/** Scintilla's default. */
		InsertMode,
		/** Vi normal mode. */
		NormalMode,
		/** Vi visual mode. */
		VisualMode,
	};

	void setEditMode(EditMode mode);
	void nextWord(int, int, int*, int*);

	/**
	 * @return The current edit mode
	 */
	EditMode editMode() const { return mode_; }

	const CharSequence& cmdSequence() const { return cmdSequence_; }

signals:
	/**
	 * Emitted when the editing mode changes.
	 * @param  mode The new mode
	 */
	void editModeChanged(Editor::ViScintilla::EditMode mode);

	/**
	 * Sends a message to be displayed by the application.
	 * @param  msg       The message to display
	 * @param  msTimeOut How long to display the message, in milliseconds
	 */
	void message(const QString& msg, int msTimeOut);

protected:
	/**
	 * The current editing mode.
	 */
	EditMode mode_;

	/**
	 * The current command, based on the first key in the sequence, NULL if no
	 * command is active.
	 */
	ViCommand* curCommand_;

	/**
	 * The current key sequence.
	 */
	CharSequence cmdSequence_;

	/**
	 * A display version of the command sequence.
	 */
	QString cmdString_;

	void keyPressEvent(QKeyEvent*);

private:
	/**
	 * Maps keys to lists of commands.
	 * @author Elad Lahav
	 */
	class CommandMap
	{
	public:
		CommandMap();
		~CommandMap();

		ViCommand* command(char c);

	private:
		/**
		 * The map of characters to commands.
		 */
		ViCommand* cmdMap_[0x100];

		/**
		 * A list of all commands, used by the destructor.
		 */
		QList<ViCommand*> cmdList_;
	};

	/**
	 * A copy of the command map shared by all ViScintilla objects.
	 */
	static CommandMap commandMap_;
};

} // namespace Editor

} // namespace KScope

#endif  // __EDITOR_VISCINTILLA_H__
