/*
Part of Minetest-c55
Copyright (C) 2010-11 celeron55, Perttu Ahola <celeron55@gmail.com>
Copyright (C) 2011 Ciaran Gultnieks <ciaran@ciarang.com>

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "servercommand.h"
#include "utility.h"
#include <sstream>

void cmd_status(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	os<<ctx->server->getStatusString();
}

void cmd_privs(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	if(ctx->parms.size() == 1)
	{
		// Show our own real privs, without any adjustments
		// made for admin status
		os<<L"-!- " + narrow_to_wide(privsToString(
				ctx->server->getPlayerAuthPrivs(ctx->player->getName())));
		return;
	}

	if((ctx->privs & PRIV_PRIVS) == 0)
	{
		os<<L"-!- You don't have permission to do that";
		return;
	}
		
	Player *tp = ctx->env->getPlayer(wide_to_narrow(ctx->parms[1]).c_str());
	if(tp == NULL)
	{
		os<<L"-!- No such player";
		return;
	}
	
	os<<L"-!- " + narrow_to_wide(privsToString(ctx->server->getPlayerAuthPrivs(tp->getName())));
}

void cmd_grantrevoke(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	if(ctx->parms.size() != 3)
	{
		os<<L"-!- Missing parameter";
		return;
	}

	if((ctx->privs & PRIV_PRIVS) == 0)
	{
		os<<L"-!- You don't have permission to do that";
		return;
	}

	u64 newprivs = stringToPrivs(wide_to_narrow(ctx->parms[2]));
	if(newprivs == PRIV_INVALID)
	{
		os<<L"-!- Invalid privileges specified";
		return;
	}

	Player *tp = ctx->env->getPlayer(wide_to_narrow(ctx->parms[1]).c_str());
	if(tp == NULL)
	{
		os<<L"-!- No such player";
		return;
	}
	
	std::string playername = wide_to_narrow(ctx->parms[1]);
	u64 privs = ctx->server->getPlayerAuthPrivs(playername);

	if(ctx->parms[0] == L"grant")
		privs |= newprivs;
	else
		privs &= ~newprivs;
	
	ctx->server->setPlayerAuthPrivs(playername, privs);
	
	os<<L"-!- Privileges change to ";
	os<<narrow_to_wide(privsToString(privs));
}

void cmd_time(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	if(ctx->parms.size() != 2)
	{
		os<<L"-!- Missing parameter";
		return;
	}

	if((ctx->privs & PRIV_SETTIME) ==0)
	{
		os<<L"-!- You don't have permission to do that";
		return;
	}

	u32 time = stoi(wide_to_narrow(ctx->parms[1]));
	ctx->server->setTimeOfDay(time);
	os<<L"-!- time_of_day changed.";
}

void cmd_shutdown(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	if((ctx->privs & PRIV_SERVER) ==0)
	{
		os<<L"-!- You don't have permission to do that";
		return;
	}

	dstream<<DTIME<<" Server: Operator requested shutdown."
		<<std::endl;
	ctx->server->requestShutdown();
					
	os<<L"*** Server shutting down (operator request)";
	ctx->flags |= 2;
}

void cmd_setting(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	if((ctx->privs & PRIV_SERVER) ==0)
	{
		os<<L"-!- You don't have permission to do that";
		return;
	}

	/*std::string confline = wide_to_narrow(
			ctx->parms[1] + L" = " + ctx->params[2]);*/

	std::string confline = wide_to_narrow(ctx->paramstring);
	
	g_settings.parseConfigLine(confline);
	
	ctx->server->saveConfig();

	os<< L"-!- Setting changed and configuration saved.";
}

