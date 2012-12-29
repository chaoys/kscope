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

#ifndef __CORE_PROCESS_H
#define __CORE_PROCESS_H

#include <QProcess>
#include "statemachine.h"

namespace KScope
{

namespace Core
{

/**
 * A process with a state-machine parser.
 * @author  Elad Lahav
 */
class Process : public QProcess, public Parser::StateMachine
{
	Q_OBJECT

public:
	Process(QObject* parent = 0);
	~Process();

	void setDeleteOnExit();

signals:
	void parseError();

protected slots:
	virtual void handleFinished(int, QProcess::ExitStatus);
	virtual void handleError(QProcess::ProcessError);
	virtual void handleStateChange(QProcess::ProcessState);

private:
	QString stdOut_;
	bool deleteOnExit_;

private slots:
	void readStandardOutput();
	void readStandardError();
};

}

}

#endif // __CORE_PROCESS_H
