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

#include "process.h"

namespace KScope
{

namespace Core
{

Process::Process(QObject* parent) : QProcess(parent)
{
	connect(this, SIGNAL(readyReadStandardOutput()), this,
	        SLOT(readStandardOutput()));
	connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), this,
	        SLOT(handleFinished(int, QProcess::ExitStatus)));
	connect(this, SIGNAL(error(QProcess::ProcessError)), this,
			SLOT(handleError(QProcess::ProcessError)));
	connect(this, SIGNAL(stateChanged(QProcess::ProcessState)), this,
	        SLOT(handleStateChange(QProcess::ProcessState)));
}

Process::~Process()
{
}

void Process::setDeleteOnExit()
{
	deleteOnExit_ = true;
}

void Process::readStandardOutput()
{
	// Read from standard output.
	stdOut_ += readAllStandardOutput();

	//Have to remove CR, otherwise cscope result parser would fail
	stdOut_.remove("\r", Qt::CaseSensitive);

	// Parse the text.
	if (!parse(stdOut_)) {
		emit parseError();
		return;
	}
}

void Process::readStandardError()
{
	qDebug() << readAllStandardError();
}

void Process::handleFinished(int code, QProcess::ExitStatus status)
{
	qDebug() << "Process finished" << code << status;
}

void Process::handleError(QProcess::ProcessError code)
{
	qDebug() << "Process error" << code;
}

void Process::handleStateChange(QProcess::ProcessState state)
{
	qDebug() << "Process state" << state;
	if (state == QProcess::NotRunning && deleteOnExit_)
		deleteLater();
}

}

}