void cmd_teleport(std::wostringstream &os,
	ServerCommandContext *ctx)
{
	if((ctx->privs & PRIV_TELEPORT) ==0)
	{
		os<<L"-!- You don't have permission to do that";
		return;
	}

	if(ctx->parms.size() != 2)
	{
		os<<L"-!- Missing parameter";
		return;
	}

	std::vector<std::wstring> coords = str_split(ctx->parms[1], L',');
	if(coords.size() != 3)
	{
		os<<L"-!- You can only specify coordinates currently";
		return;
	}

	v3f dest(stoi(coords[0])*10, stoi(coords[1])*10, stoi(coords[2])*10);
	ctx->player->setPosition(dest);
	ctx->server->SendMovePlayer(ctx->player);

	os<< L"-!- Teleported.";
}

void cmd_banunban(std::wostringstream &os, ServerCommandContext *ctx)
{
	if((ctx->privs & PRIV_BAN) == 0)
	{
		os<<L"-!- You don't have permission to do that";
		return;
	}

	if(ctx->parms.size() < 2)
	{
		std::string desc = ctx->server->getBanDescription("");
		os<<L"-!- Ban list: "<<narrow_to_wide(desc);
		return;
	}
	if(ctx->parms[0] == L"ban")
	{
		Player *player = ctx->env->getPlayer(wide_to_narrow(ctx->parms[1]).c_str());

		if(player == NULL)
		{
			os<<L"-!- No such player";
			return;
		}

		con::Peer *peer = ctx->server->getPeerNoEx(player->peer_id);
		if(peer == NULL)
		{
			dstream<<__FUNCTION_NAME<<": peer was not found"<<std::endl;
			return;
		}
		std::string ip_string = peer->address.serializeString();
		ctx->server->setIpBanned(ip_string, player->getName());
		os<<L"-!- Banned "<<narrow_to_wide(ip_string)<<L"|"
				<<narrow_to_wide(player->getName());
	}
	else
	{
		std::string ip_or_name = wide_to_narrow(ctx->parms[1]);
		std::string desc = ctx->server->getBanDescription(ip_or_name);
		ctx->server->unsetIpBanned(ip_or_name);
		os<<L"-!- Unbanned "<<narrow_to_wide(desc);
	}
}

void cmd_ticket(std::wostringstream &os, ServerCommandContext *ctx)
{
	dstream<<"entering cmd_ticket"<<std::endl;
	if(ctx->parms.size() < 2)
	{
		os<<L"-!- Available ticket commands are: /#ticket new <desc> <message>, /#ticket list";
		if((ctx->privs & PRIV_TICKET) != 0)
			os<<L"<, /#ticket answer <id> <message>, /#ticket close <id>";
		return;
	}

	if(ctx->parms[1] == L"new"){
		dstream<<"New ticket!"<<std::endl;
		if(ctx->parms.size() < 4)
		{
			os<<L"-!- /#ticket new <desc> <message>";
			return;
		}
		std::wstringstream message(std::ios_base::in | std::ios_base::out);

		for(unsigned int i = 3; i<ctx->parms.size(); i++)
		{
			dstream<<"parm "<<i<<std::endl;
			message<<ctx->parms[i]<<L" ";
		}

		bool valid = ctx->server->addTicket(ctx->player->getName(),
				ctx->parms[2], message.str());
		if(valid)
			os<<L"-!- Ticket has been added!";
		else
			os<<L"-!- Your command was wrong, musst be: /ticket new <desc> <message>";
		return;
	}
	if(ctx->parms[1] == L"list")
	{
		os<<ctx->server->getTicketList(ctx->player->getName(), (ctx->privs & PRIV_TICKET));
		return;
	}
	if(ctx->parms[1] == L"read")
	{
		if(ctx->parms.size() != 3)
		{
			os<<L"-!- Wrong format, format is: /#ticket read <id>";
			return;
		}
		unsigned int id = stoi(wide_to_narrow(ctx->parms[2]));
		// TODO: check if format was int
		std::wstring ticket = ctx->server->getTicket(id, ctx->player->getName(), (ctx->privs & PRIV_TICKET));
		if(ticket == L"")
		{
			os<<L"-!- You do not have a ticket with ID: "<<id;
			return;
		}
		os<<ticket;
		return;
	}
	if(ctx->parms[1] == L"answer")
	{
		if(ctx->parms.size() < 4)
		{
			os<<"-!- /#ticket answer <id> <message>";
			return;
		}

		std::wstringstream message(std::ios_base::in | std::ios_base::out);
		for(unsigned int i = 3; i<ctx->parms.size(); i++)
			message<<ctx->parms[i]<<L" ";
		int id = stoi(wide_to_narrow(ctx->parms[2]));
		// TODO: check if it is a number!
		bool notclosed = ctx->server->answerTicket(id, ctx->player->getName(), message.str(),
				(ctx->privs & PRIV_TICKET));
		if(notclosed)
			os<<L"-!- answered to ticket "<<ctx->parms[2];
		else
			os<<L"-!- you can not answer to ticket "<<ctx->parms[2]
				<<L" anymore, it is closed! Or wrong ID.";
		return;
	}
	if((ctx->privs & PRIV_TICKET) == 0)
	{
		os<<L"-!- You are only allowed to do: /#ticket new, /#ticket list, /#ticket answer";
		return;
	}
	if(ctx->parms[1] == L"close")
	{
		if(ctx->parms.size() < 3)
		{
			os<<L"-!- /#ticket close <id>";
			return;
		}

		unsigned int id = atoi(wide_to_narrow(ctx->parms[2]).c_str());
		bool success = ctx->server->closeTicket(id);
		if(success)
			os<<L"-!- ticket "<<ctx->parms[2]<<" has been closed!";
		else
			os<<L"-!- there is no ticket "<<ctx->parms[2];
		return;
	}
	os<<L"-!- No command /#ticket "<<ctx->parms[1]<<L" !";
}


