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

#ifndef __CORE_EXCEPTION_H__
#define __CORE_EXCEPTION_H__

#include <QString>
#include <QMessageBox>

namespace KScope
{

namespace Core
{

/**
 * A simple exception object.
 * Provides a text explaining the cause of the exception.
 * @author Elad Lahav
 */
class Exception
{
public:
	/**
	 * Class constructor.
	 * @param  reason  Describes the cause of the exception
	 */
	Exception(const QString& reason) : reason_(reason) {}

	/**
	 * @return Text describing the cause of the exception
	 */
	const QString& reason() const { return reason_; }

	/**
	 * Shows a message box explaining the exception.
	 */
	void showMessage() const {
		QMessageBox::critical(0, QObject::tr("Exception"), reason_);
	}

private:
	/**
	 * The cause of the exception, as passed to the constructor.
	 */
	QString reason_;
};

} // namespace Core

} // namespace KScope

#endif // __CORE_EXCEPTION_H__
