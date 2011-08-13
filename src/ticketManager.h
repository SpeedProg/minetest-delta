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

#ifndef TICKET_HEADER
#define TICKET_HEADER

#include <map>
#include <string>
#include <jthread.h>
#include <jmutex.h>
#include "common_irrlicht.h"
#include "exceptions.h"
#include <time.h>

struct ticket
{
	std::string name;
	std::string desc;
	std::string state;
	std::string message;
	time_t closetime;
};

class TicketManager
{
public:
	TicketManager(const std::string &bannfilepath);
	~TicketManager();
	void load();
	void save();
	bool add(const std::string &name, const std::wstring &desc,
			const std::wstring &message);
	bool remove(const unsigned int &id);
	bool answer(const unsigned int &id, const std::string &name,
			const std::wstring &message, const bool &priv);
	bool close(const unsigned int &id);
	std::wstring getTicket(const unsigned int &id, const std::string &name, const bool &priv);
	std::wstring getStatus(const unsigned int &id);
	std::wstring getList(const std::string &name, const bool &priv);
	bool isModified();
private:
	JMutex m_mutex;
	std::string m_ticketfilepath;
	std::map<unsigned int, ticket> m_tickets;
	unsigned int idcount;
	bool m_modified;
	void replace(std::string &target, const std::string &find,
			const std::string &replace);
	ticket deSerialize(std::string &target);
	void serialize(const ticket &target, std::ofstream &os);
};

#endif