std::wstring processServerCommand(ServerCommandContext *ctx)
{

	std::wostringstream os(std::ios_base::binary);
	ctx->flags = 1;	// Default, unless we change it.

	u64 privs = ctx->privs;

	if(ctx->parms.size() == 0 || ctx->parms[0] == L"help")
	{
		os<<L"-!- Available commands: ";
		os<<L"status privs ";
		if(privs & PRIV_SERVER)
			os<<L"shutdown setting ";
		if(privs & PRIV_SETTIME)
			os<<L" time";
		if(privs & PRIV_TELEPORT)
			os<<L" teleport";
		if(privs & PRIV_PRIVS)
			os<<L" grant revoke";
		if(privs & PRIV_BAN)
			os<<L" ban unban";
		os<<" ticket new";
		if(privs & PRIV_TICKET)
			os<<L" 'ticket list' 'ticket answer' 'ticket close'";
	}
	else if(ctx->parms[0] == L"status")
	{
		cmd_status(os, ctx);
	}
	else if(ctx->parms[0] == L"privs")
	{
		cmd_privs(os, ctx);
	}
	else if(ctx->parms[0] == L"grant" || ctx->parms[0] == L"revoke")
	{
		cmd_grantrevoke(os, ctx);
	}
	else if(ctx->parms[0] == L"time")
	{
		cmd_time(os, ctx);
	}
	else if(ctx->parms[0] == L"shutdown")
	{
		cmd_shutdown(os, ctx);
	}
	else if(ctx->parms[0] == L"setting")
	{
		cmd_setting(os, ctx);
	}
	else if(ctx->parms[0] == L"teleport")
	{
		cmd_teleport(os, ctx);
	}
	else if(ctx->parms[0] == L"ban" || ctx->parms[0] == L"unban")
	{
		cmd_banunban(os, ctx);
	}
	else if(ctx->parms[0] == L"ticket")
	{
		cmd_ticket(os, ctx);
	}
	else
	{
		os<<L"-!- Invalid command: " + ctx->parms[0];
	}
	return os.str();
}


