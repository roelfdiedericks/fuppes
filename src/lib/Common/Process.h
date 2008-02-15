/***************************************************************************
 *            Process.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2008 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef PROCESS_H
#define PROCESS_H

#ifdef HAVE_CONFIG
#include "../../config.h"
#endif

#include "Common.h"

#include <unistd.h>
#include <string>
#include <map>
#include <signal.h>
#include <sys/wait.h>

class CProcess;

class CProcessMgr {
	
	public:
		CProcessMgr();
		~CProcessMgr();
	
		static void	init();
		static void signal(pid_t pid, int sig);
	
		static void register_proc(CProcess* proc);
		static void unregister_proc(CProcess* proc);
	
	private:
		static CProcessMgr* 									m_instance;
	
		std::map<pid_t, CProcess*>						m_processes;
		std::map<pid_t, CProcess*>::iterator	m_processesIter;
		fuppesThreadMutex											m_mutex;
};


class CProcess {
	
	friend class CProcessMgr;
	
	public:
		CProcess();
		~CProcess();
		
		bool	start(std::string cmd);
		void	stop();
		bool	isRunning() { return m_isRunning; }
		void	waitFor();
		
		void	setInFile(std::string fileName) { m_inFile = fileName; }
		void	setOutFile(std::string fileName) { m_outFile = fileName; }
	
		pid_t	pid() { return m_pid; }
	
	private:
		void parseArgs(std::string cmd);
		
		bool					m_isRunning;
		char const**	m_args;
		int						m_numArgs;
		
		std::string		m_inFile;
		std::string		m_outFile;
	
#ifndef WIN32
		pid_t	m_pid;
#else
#endif
		
};


#endif // PROCESS_H
