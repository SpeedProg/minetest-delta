/*
Minetest-c55
Copyright (C) 2011 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "ticketManager.h"
#include <fstream>
#include <jmutexautolock.h>
#include <sstream>
#include <set>
#include <time.h>
#include "strfnd.h"
#include "debug.h"
#include "utility.h"

TicketManager::TicketManager(const std::string &ticketfilepath):
		m_ticketfilepath(ticketfilepath),
		m_modified(false)
{
	m_mutex.Init();
	try{
		load();
	}
	catch(SerializationError &e)
	{
		dstream<<"WARNING: TicketManager: creating "
				<<m_ticketfilepath<<std::endl;
	}
}

TicketManager::~TicketManager()
{
	save();
}

void TicketManager::load()
{
	JMutexAutoLock lock(m_mutex);
	idcount = 0;
	dstream<<"TicketManager: loading from "<<m_ticketfilepath<<std::endl;
	std::ifstream is(m_ticketfilepath.c_str(), std::ios::binary);
	if(is.good() == false)
	{
		dstream<<"TicketManager: failed loading from "<<m_ticketfilepath<<std::endl;
		throw SerializationError("TicketManager::load(): Couldn't open file");
	}

	for(;;)
	{
		if(is.eof() || is.good() == false)
			break;
		std::string line;
		std::getline(is, line, '\n');
		ticket t;
		//do the split the right way!
		t = deSerialize(line);
		if(t.name == "")
			continue;
		// remove tickets that where closed more than 3 days ago
		if(t.state == "closed" && difftime(time(NULL), t.closetime) > 259200)
		{
			continue;
		}

		m_tickets[idcount] = t;
		idcount++;
	}
	m_modified = false;
}

void TicketManager::save()
{
	JMutexAutoLock lock(m_mutex);
	dstream<<"TicketManager: saving to "<<m_ticketfilepath<<std::endl;
	std::ofstream os(m_ticketfilepath.c_str(), std::ios::binary);
	
	if(os.good() == false)
	{
		dstream<<"TicketManager: failed saving to "<<m_ticketfilepath<<std::endl;
		throw SerializationError("TicketManager::save(): Couldn't open file");
	}

	for(std::map<unsigned int, ticket>::iterator
			i = m_tickets.begin();
			i != m_tickets.end(); i++)
	{
		// remove tickets that where closed more than 3 days ago
		if(i->second.state == "closed" && difftime(time(NULL), i->second.closetime) > 259200)
		{
			continue;
		}
		serialize(i->second, os);
	}
	m_modified = false;
}

bool TicketManager::add(const std::string &name, const std::wstring &desc,
		const std::wstring &message)
{
	if(desc == L"" || name == "" || message == L"")
	{
		return false;
	}
	JMutexAutoLock lock(m_mutex);
	ticket t;
	t.name = name;
	t.desc = wide_to_narrow(desc);
	t.state = "open";
	t.message = name + ": " + wide_to_narrow(message);
	t.closetime = 0;
	m_tickets[idcount] = t;
	m_modified = true;
	dstream<<"TicketManager::add() added Ticket with ID: "<<idcount<<std::endl;
	idcount++;
	return true;
}

bool TicketManager::remove(const unsigned int &id)
{
	JMutexAutoLock lock(m_mutex);
	if(m_tickets.erase(id) == 0)
		return false;

	m_modified = true;
	return true;
}
	
bool TicketManager::answer(const unsigned int &id, const std::string &name,
		const std::wstring &message, const bool &priv)
{
	JMutexAutoLock lock(m_mutex);
	std::map<unsigned int, ticket>::iterator i = m_tickets.find(id);
	if(i == m_tickets.end())
		return false;
	if(i->second.state == "closed")
		return false;
	i->second.message = i->second.message + "\n" + name + ": " + wide_to_narrow(message);
	if(priv)
		i->second.state = "answered";
	else
		i->second.state = "open";
	dstream<<"TicketManager: added answer to ID: "<<id<<std::endl;
	m_modified = true;
	return true;
	
}

bool TicketManager::close(const unsigned int &id)
{
	JMutexAutoLock lock(m_mutex);
	std::map<unsigned int, ticket>::iterator i = m_tickets.find(id);
	if(i == m_tickets.end())
		return false;
	i->second.state = "closed";
	i->second.closetime = time(NULL);
	m_modified = true;
	return true;
}

std::wstring TicketManager::getTicket(const unsigned int &id,
		const std::string &name, const bool &priv)
{
	JMutexAutoLock lock(m_mutex);
	std::map<unsigned int, ticket>::iterator i = m_tickets.find(id);
	if(i == m_tickets.end())
		return L"";
	if(priv || i->second.name == name)
		return L"ID: " + narrow_to_wide(itos(id)) + L" STATUS: "+ narrow_to_wide(i->second.state) + L":\n" + narrow_to_wide(i->second.message);
	return L"";
}

std::wstring TicketManager::getList(const std::string &name, const bool &priv)
{
	JMutexAutoLock lock(m_mutex);
	std::wstring o;
	for(std::map<unsigned int, ticket>::iterator i = m_tickets.begin();
		i != m_tickets.end(); i++)
	{
		if(!priv && name != i->second.name)
		       continue;	
		o = o + L"* ID: " + narrow_to_wide(itos(i->first)) 
			+ L" STATE: " + narrow_to_wide(i->second.state)
			+ L" NAME: " + narrow_to_wide(i->second.name)
			+ L" DESC: " + narrow_to_wide(i->second.desc) + L"\n";
	}
	if(o == L"")
		o = L"-!- no tickets!";
	return o;
}

bool TicketManager::isModified()
{
	JMutexAutoLock lock(m_mutex);
	return m_modified;
}

void TicketManager::replace(std::string &target, const std::string &find, const std::string &replace)
{
	size_t s_from = 0;
	for(;;)
	{
		size_t f_at = target.find(find, s_from);
		if(f_at == std::string::npos)
			break;
		target.replace(f_at, find.size(), replace);
		s_from = f_at + replace.size();
	}
}
void TicketManager::serialize(const ticket &target, std::ofstream &os)
{
		std::string message = target.message;
		std::string name = target.name;
		std::string desc = target.desc;
		replace(message, "\\", "\\\\");
		replace(message, "\n", "\\n");
		replace(message, "|", "\\|");
		replace(name, "\\", "\\\\");
		replace(name, "\n", "\\n"); //should not contain any \n
		replace(name, "|", "\\|");
		replace(desc, "\\", "\\\\");
		replace(desc, "\n", "\\n"); //should not contain any \n
		replace(desc, "|", "\\|");
		os<<name<<"|"<<desc<<"|"<<target.state<<"|"<<target.closetime<<"|"<<message<<"\n";
}
/*
   \\n(zeinlenumbruch) -> \\\\n\n
*/
ticket TicketManager::deSerialize(std::string &target)
{
	ticket t;
	size_t s_from = 0;
	size_t lastend = 0;
	short step = 0;
	for(;;)
	{
		size_t f_slash_at = target.find("\\", s_from);
		size_t f_pipe_at = target.find("|", s_from);
		if(f_slash_at == std::string::npos && f_pipe_at == std::string::npos)
		{
			t.message = target.substr(lastend, target.size()-lastend);
			break;
		}
		if(f_slash_at == std::string::npos || f_pipe_at < f_slash_at)
		{
			if(step == 0)
			{
				t.name=target.substr(lastend, f_pipe_at-lastend);
				s_from = f_pipe_at + 1;
				step = 1;
				lastend = s_from;
			}
			else if(step == 1)
			{
				t.desc=target.substr(lastend, f_pipe_at-lastend);
				s_from = f_pipe_at + 1;
				step = 2;
				lastend = s_from;
			}
			else if(step == 2)
			{
				t.state=target.substr(lastend, f_pipe_at-lastend);
				s_from = f_pipe_at + 1;
				step = 3;
				lastend = s_from;
			}
			else if(step == 3)
			{
				std::stringstream timestream(std::ios_base::in | std::ios_base::out);
				timestream<<target.substr(lastend, f_pipe_at-lastend);
				timestream>>t.closetime;
				s_from = f_pipe_at + 1;
				step = 4;
				lastend = s_from;
			}
		}
		else if(f_pipe_at == std::string::npos || f_slash_at < f_pipe_at)
		{
			if(target[f_slash_at+1] == '\\')
			{
				target.replace(f_slash_at, 1, "");
				s_from =  f_slash_at + 1;
			}
			else if(target[f_slash_at+1] == 'n')
			{
				target.replace(f_slash_at, 2, "\n");
				s_from = f_slash_at + 1;
			}
			else if(target[f_slash_at+1] == '|')
			{
				target.replace(f_slash_at, 2, "|");
				s_from = f_slash_at + 1;
			}
		}
	}
	return t;
}

