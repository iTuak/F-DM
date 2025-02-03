
//========================================================
/*

		███████╗██╗░░░░░░█████╗░░██████╗██╗░░██╗
		██╔════╝██║░░░░░██╔══██╗██╔════╝██║░░██║
		█████╗░░██║░░░░░███████║╚█████╗░███████║
		██╔══╝░░██║░░░░░██╔══██║░╚═══██╗██╔══██║
		██║░░░░░███████╗██║░░██║██████╔╝██║░░██║
		╚═╝░░░░░╚══════╝╚═╝░░╚═╝╚═════╝░╚═╝░░╚═╝
		Deathmatch: iTuak

*/
//========================================================

#include <open.mp>
#include <a_mysql>
#include <fixes>
#include <assist>
#include <sscanf2>
#include <streamer>
#include <foreach>
#include <crashdetect>
#include <mSelection>
#include <weapon-config>
#include <mobile-checker>
#include <Pawn.CMD>
#include <Pawn.RakNet>

#include "modules/colors.inc"
#include "modules/mapping.inc"
#include "modules/textdraw.inc"

#define SERVER_VERSION "[v1.1b]"
#define IN_LOCAL true

#if IN_LOCAL == false
	#define 	MYSQL_HOST 				"109.176.229.200"
	#define 	MYSQL_USER 				"u1828_XQMFKNS8OP"
	#define 	MYSQL_PASS 				"2c^q@maxULaPgF^TEmnzdFUR"
	#define 	MYSQL_DBSE   			"s1828_entod"
#else
	#define 	MYSQL_HOST 				"localhost"
	#define 	MYSQL_USER 				"root"
	#define 	MYSQL_PASS 				""
	#define 	MYSQL_DBSE   			"fdmdb"
#endif

#define MAX_REPORTS 	30
#define MAX_ASKS 		30
#define WC_MAX_WEAPON_NAME 21

#define function%0(%1) forward%0(%1); public%0(%1)
#define PRESSED(%0) (((newkeys & (%0)) == (%0)) && ((oldkeys & (%0)) != (%0)))
#define ClearChat(%0,%1) for(new n = 0; n < %1; n++) SendClientMessage(%0, -1, " ")

#define erase(%0)                   (%0[0]='\0')
#define strmatch(%1,%2)             (!strcmp(%1,%2,true))
stock szBigString[256];
stock szSmallString[32];

#define InfoMessage(%0,%1) 	SendClientMessage(%0, COLOR_SERVER, "Notice: "COL_WHITE""%1)
#define UsageMessage(%0,%1) SendClientMessage(%0, COLOR_ORANGE, "Usage: "COL_WHITE""%1)
#define ErrorMessage(%0,%1) SendClientMessage(%0, COLOR_RED, "Error: "COL_WHITE""%1)
#define StreakMessage(%0,%1) SendClientMessage(%0, COLOR_GRAY, "Streak: "COL_WHITE""%1)
#define AdminMessage(%0,%1) SendClientMessage(%0, COLOR_ANTICHEAT, "A: "COL_WHITE""%1)
#define NoAdmin(%0) SendClientMessage(%0, COLOR_ANTICHEAT, "A: "COL_WHITE"You not authorized to use this")
#define ReportMessage(%0,%1) SendClientMessage(%0, COLOR_PINK, "Report: "COL_WHITE""%1)
#define AskMessage(%0,%1) SendClientMessage(%0, COLOR_PINK, "Ask: "COL_WHITE""%1)
#define VehicleMessage(%0,%1) SendClientMessage(%0, COLOR_BLUE, "Vehicle: "COL_WHITE""%1)
#define DuelMessage(%0,%1) SendClientMessage(%0, COLOR_DUEL, "Duel: "COL_WHITE""%1)

enum PlayerData {
	pID,
	pIP[16],
	pName[MAX_PLAYER_NAME],
	pRegDate[24],
	pLastLogin[24],
	pAdmin,
	pSkin,
	pScore,
	pMoney,
	pSeconds,
	pMinutes,
	pHours,
	pKills,
	pDeaths,
	pHeadshot,
	pAssist,
	pMuted,
    pMuteTime,
    pArenaKills,
    pDuelWins,

	//=============[ Not Save ]
	bool:pLoggedIn,
	bool:pLobbyIn,
	pDmIn,
	bool:pFreeroamIn,
	bool:pDuelIn

}
new pData[MAX_PLAYERS][PlayerData];

enum ReportData
{
	rID,
	rName[MAX_PLAYER_NAME],
	rText[50]
}
new rData[MAX_REPORTS][ReportData];
new Iterator: Reports<MAX_REPORTS>;

enum AskData
{
	aID,
	aName[MAX_PLAYER_NAME],
	aText[50]
}
new aData[MAX_ASKS][AskData];
new Iterator: Asks<MAX_ASKS>;

enum{
	DIALOG_UNUSED,
	DIALOG_REGISTER,
	DIALOG_LOGIN, 
	DIALOG_DEATHMATCH,
	DIALOG_HELP,
	DIALOG_REPORT,
	DIALOG_REPORTS,
	DIALOG_ASK,
	DIALOG_ASKS,
	SERVER_SETTINGS,
	DIALOG_SETTINGS,
	SETTINGS_PASSWORD,
	DIALOG_VEHICLES,
	DIALOG_DUEL,
    DIALOG_DUEL_PLAYER,
    DIALOG_DUEL_LOCATION,
    DIALOG_DUEL_WEAPON,
    DIALOG_DUEL_WEAPON_TWO,
    DIALOG_DUEL_HEALTH,
    DIALOG_DUEL_ARMOUR,
};

enum E_DUEL_DATA
{
	E_PLAYER,
	E_WEAPON[2],
	E_COUNTDOWN,
	Float:E_ARMOUR, 				
	Float:E_HEALTH, 					
	E_LOCATION_ID,
	E_TIMER
}
new g_duelData[MAX_PLAYERS][E_DUEL_DATA];

enum E_DUEL_LOCATION_DATA
{
	E_NAME[22],					
	Float:E_POS_ONE[4], 				
	Float:E_POS_TWO[4]
};
new g_duelLocationData 				[][E_DUEL_LOCATION_DATA] =
{
	{ "SF Bridge",				{ -1393.5670, 681.3549 ,3.0703, 212.1914 }, { -1378.9456, 660.3615, 3.0703, 33.3649 } },
	{ "Star Power",				{ 1553.6558, -1353.5206, 329.4585, 93.8621  }, { 1531.1318, -1353.1356, 329.4535, 269.8542 } },
	{ "Ghost Town",				{ -403.4645, 2249.0618, 42.4297, 101.5275 }, { -388.7307, 2197.8704, 42.4235, 276.4343 } },
	{ "Tennis Courts",			{ 755.93790, -1280.710, 13.5565, 1.3250 }, { 755.93960, -1238.688, 13.5516, 175.2829 } },
	{ "Underwater World",		{ 520.59600, -2125.663, -28.257, 357.6445 }, { 517.96600, -2093.610, -28.257, 189.6402 } },
	{ "Grove Street",			{ 2476.4580, -1668.631, 13.3249, 271.4538 }, { 2501.1560, -1667.655, 13.3559, 90.0969 } },
	{ "Ocean Docks",			{ 2683.5440, -2485.137, 13.5425, 1.8403 }, { 2683.8470, -2433.726, 13.5553, 178.1029 } },
	{ "Gacia Baseball Ground",	{ -2305.7549, 92.3505, 35.3516, 46.5659 }, { -2322.0908, 108.5021, 35.3984, 224.8133 } }
};

stock Float:GetDistanceBetweenPlayers(iPlayer1, iPlayer2, &Float: fDistance = Float: 0x7F800000, bool: bAllowNpc = false)
{
    static
    	Float:fX, Float:fY, Float:fZ;

    if(!bAllowNpc && (IsPlayerNPC(iPlayer1) || IsPlayerNPC(iPlayer2))) // since this command is designed for players
        return fDistance;

    if(GetPlayerVirtualWorld(iPlayer1) == GetPlayerVirtualWorld(iPlayer2) && GetPlayerPos(iPlayer2, fX, fY, fZ))
		fDistance = GetPlayerDistanceFromPoint(iPlayer1, fX, fY, fZ);

    return fDistance;
}

new
	g_WeaponList					[] = {0, 24, 25, 29, 30, 31, 34},
	p_duelInvitation           		[MAX_PLAYERS][MAX_PLAYERS];

new MySQL:gSQL; 
new KillStreak[MAX_PLAYERS];
new RecentDeaths[MAX_PLAYERS],
	RecentKills[MAX_PLAYERS];
new bool:CbugEnable[MAX_PLAYERS];
new bool:HeadshotEnable[MAX_PLAYERS];
new ptmCBugFreezeOver[MAX_PLAYERS];
new ptsLastFiredWeapon[MAX_PLAYERS];
new DmActor,
	FreeroamActor;
new ProtectionSpawn[MAX_PLAYERS];
new pDrunkLevelLast[MAX_PLAYERS];
new pFPS[MAX_PLAYERS];
new ChatTime[MAX_PLAYERS];
new CmdTime[MAX_PLAYERS];
new AdminDuty[MAX_PLAYERS];
new Godmode[MAX_PLAYERS];
new g_player_listitem[MAX_PLAYERS][50];
new g_player_name[MAX_PLAYERS][MAX_PLAYER_NAME];
new playerCar[MAX_PLAYERS];
new xTimerServer;

new Airplanes = mS_INVALID_LISTID;
new Bikes = mS_INVALID_LISTID;
new Boats = mS_INVALID_LISTID;
new Convertible = mS_INVALID_LISTID;
new Helicopters = mS_INVALID_LISTID;
new Industrials = mS_INVALID_LISTID;
new Lowrider = mS_INVALID_LISTID;
new OffRoad = mS_INVALID_LISTID;
new PublicService = mS_INVALID_LISTID;
new RC = mS_INVALID_LISTID;
new Saloon = mS_INVALID_LISTID;
new Sports = mS_INVALID_LISTID;
new StationWagon = mS_INVALID_LISTID;
new Trailer = mS_INVALID_LISTID;
new Unique = mS_INVALID_LISTID;

new ServerMessages[4][] = {
    "F:DM "COL_WHITE"-> Server version "COL_SERVER""SERVER_VERSION"",
    "F:DM "COL_WHITE"-> Share this server to your friends",
	"F:DM "COL_WHITE"-> Read {FFFF00}/rules"COL_WHITE" to avoid you from banned.",
    "F:DM "COL_WHITE"-> Report on discord if have bugs, cheater etc."
};

new Float:FreeroamSpawn[][4] =
{
    {1717.4598,-1712.9932,13.5000,175.5482},
    {1130.7339,-1489.0228,22.7690,89.7291},
    {2006.1132,1544.9379,13.4116,271.3974},
    {1087.0408,1073.1177,10.8382,309.2985},
	{-2033.3954,-98.3350,35.1641,2.6550},
	{-1499.3840,920.2671,7.1875,90.6116}
};

new Float:WarehousesSpawn[][4] =
{
    {1412.6399,-1.7875,1000.9244,95.5046},
    {1412.7356,-42.7349,1000.9214,89.5512},
    {1363.8529,-42.1017,1000.9207,270.9495},
    {1367.4900,-1.9307,1000.9219,268.4427}
};

new Float:GantonSpawn[][4] =
{
    {2473.3655,-1684.0613,13.4674,325.1316},
    {2466.7097,-1650.6208,13.4711,181.6488},
    {2505.0662,-1653.3523,13.5938,142.8000},
    {2511.2051,-1677.7815,13.5469,51.4311}
};

new Float:LVPDHQSpawn[][4] =
{
    {288.7460,169.3510,1007.1719,0.0000},
    {299.7592,191.2291,1007.1719,90.0897},
    {268.1379,185.7044,1008.1719,359.3976},
    {238.7381,195.9128,1008.1719,182.6768},
	{231.8197,143.3676,1003.0234,356.4805},
	{202.3060,168.6937,1003.0234,177.8036},
	{228.2650,182.7728,1003.0313,183.4889}
};

new Float:GhostTownSpawn[][4] =
{
    {-438.5914,2247.1299,42.4258,174.5066},
    {-348.0287,2214.0833,42.4844,97.4115},
    {-358.8611,2254.0798,42.4844,106.6161},
    {-403.8297,2224.7627,42.4258,8.6756}
};

new Float:TheBigEarSpawn[][4] =
{
    {-375.5099,1513.7614,75.5625,353.9412},
    {-343.1677,1542.1278,75.5625,181.2202},
    {-297.3112,1540.3911,75.5625,137.0181},
    {-318.7679,1549.8816,75.5625,88.0174},
	{-317.9949,1596.2235,75.7043,134.1755}
};

GetName(playerid)
{
	new name[MAX_PLAYER_NAME];
	GetPlayerName(playerid, name, sizeof(name));
	return name;
}

RandomEx(min, max)
{
	new rand = random(max-min)+min;
	return rand;
}

GetPName(playerid)
{
	GetPlayerName(playerid, pData[playerid][pName], MAX_PLAYER_NAME);
	return pData[playerid][pName];
}

SetPlayerMoney(playerid, cash)
{
	ResetPlayerMoney(playerid);
	return GivePlayerMoney(playerid, cash);
} 

GetAdminRank(playerid)
{
	new atitle[128];
	if(pData[playerid][pAdmin] == 3) { atitle = ""COL_FOUNDERS"Founders"COL_WHITE""; }
	if(pData[playerid][pAdmin] == 2) { atitle = ""COL_ADMINS"Admins"COL_WHITE""; }
	if(pData[playerid][pAdmin] == 1) { atitle = ""COL_TRIALADMIN"Trail Admins"COL_WHITE""; }
	if(pData[playerid][pAdmin] == 0) { atitle = ""COL_AMPINGZ"None"COL_WHITE""; }
	return atitle;
}

GetPlayerRank(playerid)
{
	new atitle[256];
	if (GetPlayerScore(playerid) >= 0 && GetPlayerScore(playerid) <= 50) { atitle = ""COL_AMPINGZ"Newbie"COL_WHITE""; }
	if (GetPlayerScore(playerid) >= 50 && GetPlayerScore(playerid) <= 100) { atitle = ""COL_JUNIOR"Junior"COL_WHITE""; }
	if (GetPlayerScore(playerid) >= 100 && GetPlayerScore(playerid) <= 500) { atitle = ""COL_SENIOR"Senior"COL_WHITE""; }
	if (GetPlayerScore(playerid) >= 500 && GetPlayerScore(playerid) <= 1000) { atitle = ""COL_PRO"Pro"COL_WHITE""; }
	if (GetPlayerScore(playerid) >= 1000 && GetPlayerScore(playerid) <= 2500) { atitle = ""COL_EXTENSION"Extension"COL_WHITE""; }
	if (GetPlayerScore(playerid) >= 2500 && GetPlayerScore(playerid) <= 5000) { atitle = ""COL_LEGENDARY"Legendary"COL_WHITE""; }
	if (GetPlayerScore(playerid) >= 5000 && GetPlayerScore(playerid) <= 10000) { atitle = ""COL_EXECUTIVE"Executive"COL_WHITE""; }
	if (GetPlayerScore(playerid) >= 10000 && GetPlayerScore(playerid) <= 25000) { atitle = ""COL_GENERAL"General"COL_WHITE""; }
	if (GetPlayerScore(playerid) >= 25000 && GetPlayerScore(playerid) <= 50000) { atitle = ""COL_MASTER"Master"COL_WHITE""; }
	if (GetPlayerScore(playerid) >= 50000 && GetPlayerScore(playerid) <= 100000) { atitle = ""COL_VETERAN"Veteran"COL_WHITE""; }
	if (GetPlayerScore(playerid) >= 100000) { atitle = ""COL_VETERAN"Veteran"COL_WHITE""; }
	return atitle;
}

TurnOnPlayerColor(playerid)
{
	if(AdminDuty[playerid])
	{
		if(pData[playerid][pAdmin] == 3) { SetPlayerColor(playerid, COLOR_FOUNDERS); }
		if(pData[playerid][pAdmin] == 2) { SetPlayerColor(playerid, COLOR_ADMINS); }
		if(pData[playerid][pAdmin] == 1) { SetPlayerColor(playerid, COLOR_TRIALADMIN); }
	}
	else
	{
		SetPlayerColor(playerid, COLOR_AMPINGZ);
	}
	return 1;
}

GetReportTotal(){
	new count = -1;
	
	foreach(new i : Reports){
	    count++;
	}
	return count;
}

GetAskTotal(){
	new count = -1;

	foreach(new i : Asks){
	    count++;
	}
	return count;
}

ClearBoxReport(id)
{
	if(Iter_Contains(Reports, id))
	{
		rData[id][rID] = INVALID_PLAYER_ID;
		format(rData[id][rName], MAX_PLAYER_NAME , "");
		format(rData[id][rText], 50, "");
		Iter_Remove(Reports, id);
		return 1;
	}
	return 0;
}

ClearBoxAsk(id)
{
	if(Iter_Contains(Asks, id))
	{
		aData[id][aID] = INVALID_PLAYER_ID;
		format(aData[id][aName], MAX_PLAYER_NAME , "");
		format(aData[id][aText], 50, "");
		Iter_Remove(Asks, id);
		return 1;
	}
	return 0;
}

IsPlayerHasReport(playerid)
{
	foreach(new i : Reports)
	{
	    if(rData[i][rID] == playerid)
			return 1;
	}
	return 0;
}

IsPlayerHasAsk(playerid)
{
	foreach(new i : Asks)
	{
	    if(aData[i][aID] == playerid)
			return 1;
	}
	return 0;
}

SendMessageToAdmins(const string[]) 
{
	foreach(new i:Player) 
		if(pData[i][pLoggedIn])
			if(pData[i][pAdmin] > 1) 
				AdminMessage(i, "%s", string); 
	return 1;
}

ConvertTime(time, &minutes, &seconds)
{
	seconds = time % 60;
	time = time / 60;
	minutes = time % 60;
}

//=========================================================================
main()
{
	printf(" ");
	print("-> ____________________ [S][A][E][L][Y] ____________________ <-");
	print("> Open-MP -> Flash Deathmatch");
	print("> Script Name : Open-MP Gamemode");
	print("> Script Version : "SERVER_VERSION"");
	print("> Script Coder : iTuak");
	print("> Last Update : 11/10/2024");
	print("> MySQL Version : R41-4");
	print("-> ____________________ [S][A][E][L][Y] ____________________ <-");
	printf(" ");
}

public OnGameModeInit()
{
	SetGameModeText("Flash Happiness! "SERVER_VERSION"");
	MySQL_SetupConnection();
    UsePlayerPedAnims();
	createMap();
	createActor();
	createGTextdraw();
	ServerRule();
	EnableStuntBonusForAll(false);
	loadScriptFiles();
	DisableInteriorEnterExits();
	SetTimer("RandomMessage", 300000, true);
	xTimerServer = SetTimer("TimerServer", 1000, false);
	return 1;
}

public OnGameModeExit()
{
	foreach(new i : Player)
	{
		if(pData[i][pLoggedIn]) SaveUser(i);
	}
	KillTimer(xTimerServer);
	mysql_close(gSQL);
	return 1;
}

MySQL_SetupConnection(ttl = 3)
{
	print("[MySQL] Connecting to database...");
	gSQL = mysql_connect(MYSQL_HOST, MYSQL_USER, MYSQL_PASS, MYSQL_DBSE);

	if(mysql_errno(gSQL) != 0)
	{
		if(ttl > 1)
		{
			print("[MySQL] Connect to database unsuccesful.");
			printf("[MySQL] Try Again (TTL: %d).", ttl-1);
			return MySQL_SetupConnection(ttl-1);
		}
		else
		{
			print("[MySQL] Connect to database unsuccesful.");
			print("[MySQL] Try check information login MySQL.");
			print("[MySQL] Close server");
			return SendRconCommand("exit");
		}
	}
	printf("[MySQL] Connect to database success! handle: %d", _:gSQL);
	mysql_log(ALL);
	return 1;
}

ServerRule()
{
	AddServerRule("developer", "SaeLy");
	return 1;
}

createActor()
{
	FreeroamActor = CreateActor(124, 1392.6371,-39.0108,1001.3125,88.4732);
	SetActorInvulnerable(FreeroamActor, false);
 	CreateDynamic3DTextLabel(""COL_SERVER"Shoot me or /freeroam to enter freeroam\n"COL_WHITE"Freeroam", -1, 1392.6371,-39.0108,1001.3125, 40);

	DmActor = CreateActor(2,1392.6310,-40.5821,1001.3125,88.0876);
	SetActorInvulnerable(DmActor, false);
	CreateDynamic3DTextLabel(""COL_SERVER"Shoot me or /dm to enter arena\n"COL_WHITE"Arena", -1, 1392.6310,-40.5821,1001.3125, 40);
	return 1;
}

function RandomMessage()
{
	new rand = random(sizeof(ServerMessages));
	SendClientMessageToAll(COLOR_SERVER, ServerMessages[rand]);
	return 1;
}

function loadScriptFiles()
{
	Airplanes = LoadModelSelectionMenu("vehicles/airplane.txt");
	Bikes = LoadModelSelectionMenu("vehicles/bike.txt");
	Boats = LoadModelSelectionMenu("vehicles/boat.txt");
	Convertible = LoadModelSelectionMenu("vehicles/convertible.txt");
	Helicopters = LoadModelSelectionMenu("vehicles/helicopter.txt");
	Industrials = LoadModelSelectionMenu("vehicles/industrial.txt");
	Lowrider = LoadModelSelectionMenu("vehicles/lowrider.txt");
	OffRoad = LoadModelSelectionMenu("vehicles/offRoad.txt");
	PublicService = LoadModelSelectionMenu("vehicles/publicservice.txt");
	RC = LoadModelSelectionMenu("vehicles/rc.txt");
	Saloon = LoadModelSelectionMenu("vehicles/saloon.txt");
	Sports = LoadModelSelectionMenu("vehicles/sport.txt");
	StationWagon = LoadModelSelectionMenu("vehicles/stationwagon.txt");
	Trailer = LoadModelSelectionMenu("vehicles/trailer.txt");
	Unique = LoadModelSelectionMenu("vehicles/unique.txt");
	return 1;
}

function TimerServer() 
{
	foreach(new playerid:Player)
	{
		if(pData[playerid][pLoggedIn])
		{
			pData[playerid][pSeconds]++;
			if(pData[playerid][pMuteTime] > 0)
			{
				pData[playerid][pMuteTime]--;
				if(pData[playerid][pMuteTime] == 0)
				{
					InfoMessage(playerid, "Your mute time has ended. You can type now.");
					pData[playerid][pMuted] = 0;
				}
			}
			if(pData[playerid][pSeconds] == 60)
		    {
		        pData[playerid][pMinutes]++;
		    }
		    if(pData[playerid][pMinutes] == 60)
		    {
		        pData[playerid][pHours]++;
		    }
		    SetPlayerTime(playerid, pData[playerid][pHours], pData[playerid][pMinutes]);
		}
	}
	xTimerServer = SetTimer("TimerServer", 1000, false);
	return 1;
}

//=========================================================================
public OnPlayerConnect(playerid)
{
	//new string[40];
    //GetPlayerVersion(playerid, string, sizeof(string));
    //if(strcmp(string, "0.3.7-R1") && strcmp(string, "0.3.7-R4")) KickVersion(playerid);

	ResetVariable(playerid);
	createLTextdraw(playerid);
	ClearChat(playerid, 25);
	GetPName(playerid);
	RemoveBuidingMapping(playerid);
	InfoMessage(playerid, "Server is loading. Please wait.");
	InfoMessage(playerid, "Credits: SaeLy.");
	InfoMessage(playerid, "You have 60 seconds to register before being automaticly kicked.");
	return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
	if(pData[playerid][pLoggedIn]) SaveUser(playerid);
	pData[playerid][pLoggedIn] = false;
	if(IsPlayerHasReport(playerid))
	{
	    foreach(new id : Reports)
	    {
	        if(rData[id][rID] != playerid) continue;
	        
			ClearBoxReport(id);
	    }
	}
	if(IsPlayerHasReport(playerid))
	{
	    foreach(new id : Asks)
	    {
	        if(aData[id][aID] != playerid) continue;
	        
			ClearBoxAsk(id);
	    }
	}
	forfeitPlayerDuel(playerid);
	return 1;
}

public OnPlayerRequestClass(playerid, classid)
{
	InterpolateCameraPos(playerid, 1039.624511, -1955.488037, 227.646926, 1479.973632, -2255.286376, 50.983421, 15000);
	InterpolateCameraLookAt(playerid, 1043.405029, -1958.182128, 225.789596, 1475.610107, -2252.845214, 50.968055, 1000);
	PlayerTextDrawShow(playerid, infotd[playerid]);
	if(!pData[playerid][pLoggedIn])
	{
		new query[128];
		mysql_format(gSQL, query, sizeof(query), "SELECT id FROM users WHERE name = '%e'", pData[playerid][pName]);
		mysql_pquery(gSQL, query, "OnUserCheck", "d", playerid);
	}
	return 1;
}

public OnPlayerSpawn(playerid)
{
	SpawnEx(playerid);
	for(new i = 0; i < 7; i++) PlayerTextDrawShow(playerid, StatsTD[playerid][i]);
    PlayerTextDrawHide(playerid, infotd[playerid]);
	return 1;
}

public OnPlayerDeath(playerid, killerid, WEAPON:reason)
{
	if(pData[killerid][pDmIn] >= 1)
	{
		new dstr[126];
		format(dstr, sizeof dstr, "~y~~h~You were killed by %s", GetName(killerid));
		GameTextForPlayer(playerid, dstr, 3000, 4);
		KillStreak[playerid] = 0;
		RecentDeaths[playerid]++;
		pData[playerid][pDeaths]++;
		if(KillStreak[killerid] != 20)
		{
			new cash = RandomEx(100, 300);
			pData[killerid][pMoney] += cash;
			pData[killerid][pScore] += 1;
			RecentKills[killerid]++;
		    KillStreak[killerid]++;
			pData[killerid][pKills]++;
			pData[killerid][pArenaKills]++;
			new string[126];
			format(string, sizeof string, "~r~~h~You have killed %s", GetName(playerid));
			GameTextForPlayer(killerid, string, 3000, 4);
			SetPlayerScore(killerid, pData[killerid][pScore]);
			SetPlayerMoney(killerid, pData[killerid][pMoney]);
		}
		if(KillStreak[killerid] == 3)
		{	
			new Float:a;
			GetPlayerArmour(killerid, a);
			pData[killerid][pScore] += 4;
			SetPlayerScore(killerid, pData[killerid][pScore]);
			SendClientMessageToAll(COLOR_GRAY, "[Streak] "COL_SERVER"%s "COL_WHITE"is got Killing Spree!", GetName(killerid));
			StreakMessage(killerid, "You get +25 armour and 5 score from your streak.");
			SetPlayerArmour(killerid, a + 25.0);
			GameTextForPlayer(killerid, "~r~~h~KILLING SPREE!", 3000, 4);

		}
		if(KillStreak[killerid] == 5)
		{
			new Float:a;
			GetPlayerArmour(killerid, a);
			pData[killerid][pScore] += 4;
			SetPlayerScore(killerid, pData[killerid][pScore]);
			SendClientMessageToAll(COLOR_GRAY, "[Streak] "COL_SERVER"%s "COL_WHITE"is got Maniac!", GetName(killerid));
			StreakMessage(killerid, "You get +25 armour and 5 score from your streak.");
			SetPlayerArmour(killerid, a + 25.0);
			GameTextForPlayer(killerid, "~r~~h~MANIAC!", 3000, 4);
		}
		if(KillStreak[killerid] == 10)
		{
			new Float:a;
			GetPlayerArmour(killerid, a);
			pData[killerid][pScore] += 4;
			SetPlayerScore(killerid, pData[killerid][pScore]);
			SendClientMessageToAll(COLOR_GRAY, "[Streak] "COL_SERVER"%s "COL_WHITE"is got Savage!", GetName(killerid));
			StreakMessage(killerid, "You get +25 armour and 5 score from your streak.");
			SetPlayerArmour(killerid, a + 25.0);
			GameTextForPlayer(killerid, "~r~~h~SAVAGE!", 3000, 4);
		}
		if(KillStreak[killerid] == 15)
		{
			new Float:a;
			GetPlayerArmour(killerid, a);
			pData[killerid][pScore] += 4;
			SetPlayerScore(killerid, pData[killerid][pScore]);
			SendClientMessageToAll(COLOR_GRAY, "[Streak] "COL_SERVER"%s "COL_WHITE"is got Rampage!", GetName(killerid));
			StreakMessage(killerid, "You get +25 armour and 5 score from your streak.");
			SetPlayerArmour(killerid, a + 25.0);
			GameTextForPlayer(killerid, "~r~~h~RAMPAGE!", 3000, 4);
		}
		if(KillStreak[killerid] == 20)
		{
			new Float:a;
			GetPlayerArmour(killerid, a);
			pData[killerid][pScore] += 4;
			SetPlayerScore(killerid, pData[killerid][pScore]);
			SendClientMessageToAll(COLOR_GRAY, "[Streak] "COL_SERVER"%s "COL_WHITE"is got Godlike!", GetName(killerid));
			StreakMessage(killerid, "You get +25 armour and 5 score from your streak.");
			SetPlayerArmour(killerid, a + 25.0);
			GameTextForPlayer(killerid, "~r~~h~GODLIKE!", 3000, 4);
		}
	}
	if(IsPlayerDueling(playerid))
	{
		forfeitPlayerDuel(playerid);
	}
	return 1;
}

public OnPlayerEnterVehicle(playerid, vehicleid, ispassenger)
{
	return 1;
}

public OnPlayerExitVehicle(playerid, vehicleid)
{
	return 1;
}

public OnVehicleSpawn(vehicleid)
{
	return 1;
}

public OnVehicleDeath(vehicleid, killerid)
{
	return 1;
}

function OnUserCheck(playerid)
{
	SetTimerEx("AutoKick", 60 * 1000, false, "i", playerid);
	new rows;
	cache_get_row_count(rows);
	if(rows == 0)
	{
		ShowPlayerDialog(playerid, DIALOG_REGISTER, DIALOG_STYLE_PASSWORD, ""COL_SERVER"F:DM "COL_WHITE"- Register", ""COL_WHITE"[MY] Welamat datang di "COL_SERVER"Flash Deathmatch"COL_WHITE",\nSila masukkan kata laluan untuk daftarkan account anda\n\n[EN] Welcome to "COL_SERVER"Flash Deathmatch"COL_WHITE",\nPlease enter the password to register your account", "Register", "Exit");
	}
	else
	{
		ShowPlayerDialog(playerid, DIALOG_LOGIN, DIALOG_STYLE_PASSWORD, ""COL_SERVER"F:DM "COL_WHITE"- Login", ""COL_WHITE"[MY] Welamat datang kembali di "COL_SERVER"Flash Deathmatch"COL_WHITE",\nSila masukkan kata laluan untuk masuk ke account anda\n\n[EN] Welcome back to "COL_SERVER"Flash Deathmatch"COL_WHITE",\nPlease enter the password to log in to your account", "Login", "Exit");
	}
	return 1;
}

SaveUser(playerid)
{
    if(pData[playerid][pLoggedIn])
    {
        new query[1000];
        mysql_format(gSQL, query, sizeof(query), "UPDATE `users` SET `admin` = %d, `skin` = %d, `score` = %d, `money` = %d, `onlineseconds` = %d, `onlineminutes` = %d, `onlinehours` = %d, `kills` = %d, `deaths` = %d, `headshot` = %d, `assist` = %d, `muted` = %d, `mutetime` = %d, `arenakills` = %d, `duelwins` = %d WHERE `id` = %d",
            pData[playerid][pAdmin],
            pData[playerid][pSkin],
            pData[playerid][pScore],
            pData[playerid][pMoney],
            pData[playerid][pSeconds],
            pData[playerid][pMinutes],
            pData[playerid][pHours],
            pData[playerid][pKills],
            pData[playerid][pDeaths],
            pData[playerid][pHeadshot],
            pData[playerid][pAssist],
            pData[playerid][pMuted],
            pData[playerid][pMuteTime],
            pData[playerid][pArenaKills],
            pData[playerid][pDuelWins],
            pData[playerid][pID]
        );

        mysql_tquery(gSQL, query);
        
        printf("[MySQL] Saved %s's data", pData[playerid][pName]);
    }
    
    return 1;
}

function ResetVariable(playerid)
{
	pData[playerid][pID]       					 = 0;
	pData[playerid][pAdmin]    					 = 0;
	pData[playerid][pSkin]    					 = 25;
	pData[playerid][pScore]						 = 0;
	pData[playerid][pMoney]						 = 1000;
	pData[playerid][pSeconds]    				 = 0;
	pData[playerid][pMinutes]					 = 0;
	pData[playerid][pHours]						 = 0;
	pData[playerid][pKills]    					 = 0;
	pData[playerid][pDeaths]   					 = 0;
	pData[playerid][pHeadshot]					 = 0;
	pData[playerid][pAssist]				     = 0;
	pData[playerid][pMuted]						 = 0;
	pData[playerid][pMuteTime]					 = 0;
	pData[playerid][pArenaKills]				 = 0;
	pData[playerid][pDuelWins]					 = 0;

	//==================================================[ Not Save ]
	pData[playerid][pLoggedIn]					 = false;
	pData[playerid][pLobbyIn]					 = false;
	pData[playerid][pDmIn]						 = false;
	pData[playerid][pFreeroamIn]   				 = false;
	pData[playerid][pDuelIn]     				 = false;
	KillStreak[playerid]						 = 0;
	RecentDeaths[playerid]						 = 0;
	RecentKills[playerid]						 = 0;
	CbugEnable[playerid] 						 = false;
	HeadshotEnable[playerid] 					 = false;
	KillTimer(ptmCBugFreezeOver[playerid]);
	ProtectionSpawn[playerid]   				 = false;
	ptsLastFiredWeapon[playerid]				 = 0;
	pDrunkLevelLast[playerid]   				 = 0;
    pFPS[playerid]          					 = 0;
	InfoPlayer[playerid] 					     = false;
	ChatTime[playerid]    						 = 0;
	CmdTime[playerid]    						 = 0;
	AdminDuty[playerid]    						 = 0;
	Godmode[playerid]    						 = 0;
	playerCar[playerid] 						 = INVALID_VEHICLE_ID;
	DestroyVehicle(playerCar[playerid]);
	g_duelData[playerid][E_PLAYER] 				 = INVALID_PLAYER_ID;
	g_duelData[playerid][E_WEAPON][0] 			 = 0;
	g_duelData[playerid][E_WEAPON][1] 			 = 0;
	g_duelData[playerid][E_HEALTH] 				 = 100.0;
	g_duelData[playerid][E_ARMOUR] 				 = 100.0;
	return 1;
}

function SpawnEx(playerid)
{
	if(pData[playerid][pLobbyIn])
	{
		SetPlayerArmour(playerid, 0);
		SetPlayerHealth(playerid, 100);
		SetPlayerSkin(playerid, pData[playerid][pSkin]);
		SetPlayerMoney(playerid, pData[playerid][pMoney]);
		SetPlayerScore(playerid, pData[playerid][pScore]);
		SetCameraBehindPlayer(playerid);
		ResetPlayerWeapons(playerid);
		SetPlayerInterior(playerid, 0);
		SetPlayerVirtualWorld(playerid, 0);
		SetPlayerPos(playerid, 1387.3857,-33.0188,1000.9553);
		SetPlayerFacingAngle(playerid, 181.8596);
		TogglePlayerControllable(playerid, false);
		GivePlayerWeapon(playerid, WEAPON_DEAGLE, 999999);
		SetTimerEx("WaitSpawn", 5000, false, "i", playerid);
		TurnOnPlayerColor(playerid);
		ShowInfoForPlayer(playerid, 5000, "~r~#FlashDM ~w~- You are entering Lobby!");
	}
	if(pData[playerid][pFreeroamIn])
	{
		new rand = random(sizeof(FreeroamSpawn));
		SetPlayerArmour(playerid, 0);
		SetPlayerHealth(playerid, 100);
		SetPlayerSkin(playerid, pData[playerid][pSkin]);
		SetPlayerMoney(playerid, pData[playerid][pMoney]);
		SetPlayerScore(playerid, pData[playerid][pScore]);
		SetCameraBehindPlayer(playerid);
		ResetPlayerWeapons(playerid);
		SetPlayerInterior(playerid, 0);
		SetPlayerVirtualWorld(playerid, 10);
		SetPlayerPos(playerid, FreeroamSpawn[rand][0], FreeroamSpawn[rand][1],FreeroamSpawn[rand][2]);
		SetPlayerFacingAngle(playerid, FreeroamSpawn[rand][3]);
		ProtectionSpawn[playerid] = true;
		TogglePlayerControllable(playerid, false);
		SetTimerEx("SpawnProtection", 5000, false, "i", playerid);
		TurnOnPlayerColor(playerid);
		ShowInfoForPlayer(playerid, 5000, "~r~#FlashDM ~w~- You are entering Freeroam!");
	}
	if(pData[playerid][pDmIn] == 1)
	{
		new rand = random(sizeof(LVPDHQSpawn));
		SetPlayerArmour(playerid, 0);
		SetPlayerHealth(playerid, 100);
		SetPlayerSkin(playerid, pData[playerid][pSkin]);
		SetPlayerMoney(playerid, pData[playerid][pMoney]);
		SetPlayerScore(playerid, pData[playerid][pScore]);
		SetCameraBehindPlayer(playerid);
		ResetPlayerWeapons(playerid);
		SetPlayerInterior(playerid, 3);
		SetPlayerVirtualWorld(playerid, 11);
		SetPlayerPos(playerid, LVPDHQSpawn[rand][0], LVPDHQSpawn[rand][1],LVPDHQSpawn[rand][2]);
		SetPlayerFacingAngle(playerid, LVPDHQSpawn[rand][3]);
		GivePlayerWeapon(playerid, WEAPON_SHOTGUN, 999999);
		GivePlayerWeapon(playerid, WEAPON_DEAGLE, 999999);
		ProtectionSpawn[playerid] = true;
		TogglePlayerControllable(playerid, false);
		SetTimerEx("SpawnProtection", 5000, false, "i", playerid);
		TurnOnPlayerColor(playerid);
		ShowInfoForPlayer(playerid, 5000, "~r~#FlashDM ~w~- You are entering Arena!");
	}
	if(pData[playerid][pDmIn] == 2)
	{
		new rand = random(sizeof(LVPDHQSpawn));
		SetPlayerArmour(playerid, 0);
		SetPlayerHealth(playerid, 100);
		SetPlayerSkin(playerid, pData[playerid][pSkin]);
		SetPlayerMoney(playerid, pData[playerid][pMoney]);
		SetPlayerScore(playerid, pData[playerid][pScore]);
		SetPlayerPos(playerid, LVPDHQSpawn[rand][0], LVPDHQSpawn[rand][1],LVPDHQSpawn[rand][2]);
		SetPlayerFacingAngle(playerid, LVPDHQSpawn[rand][3]);
		SetCameraBehindPlayer(playerid);
		SetPlayerInterior(playerid, 3);
		SetPlayerVirtualWorld(playerid, 12);
		ResetPlayerWeapons(playerid);
		GivePlayerWeapon(playerid, WEAPON_SHOTGUN, 999999);
		GivePlayerWeapon(playerid, WEAPON_DEAGLE, 999999);
		ProtectionSpawn[playerid] = true;
		TogglePlayerControllable(playerid, false);
		SetTimerEx("SpawnProtection", 5000, false, "i", playerid);
		TurnOnPlayerColor(playerid);
		ShowInfoForPlayer(playerid, 5000, "~r~#FlashDM ~w~- You are entering Arena!");
	}
	if(pData[playerid][pDmIn] == 3)
	{
		new rand = random(sizeof(GhostTownSpawn));
		SetPlayerHealth(playerid, 100);
		SetPlayerArmour(playerid, 0);
		SetPlayerSkin(playerid, pData[playerid][pSkin]);
		SetPlayerMoney(playerid, pData[playerid][pMoney]);
		SetPlayerScore(playerid, pData[playerid][pScore]);
		SetPlayerPos(playerid, GhostTownSpawn[rand][0], GhostTownSpawn[rand][1],GhostTownSpawn[rand][2]);
		SetPlayerFacingAngle(playerid, GhostTownSpawn[rand][3]);
		SetCameraBehindPlayer(playerid);
		SetPlayerInterior(playerid, 0);
		SetPlayerVirtualWorld(playerid, 13);
		ResetPlayerWeapons(playerid);
		GivePlayerWeapon(playerid, WEAPON_SHOTGUN, 999999);
		GivePlayerWeapon(playerid, WEAPON_DEAGLE, 999999);
		ProtectionSpawn[playerid] = true;
		TogglePlayerControllable(playerid, false);
		SetTimerEx("SpawnProtection", 5000, false, "i", playerid);
		TurnOnPlayerColor(playerid);
		ShowInfoForPlayer(playerid, 5000, "~r~#FlashDM ~w~- You are entering Arena!");
	}
	if(pData[playerid][pDmIn] == 4)
	{
		new rand = random(sizeof(GhostTownSpawn));
		SetPlayerArmour(playerid, 0);
		SetPlayerHealth(playerid, 100);
		SetPlayerSkin(playerid, pData[playerid][pSkin]);
		SetPlayerMoney(playerid, pData[playerid][pMoney]);
		SetPlayerScore(playerid, pData[playerid][pScore]);
		SetCameraBehindPlayer(playerid);
		ResetPlayerWeapons(playerid);
		SetPlayerInterior(playerid, 0);
		SetPlayerVirtualWorld(playerid, 14);
		GivePlayerWeapon(playerid, WEAPON_SHOTGUN, 999999);
		GivePlayerWeapon(playerid, WEAPON_DEAGLE, 999999);
		SetPlayerPos(playerid, GhostTownSpawn[rand][0], GhostTownSpawn[rand][1],GhostTownSpawn[rand][2]);
		SetPlayerFacingAngle(playerid, GhostTownSpawn[rand][3]);
		ProtectionSpawn[playerid] = true;
		TogglePlayerControllable(playerid, false);
		SetTimerEx("SpawnProtection", 5000, false, "i", playerid);
		TurnOnPlayerColor(playerid);
		ShowInfoForPlayer(playerid, 5000, "~r~#FlashDM ~w~- You are entering Arena!");
	}
	if(pData[playerid][pDmIn] == 5)
	{
		new rand = random(sizeof(WarehousesSpawn));
		SetPlayerArmour(playerid, 0);
		SetPlayerHealth(playerid, 100);
		SetPlayerSkin(playerid, pData[playerid][pSkin]);
		SetPlayerMoney(playerid, pData[playerid][pMoney]);
		SetPlayerScore(playerid, pData[playerid][pScore]);
		SetPlayerPos(playerid, WarehousesSpawn[rand][0], WarehousesSpawn[rand][1], WarehousesSpawn[rand][2]);
		SetPlayerFacingAngle(playerid, WarehousesSpawn[rand][3]);
		SetCameraBehindPlayer(playerid);
		SetPlayerInterior(playerid, 1);
		SetPlayerVirtualWorld(playerid, 15);
		ResetPlayerWeapons(playerid);
		GivePlayerWeapon(playerid, WEAPON_DEAGLE, 999999);
		ProtectionSpawn[playerid] = true;
		TogglePlayerControllable(playerid, false);
		SetTimerEx("SpawnProtection", 5000, false, "i", playerid);
		TurnOnPlayerColor(playerid);
		ShowInfoForPlayer(playerid, 5000, "~r~#FlashDM ~w~- You are entering Arena!");
	}
	if(pData[playerid][pDmIn] == 6)
	{
		new rand = random(sizeof(TheBigEarSpawn));
		SetPlayerHealth(playerid, 100);
		SetPlayerArmour(playerid, 0);
		SetPlayerSkin(playerid, pData[playerid][pSkin]);
		SetPlayerMoney(playerid, pData[playerid][pMoney]);
		SetPlayerScore(playerid, pData[playerid][pScore]);
		SetPlayerPos(playerid, TheBigEarSpawn[rand][0], TheBigEarSpawn[rand][1], TheBigEarSpawn[rand][2]);
		SetPlayerFacingAngle(playerid, TheBigEarSpawn[rand][3]);
		SetCameraBehindPlayer(playerid);
		SetPlayerInterior(playerid, 0);
		SetPlayerVirtualWorld(playerid, 16);
		ResetPlayerWeapons(playerid);
		GivePlayerWeapon(playerid, WEAPON_DEAGLE, 999999);
		ProtectionSpawn[playerid] = true;
		TogglePlayerControllable(playerid, false);
		SetTimerEx("SpawnProtection", 5000, false, "i", playerid);
		TurnOnPlayerColor(playerid);
		ShowInfoForPlayer(playerid, 5000, "~r~#FlashDM ~w~- You are entering Arena!");
	}
	if(pData[playerid][pDmIn] == 7)
	{
		new rand = random(sizeof(GantonSpawn));
		SetPlayerHealth(playerid, 100);
		SetPlayerArmour(playerid, 0);
		SetPlayerSkin(playerid, pData[playerid][pSkin]);
		SetPlayerMoney(playerid, pData[playerid][pMoney]);
		SetPlayerScore(playerid, pData[playerid][pScore]);
		SetPlayerPos(playerid, GantonSpawn[rand][0], GantonSpawn[rand][1], GantonSpawn[rand][2]);
		SetPlayerFacingAngle(playerid, GantonSpawn[rand][3]);
		SetCameraBehindPlayer(playerid);
		SetPlayerInterior(playerid, 0);
		SetPlayerVirtualWorld(playerid, 17);
		ResetPlayerWeapons(playerid);
		GivePlayerWeapon(playerid, WEAPON_DEAGLE, 999999);
		ProtectionSpawn[playerid] = true;
		TogglePlayerControllable(playerid, false);
		SetTimerEx("SpawnProtection", 5000, false, "i", playerid);
		TurnOnPlayerColor(playerid);
		ShowInfoForPlayer(playerid, 5000, "~r~#FlashDM ~w~- You are entering Arena!");
	}
	return 1;
}

function WaitSpawn(playerid)
{
	TogglePlayerControllable(playerid, true);
	return 1;
}

function SpawnProtection(playerid)
{
	ProtectionSpawn[playerid] = false;
	TogglePlayerControllable(playerid, true);
	return 1;
}

ShowInfoForPlayer(playerid, time, const info[])
{
	KillTimer(TimerInfo[playerid]);
	PlayerTextDrawShow(playerid, infotd[playerid]);
	PlayerTextDrawSetString(playerid, infotd[playerid], info);
	InfoPlayer[playerid] = true;
	TimerInfo[playerid] = SetTimerEx("HideInfoForPlayer", time, false, "i", playerid);
}

function HideInfoForPlayer(playerid)
{
    if(!pData[playerid][pLoggedIn]) return 1;
    PlayerTextDrawHide(playerid, infotd[playerid]);
    InfoPlayer[playerid] = false;
    return 1;
}

function OnPlayerAssist(playerid, deathid, killerid)
{
	new cash = RandomEx(100, 300);
	pData[playerid][pMoney] += cash;
    SetPlayerMoney(playerid, pData[playerid][pMoney]);
	pData[playerid][pScore] += 1;
	SetPlayerScore(playerid, pData[playerid][pScore]);
	pData[playerid][pAssist]++;
	new string[126];
	format(string, sizeof string, "~g~~h~You have assist %s", GetName(deathid));
	GameTextForPlayer(playerid, string, 3000, 4);
    return 1;
}

function KickVersion(playerid)
{
	new string[256];
	new str[40];
    GetPlayerVersion(playerid, str, sizeof(str));
	format(string, sizeof(string), ""COL_WHITE"You has been kicked by Server, Please use "COL_SERVER"0.3.7 R3"COL_WHITE", "COL_SERVER"0.3.7 R5"COL_WHITE" and "COL_SERVER"0.3.DL"COL_WHITE"\n\nYou Client: %s", str);
	ShowPlayerDialog(playerid, DIALOG_UNUSED, DIALOG_STYLE_MSGBOX, ""COL_SERVER"F:DM "COL_WHITE"- Kick", string, "Okay", "Exit");
	SetTimerEx("KickPlayer", 3000, true, "%d", playerid);
	return 1;
}

function KickPlayer(playerid)
{
	ClearChat(playerid, 25);
	Kick(playerid);
	return 0;
}

SetPlayerSpawnAfterDuel(playerid)
{
	if(IsPlayerDueling(playerid))
	{
		// teleport back to lobby
		pData[playerid][pDuelIn] = false;
    	pData[playerid][pLobbyIn] = true;
		SpawnEx(playerid);

		// reset duel variables
		g_duelData[playerid][E_PLAYER] 	= INVALID_PLAYER_ID;
		return 1;
	}
	return 1;
}


static stock ClearDuelInvites(playerid)
{
	new current_time = gettime();
	new count = 0;
	for(new i = 0; i < MAX_PLAYERS; i ++)
	{
		if(p_duelInvitation[playerid][i] != 0 && current_time > p_duelInvitation[playerid][i])
		{
			p_duelInvitation[playerid][i] = 0;
			count ++;
		}
	}
	return count;
}

IsPlayerDueling(playerid) {
	return pData[playerid][pDuelIn];
}

ShowPlayerDuelMenu(playerid)
{
	new string[500];
	format(string,sizeof(string),
	"Setting\tOption\n\
	"COL_WHITE"Location:\t"COL_SERVER"%s\n\
	"COL_WHITE"Enemies:\t"COL_SERVER"%s\n\
	"COL_WHITE"Health:\t"COL_SERVER"%.2f%%\n\
	"COL_WHITE"Armour:\t"COL_SERVER"%.2f%%\n\
	"COL_WHITE"Primary Weapon:\t"COL_SERVER"%s\n\
	"COL_WHITE"Secondary Weapon:\t"COL_SERVER"%s\n\
	"COL_WHITE"Reset duel\t"COL_SERVER"[Enter] ->\n\
	"COL_WHITE"Send Duel\t"COL_SERVER"[Enter] ->",
	g_duelLocationData[g_duelData[playerid][E_LOCATION_ID]][E_NAME],
	(!IsPlayerConnected(g_duelData[playerid][E_PLAYER]) ? (""COL_WHITE"None") : (GetName(g_duelData[playerid][E_PLAYER]))),
	g_duelData[playerid][E_HEALTH],
	g_duelData[playerid][E_ARMOUR],
	ReturnWeaponName(WEAPON:g_duelData[playerid][E_WEAPON][0]),
	ReturnWeaponName(WEAPON:g_duelData[playerid][E_WEAPON][1]));
	ShowPlayerDialog(playerid, DIALOG_DUEL, DIALOG_STYLE_TABLIST_HEADERS, ""COL_SERVER"F:DM "COL_WHITE"- Duel Setup", string, "Select", "Cancel");
	return 1;
}

function OnDuelTimer(targetid)
{
	new
		playerid = g_duelData[targetid][E_PLAYER];

	g_duelData[targetid][E_COUNTDOWN] --;
	if(g_duelData[targetid][E_COUNTDOWN] <= 0)
	{
		GameTextForPlayer(targetid, "~g~~h~FIGHT!", 1500, 4);
		GameTextForPlayer(playerid, "~g~~h~FIGHT!", 1500, 4);

		PlayerPlaySound(targetid, 1057, 0.0, 0.0, 0.0);
		PlayerPlaySound(playerid, 1057, 0.0, 0.0, 0.0);

		TogglePlayerControllable(playerid, true);
		TogglePlayerControllable(targetid, true);

		KillTimer(g_duelData[targetid][E_TIMER]);
	}
	else
	{
		format(szSmallString, sizeof(szSmallString), "~w~%d", g_duelData[targetid][E_COUNTDOWN]);
		GameTextForPlayer(targetid, szSmallString, 1500, 4 );
		GameTextForPlayer(playerid, szSmallString, 1500, 4 );

		PlayerPlaySound(targetid, 1056, 0.0, 0.0, 0.0);
		PlayerPlaySound(playerid, 1056, 0.0, 0.0, 0.0);
	}
	return 1;
}

forfeitPlayerDuel(playerid)
{
	if(!IsPlayerDueling(playerid))
		return 0;

	ClearDuelInvites(playerid);

	new
		winnerid = g_duelData[playerid][E_PLAYER];

	if(!IsPlayerConnected(winnerid) || ! IsPlayerDueling(winnerid))
		return 0;

	//SpawnPlayer( winnerid );
	SetPlayerSpawnAfterDuel(winnerid);
	SetPlayerSpawnAfterDuel(playerid);
	ClearDuelInvites(winnerid);

	pData[playerid][pMoney] -= 1000;
	pData[winnerid][pMoney] += 1000;
	pData[winnerid][pScore] += 3;
	pData[winnerid][pDuelWins]++;

	SetPlayerScore(winnerid, pData[winnerid][pScore]);
	SetPlayerMoney(winnerid, pData[winnerid][pMoney]);
	SetPlayerMoney(playerid, pData[playerid][pMoney]);

    new string[128];
    format(string, sizeof(string), "Duel: "COL_WHITE"%s has won the duel against "COL_DUEL"%s.", GetName(winnerid), GetName(playerid));
	SendClientMessageToAll(COLOR_DUEL, string);
	return 1;
}

//=========================================================================
public OnPlayerRequestSpawn(playerid)
{
	if(!pData[playerid][pLoggedIn])
	{
		ErrorMessage(playerid, "You must login first!");
		return 0;
	}
	return 1;
}

public OnPlayerCommandText(playerid, cmdtext[])
{
	return 0;
}

public OnPlayerText(playerid, text[])
{
	if(!pData[playerid][pLoggedIn])
	{
		ErrorMessage(playerid, "You must login to start chats.");
		return 0;
	}
	if(pData[playerid][pMuted] == 1)
    {

		ErrorMessage(playerid, "You are muted, %s seconds left.",  gettime(pData[playerid][pMuteTime]));
		return 0;
    }
	if(ChatTime[playerid] > gettime())
	{
		ErrorMessage(playerid, "Calm down, don't spamming");
		return 0;
	}

	if(pData[playerid][pAdmin] > 3) ChatTime[playerid] = gettime()+0;
	else if(pData[playerid][pAdmin] > 1) ChatTime[playerid] = gettime()+1;
	else ChatTime[playerid] = gettime()+5;

	new str[100];
	if(pData[playerid][pAdmin] > 1)
	{
		str = ""COL_WHITE"["COL_GREEN"A"COL_WHITE"]";
	}
	else
	{
		str = "";
	}

	for(new i = 0; i < MAX_PLAYERS; i++)
    {
		if(pData[i][pLobbyIn])
		{
			SendClientMessage(i, COLOR_WHITE, "[L]%s[%s] %s [%d]: %s", str, GetPlayerRank(playerid), GetName(playerid), playerid, text);
		}
		else if(pData[i][pFreeroamIn])
		{
			SendClientMessage(i, COLOR_WHITE, "[L]%s[%s] %s [%d]: %s", str, GetPlayerRank(playerid), GetName(playerid), playerid, text);
		}
		else if(pData[i][pDuelIn])
		{
			SendClientMessage(i, COLOR_WHITE, "[L]%s[%s] %s [%d]: %s", str, GetPlayerRank(playerid), GetName(playerid), playerid, text);
		}
		else if(pData[i][pDmIn] == 1)
		{
			SendClientMessage(i, COLOR_WHITE, "[L]%s[%s] %s [%d]: %s", str, GetPlayerRank(playerid), GetName(playerid), playerid, text);
		}
		else if(pData[i][pDmIn] == 2)
		{
			SendClientMessage(i, COLOR_WHITE, "[L]%s[%s] %s [%d]: %s", str, GetPlayerRank(playerid), GetName(playerid), playerid, text);
		}
		else if(pData[i][pDmIn] == 3)
		{
			SendClientMessage(i, COLOR_WHITE, "[L]%s[%s] %s [%d]: %s", str, GetPlayerRank(playerid), GetName(playerid), playerid, text);
		}
		else if(pData[i][pDmIn] == 4)
		{
			SendClientMessage(i, COLOR_WHITE, "[L]%s[%s] %s [%d]: %s", str, GetPlayerRank(playerid), GetName(playerid), playerid, text);
		}
		else if(pData[i][pDmIn] == 5)
		{
			SendClientMessage(i, COLOR_WHITE, "[L]%s[%s] %s [%d]: %s", str, GetPlayerRank(playerid), GetName(playerid), playerid, text);
		}
		else if(pData[i][pDmIn] == 6)
		{
			SendClientMessage(i, COLOR_WHITE, "[L]%s[%s] %s [%d]: %s", str, GetPlayerRank(playerid), GetName(playerid), playerid, text);
		}
		else if(pData[i][pDmIn] == 7)
		{
			SendClientMessage(i, COLOR_WHITE, "[L]%s[%s] %s [%d]: %s", str, GetPlayerRank(playerid), GetName(playerid), playerid, text);
		}
	}
	return 0;
}

public OnPlayerUpdate(playerid)
{
	new drunknew;
    drunknew = GetPlayerDrunkLevel(playerid);
    if (drunknew < 100) { 
        SetPlayerDrunkLevel(playerid, 2000);
    } else {
        if (pDrunkLevelLast[playerid] != drunknew) {
            new wfps = pDrunkLevelLast[playerid] - drunknew;
            if ((wfps > 0) && (wfps < 200))
                pFPS[playerid] = wfps;
            pDrunkLevelLast[playerid] = drunknew;
        }
        
    }

	//===============================================================
	new string[256];
    format(string, sizeof string, "~w~FPS: %d ~n~PING: %d ~n~PL: %.2f%", pFPS[playerid], GetPlayerPing(playerid), NetStats_PacketLossPercent(playerid));
	PlayerTextDrawSetString(playerid, StatsTD[playerid][0], string);

	new string2[256];
	if(RecentDeaths[playerid] > 0)
	{
		format(string2, sizeof string2, "~w~KILLS: %d ~n~DEATHS: %d ~n~RATIO: %0.1f", RecentKills[playerid], RecentDeaths[playerid], floatdiv(RecentKills[playerid], RecentDeaths[playerid]));
		PlayerTextDrawSetString(playerid, StatsTD[playerid][1], string2);
	}
	else
	{
		format(string2, sizeof string2, "~w~KILLS: %d ~n~DEATHS: %d ~n~RATIO: 0.0", RecentKills[playerid], RecentDeaths[playerid]);
		PlayerTextDrawSetString(playerid, StatsTD[playerid][1], string2);
	}

	GetPlayerName(playerid, g_player_name[playerid], MAX_PLAYER_NAME);

	new fucktime[256],year,month,day,hours,minutes,seconds;
    getdate(year, month, day), gettime(hours, minutes, seconds);
    format(fucktime, sizeof fucktime, "%d.%s%d.%s%d", day, ((month < 10) ? ("0") : ("")), month, (year < 10) ? ("0") : (""), year);
    PlayerTextDrawSetString(playerid, StatsTD[playerid][6], fucktime);
    format(fucktime, sizeof fucktime, "%s%d:%s%d:%s%d", (hours < 10) ? ("0") : (""), hours, (minutes < 10) ? ("0") : (""), minutes, (seconds < 10) ? ("0") : (""), seconds);
    PlayerTextDrawSetString(playerid, StatsTD[playerid][5], fucktime);
	return 1;
}

public OnPlayerKeyStateChange(playerid, KEY:newkeys, KEY:oldkeys)
{
	if(!CbugEnable[playerid] && GetPlayerState(playerid) == PLAYER_STATE_ONFOOT)
	{
		if(PRESSED(KEY_FIRE))
		{
			switch(GetPlayerWeapon(playerid))
			{
				case WEAPON_DEAGLE, WEAPON_SHOTGUN, WEAPON_SNIPER:
				{
					ptsLastFiredWeapon[playerid] = gettime();
				}
			}
		}
		else if(PRESSED(KEY_CROUCH))
		{
			if((gettime() - ptsLastFiredWeapon[playerid]) < 1)
			{
				TogglePlayerControllable(playerid, false);
				CbugEnable[playerid] = true;
				GameTextForPlayer(playerid, "~w~~h~DON'T ~r~C-BUG!", 3000, 4);
				KillTimer(ptmCBugFreezeOver[playerid]);
				ptmCBugFreezeOver[playerid] = SetTimerEx("CBugFreezeOver", 1500, false, "i", playerid);
			}
		}
	}
	return 1;
}

function CBugFreezeOver(playerid)
{
	TogglePlayerControllable(playerid, true);
	CbugEnable[playerid] = false;
	return 1;
}

public OnPlayerStateChange(playerid, PLAYER_STATE:newstate, PLAYER_STATE:oldstate)
{
	return 1;
}

public OnPlayerEnterCheckpoint(playerid)
{
	return 1;
}

public OnPlayerLeaveCheckpoint(playerid)
{
	return 1;
}

public OnPlayerEnterRaceCheckpoint(playerid)
{
	return 1;
}

public OnPlayerLeaveRaceCheckpoint(playerid)
{
	return 1;
}

public OnPlayerGiveDamageActor(playerid, damaged_actorid, Float:amount, WEAPON:weaponid, bodypart)
{
	if(damaged_actorid == DmActor)
	{
		callcmd::dm(playerid);
		return 1;
	}
	if(damaged_actorid == FreeroamActor)
	{
		callcmd::freeroam(playerid); 
		return 1;
	}
	return 1;
}

public OnActorStreamIn(actorid, forplayerid)
{
	return 1;
}

public OnActorStreamOut(actorid, forplayerid)
{
	return 1;
}

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
	if(dialogid == DIALOG_REGISTER)
	{
		if(!response) return Kick(playerid);
		if(strlen(inputtext) < 4 || strlen(inputtext) > 24)
		{
			ShowPlayerDialog(playerid, DIALOG_REGISTER, DIALOG_STYLE_PASSWORD, ""COL_SERVER"F:DM "COL_WHITE"- Register", ""COL_WHITE"[MY] Welamat datang di "COL_SERVER"Flash Deathmatch"COL_WHITE",\nSila masukkan kata laluan untuk daftarkan account anda\n\n[EN] Welcome to "COL_SERVER"Flash Deathmatch"COL_WHITE",\nPlease enter the password to register your account", "Register", "Exit");
            ErrorMessage(playerid, "Your password must more than 4 words and no more than 24 words");
            return 1;
		}

		new query[256];
		mysql_format(gSQL, query, sizeof(query), "INSERT INTO users (name, password) VALUES ('%e', MD5('%e'))", pData[playerid][pName], inputtext);
		mysql_pquery(gSQL, query, "OnUserRegister", "d", playerid);
		return 1;
	}
	if(dialogid == DIALOG_LOGIN)
	{
		if(strlen(inputtext) < 4 || strlen(inputtext) > 24)
		{
			ShowPlayerDialog(playerid, DIALOG_LOGIN, DIALOG_STYLE_PASSWORD, ""COL_SERVER"F:DM "COL_WHITE"- Login", ""COL_WHITE"[MY] Welamat datang kembali di "COL_SERVER"Flash Deathmatch"COL_WHITE",\nSila masukkan kata laluan untuk masuk ke account anda\n\n[EN] Welcome back to "COL_SERVER"Flash Deathmatch"COL_WHITE",\nPlease enter the password to log in to your account", "Login", "Exit");
            ErrorMessage(playerid, "Your password must more than 4 words and no more than 24 words");
            return 1;
		}

		new query[256];
		mysql_format(gSQL, query, sizeof(query), "SELECT * FROM users WHERE name = '%e' AND password = MD5('%e')", pData[playerid][pName], inputtext);
		mysql_pquery(gSQL, query, "OnUserLogin", "d", playerid);
		return 1;
	}
	if(dialogid == DIALOG_DEATHMATCH)
	{
		if(response)
		{
            if(listitem == 0)
            {
				if(IsPlayerUsingSampMobile(playerid)) 
					return ErrorMessage(playerid, "This room only for pc users.");
					
				pData[playerid][pDmIn] = 1;
				pData[playerid][pLobbyIn] = false;
				SpawnEx(playerid);
	        }
            if(listitem == 1)
            {
				if(!IsPlayerUsingSampMobile(playerid)) 
					return ErrorMessage(playerid, "This room only for android users.");

				pData[playerid][pDmIn] = 2;
				pData[playerid][pLobbyIn] = false;
				SpawnEx(playerid);
	        }
            if(listitem == 2)
            {
				if(IsPlayerUsingSampMobile(playerid)) 
					return ErrorMessage(playerid, "This room only for pc users.");

				pData[playerid][pDmIn] = 3;
				pData[playerid][pLobbyIn] = false;
				SpawnEx(playerid);
	        }
			if(listitem == 3)
            {
				if(!IsPlayerUsingSampMobile(playerid)) 
					return ErrorMessage(playerid, "This room only for android users.");

				pData[playerid][pDmIn] = 4;
				pData[playerid][pLobbyIn] = false;
				SpawnEx(playerid);
	        }
            if(listitem == 4)
            {
				pData[playerid][pDmIn] = 5;
				pData[playerid][pLobbyIn] = false;
				CbugEnable[playerid] = true;
				SpawnEx(playerid);
	        }
            if(listitem == 5)
            {
				pData[playerid][pDmIn] = 6;
				pData[playerid][pLobbyIn] = false;
				CbugEnable[playerid] = true;
				SpawnEx(playerid);
	        }
			if(listitem == 6)
            {
				pData[playerid][pDmIn] = 7;
				pData[playerid][pLobbyIn] = false;
				HeadshotEnable[playerid] = true;
				SpawnEx(playerid);
	        }
	    }
    }
	if(dialogid == DIALOG_HELP)
	{
		if(response)
		{
			if(listitem == 0)
			{
				new string[2000];
				format(string,sizeof(string),
				"Commands\tDescription\n\
				"COL_WHITE"/settings\t"COL_SERVER"To setting your account.\n\
				"COL_WHITE"/changepassword\t"COL_SERVER"To your password.");
				ShowPlayerDialog(playerid, DIALOG_UNUSED, DIALOG_STYLE_TABLIST_HEADERS, ""COL_SERVER"F:DM "COL_WHITE"- Account Commands", string, "Okay", "Close");	
			}
			if(listitem == 1)
			{
				new string[2000];
				format(string,sizeof(string),
				"Commands\tDescription\n\
				"COL_WHITE"/dm\t"COL_SERVER"To enter arena.\n\
				"COL_WHITE"/duel\t"COL_SERVER"To duel with another players.\n\
				"COL_WHITE"/freeroam\t"COL_SERVER"To enter freeroam.");
				ShowPlayerDialog(playerid, DIALOG_UNUSED, DIALOG_STYLE_TABLIST_HEADERS, ""COL_SERVER"F:DM "COL_WHITE"- Mode Commands", string, "Okay", "Close");	
			}
			if(listitem == 2)
			{
				new string[2000];
				format(string,sizeof(string),
				"Commands\tDescription\n\
				"COL_WHITE"/rules\t"COL_SERVER"Read server rules to avoid being banned.\n\
				"COL_WHITE"/checkdevice\t"COL_SERVER"To check player device.\n\
				"COL_WHITE"/checkfps\t"COL_SERVER"To check player fps.\n\
				"COL_WHITE"/checkping\t"COL_SERVER"To check player ping.\n\
				"COL_WHITE"/checkpl\t"COL_SERVER"To check player packetloss.\n\
				"COL_WHITE"/admins\t"COL_SERVER"To check online admins.\n\
				"COL_WHITE"/stats\t"COL_SERVER"To check player stats.\n\
				"COL_WHITE"/pm\t"COL_SERVER"To send private message to other player.\n\
				"COL_WHITE"/globalchat (/g)\t"COL_SERVER"To start global chats.\n\
				"COL_WHITE"/pay\t"COL_SERVER"To pay money to another players.\n\
				"COL_WHITE"/animlist\t"COL_SERVER"To use animation.\n\
				"COL_WHITE"/stopanim\t"COL_SERVER"To stop player anim.\n\
				"COL_WHITE"/ask\t"COL_SERVER"To ask question about servers.\n\
				"COL_WHITE"/cancelask\t"COL_SERVER"To cancel your ask.\n\
				"COL_WHITE"/report\t"COL_SERVER"To report bugs/players.\n\
				"COL_WHITE"/cancelreport\t"COL_SERVER"To cancel your report.\n\
				"COL_WHITE"/ranks\t"COL_SERVER"Server available ranks on server.");
				ShowPlayerDialog(playerid, DIALOG_UNUSED, DIALOG_STYLE_TABLIST_HEADERS, ""COL_SERVER"F:DM "COL_WHITE"- General Commands", string, "Okay", "Close");
			}
			if(listitem == 3)
			{
				new string[2000];
				format(string,sizeof(string),
				"Commands\tDescription\n\
				"COL_WHITE"/godmode\t"COL_SERVER"To enable/disable godmode.\n\
				"COL_WHITE"/weap\t"COL_SERVER"To spawn player weapon.\n\
				"COL_WHITE"/vehicle\t"COL_SERVER"To spawn player vehicle.\n\
				"COL_WHITE"/tune\t"COL_SERVER"To tune player vehicle.\n\
				"COL_WHITE"/warp\t"COL_SERVER"To teleport player in freeroam.");
				ShowPlayerDialog(playerid, DIALOG_UNUSED, DIALOG_STYLE_TABLIST_HEADERS, ""COL_SERVER"F:DM "COL_WHITE"- Freeroam Commands", string, "Okay", "Close");
			}
		}
	}
	if(dialogid == DIALOG_REPORT)
	{
		new
			id = g_player_listitem[playerid][listitem],
			otherid = rData[id][rID];

        if(response)
        {
            if(!IsPlayerConnected(otherid))
                return ClearBoxReport(id);

			ShowPlayerDialog(playerid, DIALOG_REPORTS, DIALOG_STYLE_INPUT, ""COL_SERVER"F:DM "COL_WHITE"- Reports Panel",
			"What do you want to do about this reports?\n"\
			"If you want to reject it, you can fill in the reason in the box below\n"\
			"However, if you want to accept it. You just need to click on the accept button", "Accept", "Denied");

			SetPVarInt(playerid, "TEMP_LISTITEM", id);
		}
	}
	if(dialogid == DIALOG_ASK)
	{
		new
		    id = g_player_listitem[playerid][listitem],
	     	otherid = aData[id][aID];

        if(response)
        {
            if(!IsPlayerConnected(otherid))
                return ClearBoxAsk(id);

			ShowPlayerDialog(playerid, DIALOG_ASKS, DIALOG_STYLE_INPUT, ""COL_SERVER"F:DM "COL_WHITE"- Asks Panel",
			"What do you want to do with this query?\n"\
			"If you don't want to answer, you can fill in the reason in the box below\n"\
			"However, if you want to answer it. You need to fill in the answer in the box", "Accept", "Denied");

			SetPVarInt(playerid, "TEMP_LISTITEM", id);
		}
	}
	if(dialogid == DIALOG_REPORTS)
	{
		new id = GetPVarInt(playerid, "TEMP_LISTITEM");
        
        if(Iter_Contains(Reports, id))
        {
            if(response){
                ReportMessage(rData[id][rID], "Administrator %s accept your reports", g_player_name[playerid]);

                AdminMessage(playerid, "You receive reports from %s", rData[id][rName]);
            }
            else
            {
                ReportMessage(rData[id][rID], "Administrator %s denied your reports, feedback: %s", g_player_name[playerid], inputtext);

                AdminMessage(playerid, "You receive reports from %s", rData[id][rName]);
            }
        }
        ClearBoxReport(id);
        DeletePVar(playerid, "TEMP_LISTITEM");
	}
	if(dialogid == DIALOG_ASKS)
	{
		new id = GetPVarInt(playerid, "TEMP_LISTITEM");

		if(strlen(inputtext))
		{
            if(Iter_Contains(Asks, id))
            {
	            if(response){
	                AskMessage(aData[id][aID], "Admininstrator %s accept your question, feedback: %s", g_player_name[playerid], inputtext);

					AdminMessage(playerid, "You receive question from %s", aData[id][aName]);
	            }
	            else
	            {
	                AskMessage(aData[id][aID], "Admininstrator %s denied your question, feedback: %s", g_player_name[playerid], inputtext);

					AdminMessage(playerid, "You receive question from %s", aData[id][aName]);
	            }
	        }
		}
        ClearBoxAsk(id);
        DeletePVar(playerid, "TEMP_LISTITEM");
	}
	if(dialogid == DIALOG_SETTINGS)
	{	
		if(response)
		{
			if(listitem == 0)
			{
				ShowPlayerDialog(playerid, SETTINGS_PASSWORD, DIALOG_STYLE_PASSWORD, ""COL_SERVER"F:DM "COL_WHITE"- Change Password", "\
				Please, enter your new password.\n\
				Afterwards you will use your new password to login.\n\
				Length: "COL_GREEN"4-24\n\
				", "Change", "Cancel");
			}
		}
	}
	if(dialogid == SETTINGS_PASSWORD)
	{
		if(!response) return callcmd::settings(playerid);
		if(response)
		{
			new password1[24];
			if(sscanf(inputtext, "s[24]", password1)) 
				return ShowPlayerDialog(playerid, SETTINGS_PASSWORD, DIALOG_STYLE_PASSWORD, ""COL_SERVER"F:DM "COL_WHITE"- Change Password", "\
				Please, enter your new password.\n\
				Afterwards you will use your new password to login.\n\
				Length: "COL_GREEN"4-24\n\
				", "Change", "Cancel");

			if(strlen(password1) < 4 || strlen(password1) > 24)  
				return ShowPlayerDialog(playerid, SETTINGS_PASSWORD, DIALOG_STYLE_PASSWORD, ""COL_SERVER"F:DM "COL_WHITE"- Change Password", "\
				Please, enter your new password.\n\
				Afterwards you will use your new password to login.\n\
				Length: "COL_GREEN"4-24\n\
				", "Change", "Cancel");

			new query[160];
			mysql_format(gSQL, query, sizeof(query), "UPDATE `users` SET `password` = MD5('%s') WHERE `id`=%d", password1, pData[playerid][pID]);
			mysql_tquery(gSQL, query);

			ClearChat(playerid, 25);
			InfoMessage(playerid, "You have successfully changed your password.");
			InfoMessage(playerid, "Your new password: %s", password1);
			InfoMessage(playerid, "Use [F8] to save screenshoot of your new password.");
		}
	}
	if(dialogid == DIALOG_VEHICLES)
	{
		if(response)
		{
			if(listitem == 0)
			{
				ShowModelSelectionMenu(playerid, Airplanes, "Airplanes");
			}
			if(listitem == 1)
			{
				ShowModelSelectionMenu(playerid, Bikes, "Bikes");
			}
			if(listitem == 2)
			{
				ShowModelSelectionMenu(playerid, Boats, "Boats");
			}
			if(listitem == 3)
			{
				ShowModelSelectionMenu(playerid, Convertible, "Convertible");
			}
			if(listitem == 4)
			{
				ShowModelSelectionMenu(playerid, Helicopters, "Helicopters");
			}
			if(listitem == 5)
			{
				ShowModelSelectionMenu(playerid, Industrials, "Industrials");
			}
			if(listitem == 6)
			{
				ShowModelSelectionMenu(playerid, Lowrider, "Lowrider");
			}
			if(listitem == 7)
			{
				ShowModelSelectionMenu(playerid, OffRoad, "Off Road");
			}
			if(listitem == 8)
			{
				ShowModelSelectionMenu(playerid, PublicService, "Public Service");
			}
			if(listitem == 9)
			{
				ShowModelSelectionMenu(playerid, RC, "RC Vehicles");
			}
			if(listitem == 10)
			{
				ShowModelSelectionMenu(playerid, Saloon, "Saloons");
			}
			if(listitem == 11)
			{
				ShowModelSelectionMenu(playerid, Sports, "Sport Cars");
			}
			if(listitem == 12)
			{
				ShowModelSelectionMenu(playerid, StationWagon, "Station Wagon");
			}
			if(listitem == 13)
			{
				ShowModelSelectionMenu(playerid, Trailer, "Trailer");
			}
			if(listitem == 14)
			{
				ShowModelSelectionMenu(playerid, Unique, "Unique Vehicles");
			}
		}
	}
	if(dialogid == SERVER_SETTINGS)
	{	
		if(response)
		{
			if(listitem == 0)
			{
				if(pData[playerid][pAdmin] < 2)
					return NoAdmin(playerid);

				if(GetAskTotal() == -1)
				    return AskMessage(playerid, "There are no question at this time");

				AdminMessage(playerid, "You cleared all question");
				SendClientMessageToAll(COLOR_ANTICHEAT, "A: "COL_WHITE"Administrator %s has cleared question box", GetName(playerid));
				foreach(new i : Asks){
					ClearBoxAsk(i);
				}
			}
			if(listitem == 1)
			{
				if(pData[playerid][pAdmin] < 2)
					return NoAdmin(playerid);

				if(GetReportTotal() == -1)
				    return ReportMessage(playerid, "There are no reports at this time");
				    
				AdminMessage(playerid, "You cleared all reports");
				SendClientMessageToAll(COLOR_ANTICHEAT, "A: "COL_WHITE"Administrator %s has cleared reports box", GetName(playerid));
				foreach(new i : Reports){
					ClearBoxReport(i);
				}
			}
		}
	}
	if((dialogid == DIALOG_DUEL) && response)
	{
		erase(szBigString);
		switch (listitem)
		{
			case 0:
			{
				new
					iLocationID = g_duelData[playerid][E_LOCATION_ID];

				for(new i = 0; i < sizeof(g_duelLocationData); i++) {
					format(szBigString, sizeof(szBigString), "%s%s%s\n", szBigString, iLocationID == i ? (COL_WHITE) : (COL_WHITE), g_duelLocationData[i][E_NAME]);
				}
				ShowPlayerDialog(playerid, DIALOG_DUEL_LOCATION, DIALOG_STYLE_LIST, ""COL_SERVER"F:DM "COL_WHITE"- Duel Location", szBigString, "Select", "Back");
			}
			case 1: ShowPlayerDialog(playerid, DIALOG_DUEL_PLAYER, DIALOG_STYLE_INPUT, ""COL_SERVER"F:DM "COL_WHITE"- Duel Enemies", ""COL_WHITE"Please type the name of the player you wish to duel:\n\n"COL_SERVER"Note: You can enter partially their names.", "Select", "Back");
			case 2: ShowPlayerDialog(playerid, DIALOG_DUEL_HEALTH, DIALOG_STYLE_INPUT, ""COL_SERVER"F:DM "COL_WHITE"- Duel Health", ""COL_WHITE"Enter the amount of health you will begin with:\n\n"COL_SERVER"Note: The default health is 100.0.", "Select", "Back");
			case 3: ShowPlayerDialog(playerid, DIALOG_DUEL_ARMOUR, DIALOG_STYLE_INPUT, ""COL_SERVER"F:DM "COL_WHITE"- Duel Armour", ""COL_WHITE"Enter the amount of armour you will begin with:\n\n"COL_SERVER"Note: The default armour is 100.0.", "Select", "Back");
			case 4:
			{
				new
					iWeapon = g_duelData[playerid][E_WEAPON][0];

				for(new i = 0; i < sizeof(g_WeaponList); i++) {
					format(szBigString, sizeof(szBigString), "%s%s%s\n", szBigString, iWeapon == g_WeaponList[i] ? (COL_WHITE) : (COL_WHITE), ReturnWeaponName(WEAPON:g_WeaponList[i]));
				}
				ShowPlayerDialog(playerid, DIALOG_DUEL_WEAPON, DIALOG_STYLE_LIST, ""COL_SERVER"F:DM "COL_WHITE"- Duel Primary Weapon", szBigString, "Select", "Back");
			}
			case 5:
			{
				new
					iWeapon = g_duelData[playerid][E_WEAPON][1];

				for(new i = 0; i < sizeof(g_WeaponList); i++) {
					format(szBigString, sizeof(szBigString), "%s%s%s\n", szBigString, iWeapon == g_WeaponList[i] ? (COL_WHITE) : (COL_WHITE), ReturnWeaponName(WEAPON:g_WeaponList[i]));
				}
				ShowPlayerDialog(playerid, DIALOG_DUEL_WEAPON_TWO, DIALOG_STYLE_LIST, ""COL_SERVER"F:DM "COL_WHITE"- Duel Secondary Weapon", szBigString, "Select", "Back");
			}
			case 6: 
			{
				g_duelData[playerid][E_WEAPON][0] = 0;
				g_duelData[playerid][E_WEAPON][1] = 0;
				g_duelData[playerid][E_HEALTH] 	  = 100.0;
				g_duelData[playerid][E_ARMOUR] 	  = 100.0;
				g_duelData[playerid][E_PLAYER] 	  = -1;
				ShowPlayerDuelMenu(playerid);
			}
			case 7:
			{
				new targetid = g_duelData[playerid][E_PLAYER];
				if(!IsPlayerConnected(targetid))
				{
					DuelMessage(playerid, "You haven't slected anyone to duel.");
					return ShowPlayerDuelMenu(playerid);
				}
				p_duelInvitation[playerid][targetid] = gettime()+30;

				new duelmap[24];
				if(g_duelData[playerid][E_LOCATION_ID] == 0)
				duelmap = "SF Bridge";
				else if(g_duelData[playerid][E_LOCATION_ID] == 1)
				duelmap = "Star Power";
				else if(g_duelData[playerid][E_LOCATION_ID] == 2)
				duelmap = "Ghost Town";
				else if(g_duelData[playerid][E_LOCATION_ID] == 3)
				duelmap = "Tennis Courts";
				else if(g_duelData[playerid][E_LOCATION_ID] == 4)
				duelmap = "Underwater World";
				else if(g_duelData[playerid][E_LOCATION_ID] == 5)
				duelmap = "Grove Street";
				else if(g_duelData[playerid][E_LOCATION_ID] == 6)
				duelmap = "Ocean Docks";
				else if(g_duelData[playerid][E_LOCATION_ID] == 7)
				duelmap = "Gacia Baseball Ground";

				new gunname[32], gunname2[32];
				GetWeaponName(WEAPON:g_duelData[playerid][E_WEAPON][0], gunname, sizeof(gunname));
				GetWeaponName(WEAPON:g_duelData[playerid][E_WEAPON][1], gunname2, sizeof(gunname2));

				DuelMessage(playerid, "You have sent a duel invitation to %s.", GetName(targetid));
				DuelMessage(targetid, ""COL_DUEL"%s"COL_WHITE" has invited you to duel, {FFFF00}/duel accept %d"COL_WHITE"", GetName(playerid), playerid);
			}
		}
		return 1;
	}
	if(dialogid == DIALOG_DUEL_PLAYER)
	{
		if(!response)
			return ShowPlayerDuelMenu(playerid);

		new targetid;

		if(sscanf(inputtext, "u", targetid))
			return ShowPlayerDialog(playerid, DIALOG_DUEL_PLAYER, DIALOG_STYLE_INPUT, ""COL_SERVER"F:DM "COL_WHITE"- Duel Enemies", ""COL_DUEL"Please type the name of the player you wish to duel:\n\n"COL_SERVER"Note: You can enter partially their names.", "Select", "Back" );

		if(targetid == playerid)
			return ShowPlayerDialog(playerid, DIALOG_DUEL_PLAYER, DIALOG_STYLE_INPUT, ""COL_SERVER"F:DM "COL_WHITE"- Duel Enemies", ""COL_DUEL"Please type the name of the player you wish to duel:\n\n"COL_RED"You can't invite yourself to duel!", "Select", "Back" );

		if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid))
			return ShowPlayerDialog(playerid, DIALOG_DUEL_PLAYER, DIALOG_STYLE_INPUT, ""COL_SERVER"F:DM "COL_WHITE"- Duel Enemies", ""COL_DUEL"Please type the name of the player you wish to duel:\n\n"COL_RED"Player is not connected!", "Select", "Back" );

		if(IsPlayerDueling(playerid))
			return ShowPlayerDialog(playerid, DIALOG_DUEL_PLAYER, DIALOG_STYLE_INPUT, ""COL_SERVER"F:DM "COL_WHITE"- Duel Enemies", ""COL_DUEL"Please type the name of the player you wish to duel:\n\n"COL_RED"You are already in a duel!", "Select", "Back" );

		if(IsPlayerDueling(targetid))
			return ShowPlayerDialog(playerid, DIALOG_DUEL_PLAYER, DIALOG_STYLE_INPUT, ""COL_SERVER"F:DM "COL_WHITE"- Duel Enemies", ""COL_DUEL"Please type the name of the player you wish to duel:\n\n"COL_RED"This player is already in a duel!", "Select", "Back" );

		if(GetDistanceBetweenPlayers(playerid, targetid) > 25.0)
			return ShowPlayerDialog(playerid, DIALOG_DUEL_PLAYER, DIALOG_STYLE_INPUT, ""COL_SERVER"F:DM "COL_WHITE"- Duel Enemies", ""COL_DUEL"Please type the name of the player you wish to duel:\n\n"COL_RED"The player you wish to duel is not near you.", "Select", "Back" );

		DuelMessage(playerid, "You have selected "COL_DUEL"%s"COL_WHITE" as your enemies.", GetName(targetid));
		g_duelData[playerid][E_PLAYER] = targetid;
		return ShowPlayerDuelMenu(playerid), 1;
	}
	if(dialogid == DIALOG_DUEL_LOCATION)
	{
		if (!response)
			return ShowPlayerDuelMenu(playerid);

		if(g_duelData[playerid][E_LOCATION_ID] == listitem)
		{
			DuelMessage(playerid, "You have already selected this location.");
			return ShowPlayerDuelMenu(playerid);
		}
		DuelMessage(playerid, "You have changed the duel location to "COL_DUEL"%s"COL_WHITE"", g_duelLocationData[listitem][E_NAME]);
		g_duelData[playerid][E_LOCATION_ID] = listitem;
		ShowPlayerDuelMenu(playerid);
		return 1;
	}
	if(dialogid == DIALOG_DUEL_WEAPON)
	{
		if(!response)
			return ShowPlayerDuelMenu(playerid);

		if(g_duelData[playerid][E_WEAPON][0] == g_WeaponList[listitem])
		{
			DuelMessage(playerid, "You have already selected this weapon.");
			return ShowPlayerDuelMenu(playerid);
		}
		DuelMessage(playerid, "You have changed Pripary Weapon to "COL_DUEL"%s"COL_WHITE"", ReturnWeaponName(WEAPON:g_WeaponList[listitem]));
		g_duelData[playerid][E_WEAPON][0] = g_WeaponList[listitem];
		ShowPlayerDuelMenu(playerid);
		return 1;
	}
	if(dialogid == DIALOG_DUEL_WEAPON_TWO)
	{
		if(!response)
			return ShowPlayerDuelMenu(playerid);

		if(g_duelData[playerid][E_WEAPON][1] == g_WeaponList[listitem])
		{
			DuelMessage(playerid, "You have already selected this weapon.");
			return ShowPlayerDuelMenu(playerid);
		}
		DuelMessage(playerid, "You have changed Secondary Weapon to "COL_DUEL"%s"COL_WHITE"", ReturnWeaponName(WEAPON:g_WeaponList[listitem]));
		g_duelData[playerid][E_WEAPON][1] = g_WeaponList[listitem];
		ShowPlayerDuelMenu(playerid);
		return 1;
	}
	if(dialogid == DIALOG_DUEL_HEALTH)
	{
		if(!response)
			return ShowPlayerDuelMenu(playerid);

		new
			Float: fHealth;

		if(sscanf(inputtext, "f", fHealth))
			return ShowPlayerDialog(playerid, DIALOG_DUEL_HEALTH, DIALOG_STYLE_INPUT, ""COL_SERVER"F:DM "COL_WHITE"- Duel Health", ""COL_DUEL"Enter the amount of health you will begin with:\n\n"COL_SERVER"Note: The default health is 100.0.", "Select", "Back" );

		if(!(1.0 <= fHealth <= 100.0))
			return ShowPlayerDialog(playerid, DIALOG_DUEL_HEALTH, DIALOG_STYLE_INPUT, ""COL_SERVER"F:DM "COL_WHITE"- Duel Health", ""COL_DUEL"Enter the amount of health you will begin with:\n\n"COL_RED"The amount you have entered is a invalid amount, 1 to 100 only!", "Select", "Back" );

		DuelMessage(playerid, "You have changed Health to "COL_SERVER"%0.2f%%"COL_WHITE"", fHealth);
		g_duelData[playerid][E_HEALTH] = fHealth;
		ShowPlayerDuelMenu(playerid);
		return 1;
	}
	if(dialogid == DIALOG_DUEL_ARMOUR)
	{
		if(!response)
			return ShowPlayerDuelMenu(playerid);

		new
			Float: fArmour;

		if(sscanf(inputtext, "f", fArmour))
			return ShowPlayerDialog(playerid, DIALOG_DUEL_ARMOUR, DIALOG_STYLE_INPUT, ""COL_SERVER"F:DM "COL_WHITE"- Duel Armour", ""COL_DUEL"Enter the amount of armour you will begin with:\n\n"COL_SERVER"Note: The default armour is 100.0.", "Select", "Back" );

		if(!( 0.0 <= fArmour <= 100.0))
			return ShowPlayerDialog(playerid, DIALOG_DUEL_ARMOUR, DIALOG_STYLE_INPUT, ""COL_SERVER"F:DM "COL_WHITE"- Duel Armour", ""COL_DUEL"Enter the amount of armour you will begin with:\n\n"COL_RED"The amount you have entered is a invalid amount, 0 to 100 only!", "Select", "Back" );
        
		DuelMessage(playerid, "You have changed Armour to "COL_SERVER"%0.2f%%"COL_WHITE"", fArmour);
		g_duelData[playerid][E_ARMOUR] = fArmour;
		ShowPlayerDuelMenu(playerid);
		return 1;
	}
	return 1;
}

public OnPlayerEnterGangZone(playerid, zoneid)
{
	return 1;
}

public OnPlayerLeaveGangZone(playerid, zoneid)
{
	return 1;
}

public OnPlayerEnterPlayerGangZone(playerid, zoneid)
{
	return 1;
}

public OnPlayerLeavePlayerGangZone(playerid, zoneid)
{
	return 1;
}

public OnPlayerClickGangZone(playerid, zoneid)
{
	return 1;
}

public OnPlayerClickPlayerGangZone(playerid, zoneid)
{
	return 1;
}

public OnPlayerSelectedMenuRow(playerid, row)
{
	return 1;
}

public OnPlayerExitedMenu(playerid)
{
	return 1;
}

public OnClientCheckResponse(playerid, actionid, memaddr, retndata)
{
	return 1;
}

public OnRconLoginAttempt(ip[], password[], success)
{
	return 1;
}

public OnPlayerFinishedDownloading(playerid, virtualworld)
{
	return 1;
}

public OnPlayerRequestDownload(playerid, DOWNLOAD_REQUEST:type, crc)
{
	return 1;
}

public OnRconCommand(cmd[])
{
	return 0;
}

public OnPlayerSelectObject(playerid, SELECT_OBJECT:type, objectid, modelid, Float:fX, Float:fY, Float:fZ)
{
	return 1;
}

public OnPlayerEditObject(playerid, playerobject, objectid, EDIT_RESPONSE:response, Float:fX, Float:fY, Float:fZ, Float:fRotX, Float:fRotY, Float:fRotZ)
{
	return 1;
}

public OnPlayerEditAttachedObject(playerid, EDIT_RESPONSE:response, index, modelid, boneid, Float:fOffsetX, Float:fOffsetY, Float:fOffsetZ, Float:fRotX, Float:fRotY, Float:fRotZ, Float:fScaleX, Float:fScaleY, Float:fScaleZ)
{
	return 1;
}

public OnObjectMoved(objectid)
{
	return 1;
}

public OnPlayerObjectMoved(playerid, objectid)
{
	return 1;
}

public OnPlayerPickUpPickup(playerid, pickupid)
{
	return 1;
}

public OnPlayerPickUpPlayerPickup(playerid, pickupid)
{
	return 1;
}

public OnPickupStreamIn(pickupid, playerid)
{
	return 1;
}

public OnPickupStreamOut(pickupid, playerid)
{
	return 1;
}

public OnPlayerPickupStreamIn(pickupid, playerid)
{
	return 1;
}

public OnPlayerPickupStreamOut(pickupid, playerid)
{
	return 1;
}

public OnPlayerStreamIn(playerid, forplayerid)
{
	return 1;
}

public OnPlayerStreamOut(playerid, forplayerid)
{
	return 1;
}

public OnPlayerDamage(&playerid, &Float:amount, &issuerid, &WEAPON:weapon, &bodypart)
{
	if(!pData[playerid][pLoggedIn]) return 0;
	if(pData[playerid][pLobbyIn]) return 0;
	if(ProtectionSpawn[playerid]) {
		GameTextForPlayer(issuerid, "~w~~h~SPAWN ~r~PROTECTION!", 3000, 4);
		return 0;
	}
	if(AdminDuty[playerid]) {
		GameTextForPlayer(issuerid, "~w~~h~DON'T ~r~SHOT ADMINS!", 3000, 4);
		return 0;
	}
	if(Godmode[playerid]) {
		GameTextForPlayer(issuerid, "~w~~h~GODMODE ~r~PROTECTION!", 3000, 4);
		return 0;
	}

	new string[48];
	format(string, sizeof(string), ""COL_WHITE"-%.2f"COL_RED" by %s", amount, GetName(issuerid));
	SetPlayerChatBubble(playerid, string, -1, 10.0, 2000);

	TextDrawShowForPlayer(issuerid, HitMarker);
	timarker = SetTimerEx("HTD", 200, true, "%d", issuerid);

	if(HeadshotEnable[issuerid] && bodypart == 9)
	{
		pData[issuerid][pHeadshot]++;
		SetPlayerHealth(playerid, 0.0);
		GameTextForPlayer(issuerid, "~r~~h~Headshot!", 3000, 4);
		return 0;
	}
	return 1;
}

public OnPlayerClickPlayer(playerid, clickedplayerid, CLICK_SOURCE:source)
{
	return 1;
}

public OnPlayerWeaponShot(playerid, WEAPON:weaponid, BULLET_HIT_TYPE:hittype, hitid, Float:fX, Float:fY, Float:fZ)
{
	return 1;
}

public OnPlayerClickMap(playerid, Float:fX, Float:fY, Float:fZ)
{
	return 1;
}

public OnIncomingConnection(playerid, ip_address[], port)
{
	return 1;
}

public OnPlayerInteriorChange(playerid, newinteriorid, oldinteriorid)
{
	return 1;
}

public OnPlayerClickTextDraw(playerid, Text:clickedid)
{
	return 1;
}

public OnPlayerClickPlayerTextDraw(playerid, PlayerText:playertextid)
{
	return 1;
}

public OnTrailerUpdate(playerid, vehicleid)
{
	return 1;
}

public OnVehicleSirenStateChange(playerid, vehicleid, newstate)
{
	return 1;
}

public OnVehicleStreamIn(vehicleid, forplayerid)
{
	return 1;
}

public OnVehicleStreamOut(vehicleid, forplayerid)
{
	return 1;
}

public OnVehicleMod(playerid, vehicleid, componentid)
{
	return 1;
}

public OnEnterExitModShop(playerid, enterexit, interiorid)
{
	return 1;
}

public OnVehiclePaintjob(playerid, vehicleid, paintjobid)
{
	return 1;
}

public OnVehicleRespray(playerid, vehicleid, color1, color2)
{
	return 1;
}

public OnVehicleDamageStatusUpdate(vehicleid, playerid)
{
	return 1;
}

public OnUnoccupiedVehicleUpdate(vehicleid, playerid, passenger_seat, Float:new_x, Float:new_y, Float:new_z, Float:vel_x, Float:vel_y, Float:vel_z)
{
	return 1;
}

function OnUserRegister(playerid)
{
	pData[playerid][pID] = cache_insert_id();
	pData[playerid][pLoggedIn]  = true;
	pData[playerid][pLobbyIn]  = true;
	SpawnPlayer(playerid);
	SendClientMessage(playerid, COLOR_SERVER, "F:DM "COL_WHITE"-> Welcome "COL_SERVER"%s!, "COL_WHITE"hope you enjoy playing in Flash Deathmatch.", GetName(playerid));
	return 1;
}

function OnUserLogin(playerid)
{
	new rows;
	cache_get_row_count(rows);
	if(rows == 0)
	{
		ShowPlayerDialog(playerid, DIALOG_LOGIN, DIALOG_STYLE_PASSWORD, ""COL_SERVER"F:DM "COL_WHITE"- Login", ""COL_WHITE"[MY] Welamat datang kembali di "COL_SERVER"Flash Deathmatch"COL_WHITE",\nSila masukkan kata laluan untuk masuk ke account anda\n\n[EN] Welcome back to "COL_SERVER"Flash Deathmatch"COL_WHITE",\nPlease enter the password to log in to your account", "Login", "Exit");
		ErrorMessage(playerid, "You input wrong password!");
	}
	else
	{
		cache_get_value_name_int(0, "id", pData[playerid][pID]);
		cache_get_value_name_int(0, "admin", pData[playerid][pAdmin]);
		cache_get_value_name_int(0, "skin", pData[playerid][pSkin]);
		cache_get_value_name_int(0, "score", pData[playerid][pScore]);
		cache_get_value_name_int(0, "money", pData[playerid][pMoney]);
		cache_get_value_name_int(0, "onlineseconds", pData[playerid][pSeconds]);
		cache_get_value_name_int(0, "onlineminutes", pData[playerid][pMinutes]);
		cache_get_value_name_int(0, "onlinehours", pData[playerid][pHours]);
		cache_get_value_name_int(0, "kills", pData[playerid][pKills]);
		cache_get_value_name_int(0, "deaths", pData[playerid][pDeaths]);
		cache_get_value_name_int(0, "headshot", pData[playerid][pHeadshot]);
		cache_get_value_name_int(0, "assist", pData[playerid][pAssist]);
		cache_get_value_name_int(0, "muted", pData[playerid][pMuted]);
		cache_get_value_name_int(0, "mutetime", pData[playerid][pMuteTime]);
		cache_get_value_name_int(0, "arenakills", pData[playerid][pArenaKills]);
		cache_get_value_name_int(0, "duelwins", pData[playerid][pDuelWins]);
		pData[playerid][pLoggedIn]  = true;
		pData[playerid][pLobbyIn]  = true;
		SpawnPlayer(playerid);
		SendClientMessage(playerid, COLOR_SERVER, "F:DM "COL_WHITE"-> Welcome back "COL_SERVER"%s!, "COL_WHITE"hope you enjoy playing in Flash Deathmatch.", GetName(playerid));
		if(pData[playerid][pAdmin] > 1)
		{
			AdminMessage(playerid, "-> You has been logged as %s, use {FFFF00}/ahelp"COL_WHITE" for commands", GetAdminRank(playerid));
		}
		else InfoMessage(playerid, "-> use {FFFF00}/ask"COL_WHITE" for question {FFFF00}/report"COL_WHITE" for reports and {FFFF00}/help"COL_WHITE" for commands");
	}
	return 1;
}

function AutoKick(playerid) 
{
	if(pData[playerid][pLoggedIn]) return 1;
	InfoMessage(playerid, "60 seconds pass, you are being automaticly kicked.");
	return 1;
}

public OnPlayerCommandPerformed(playerid, cmd[], params[], result, flags)
{
	if(result == -1)
	{
		ErrorMessage(playerid, "Command that you typed doesn't exist. {FFFF00}/help"COL_WHITE"");
		return 0;
	}
	else
	{
		if(pData[playerid][pAdmin] > 3) CmdTime[playerid] = gettime()+0;
		else if(pData[playerid][pAdmin] > 1) CmdTime[playerid] = gettime()+1;
		else CmdTime[playerid] = gettime()+5;
	}
	return 1;
}

public OnPlayerCommandReceived(playerid, cmd[], params[], flags)
{
	if(!pData[playerid][pLoggedIn])
	{
		ErrorMessage(playerid, "You must login to use commands.");
		return 0;
	}
	if(CmdTime[playerid] > gettime())
	{
		ErrorMessage(playerid, "Calm down, don't spamming");
		return 0;
	}
	return 1;
}

public OnPlayerModelSelection(playerid, response, listid, modelid)
{
	new VW, INT;
    if(listid == Airplanes || listid == Bikes || listid == Boats || listid == Convertible || listid == Helicopters || listid == Industrials || listid == Lowrider || listid == OffRoad || listid == PublicService || listid == RC || listid == Saloon || listid == Sports || listid == StationWagon || listid == Trailer || listid == Unique)
    {
        if(response)
        {
            DestroyVehicle(playerCar[playerid]);
            new Float:pos[4], color[2];
			GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
			GetPlayerFacingAngle(playerid, pos[3]);
			color[0] = random(256);
			color[1] = random(256);
			playerCar[playerid] = CreateVehicle(modelid, pos[0], pos[1], pos[2]+2.0, pos[3], color[0], color[1], -1);
			VW = GetPlayerVirtualWorld(playerid);
    		INT = GetPlayerInterior(playerid);
    		SetVehicleVirtualWorld(playerCar[playerid], VW);
    		LinkVehicleToInterior(playerCar[playerid], INT);
			PutPlayerInVehicle(playerid, playerCar[playerid], 0);
			VehicleMessage(playerid, "You success spawn vehicle %d.", modelid);
        }
        else VehicleMessage(playerid, "Canceled spawn vehicles.");
        return 1;
    }
    return 1;
}

function HTD(issuerid)
{
	TextDrawHideForPlayer(issuerid, HitMarker);
	KillTimer(timarker);
	return 1;
}


//=========================================================================
CMD:settings(playerid)
{
	new string[256];
	format(string,sizeof(string),
	"Option\tDescription\n\
	"COL_WHITE"Change Password\t"COL_SERVER"To change your password.");
	ShowPlayerDialog(playerid, DIALOG_SETTINGS, DIALOG_STYLE_TABLIST_HEADERS, ""COL_SERVER"F:DM "COL_WHITE"- Settings Account", string, "Select", "Close");
	return 1;
}

//=========================================================================
CMD:help(playerid)
{
	new string[256];
	format(string,sizeof(string),
	"Option\tDescription\n\
	"COL_WHITE"Account\t"COL_SERVER"Command to setting your account.\n\
	"COL_WHITE"Mode\t"COL_SERVER"Commands mode.\n\
	"COL_WHITE"General\t"COL_SERVER"Commands of servers.\n\
	"COL_WHITE"Freeroam\t"COL_SERVER"Commands in freeroam mode.");
	ShowPlayerDialog(playerid, DIALOG_HELP, DIALOG_STYLE_TABLIST_HEADERS, ""COL_SERVER"F:DM "COL_WHITE"- Help Commands", string, "Okay", "Close");
	return 1;
}

CMD:lobby(playerid)
{
	if(pData[playerid][pLobbyIn]) 
		return ErrorMessage(playerid, "You already in lobby!"); 

	if(IsPlayerDueling(playerid))
		return ErrorMessage(playerid, "Please finish your duel!");

	pData[playerid][pLobbyIn] = true;
	pData[playerid][pDmIn] = 0;
	pData[playerid][pFreeroamIn] = false;

    //===========================[ FR ]
	Godmode[playerid] = false;
	playerCar[playerid] = INVALID_VEHICLE_ID;
	DestroyVehicle(playerCar[playerid]);

	//===========================[ DM ]
	CbugEnable[playerid] = false;
	HeadshotEnable[playerid] = false;
	SpawnEx(playerid);

	return 1;
}

CMD:rules(playerid)
{
	new string[700];
	format(string,sizeof(string),
	""COL_WHITE"1. - "COL_SERVER"Don't using programs that harm the server or player.\n\
	"COL_WHITE"2. - "COL_SERVER"Don't team's Up in the deathmatch arena to kill players until the player feels harmed.\n\
	"COL_WHITE"3. - "COL_SERVER"Don't make excessive insults to race, nation, country and religion\n\
	"COL_WHITE"4. - "COL_SERVER"Don't cheating for farming score and money by using bots etc\n\
	"COL_WHITE"5. - "COL_SERVER"Don't using forbidden file like fast Jiggle, anti stun, auto run, sprinthook, auto litefoot etc.");
	ShowPlayerDialog(playerid, DIALOG_UNUSED, DIALOG_STYLE_MSGBOX, ""COL_SERVER"F:DM "COL_WHITE"- Server Rules", string, "Okay", "Close");
	return 1;
}

CMD:checkdevice(playerid, params[])
{
	new targetid, str[126];
	new vstr[40];
    GetPlayerVersion(playerid, vstr, sizeof(vstr));
	if(sscanf(params,"d", targetid)) 
		return UsageMessage(playerid, "/checkdevice [playerid]");

	if(!pData[targetid][pLoggedIn])
		return ErrorMessage(playerid, "The player is not connected!");

	if(IsPlayerUsingSampMobile(targetid)) 
	{
		str = ""COL_GREEN"Android"COL_WHITE"";
	}
	else if(!IsPlayerUsingSampMobile(targetid)) 
	{
		str = ""COL_RED"PC"COL_WHITE"";
	}

	InfoMessage(playerid, "Recently detect "COL_SERVER"%s "COL_WHITE"as Device: %s -> %s", GetName(targetid), str, vstr);
	return 1;
}

CMD:checkfps(playerid, params[])
{
	new targetid;
	if(sscanf(params,"d", targetid)) 
		return UsageMessage(playerid, "/checkfps [playerid]");

	if(!pData[targetid][pLoggedIn])
		return ErrorMessage(playerid, "The player is not connected!");

	InfoMessage(playerid, "Recently detect "COL_SERVER"%s "COL_WHITE"as FPS: %d", GetName(targetid), pFPS[targetid]);
	return 1;
}

CMD:checkping(playerid, params[])
{
	new targetid;
	if(sscanf(params,"d", targetid)) 
		return UsageMessage(playerid, "/checkping [playerid]");

	if(!pData[targetid][pLoggedIn])
		return ErrorMessage(playerid, "The player is not connected!");

	InfoMessage(playerid, "Recently detect "COL_SERVER"%s "COL_WHITE"as PING: %d", GetName(targetid), GetPlayerPing(targetid));
	return 1;
}

CMD:checkpl(playerid, params[])
{
	new targetid;
	if(sscanf(params,"d", targetid)) 
		return UsageMessage(playerid, "/checkpl [playerid]");

	if(!pData[targetid][pLoggedIn])
		return ErrorMessage(playerid, "The player is not connected!");

	InfoMessage(playerid, "Recently detect "COL_SERVER"%s "COL_WHITE"as PL: %.2f", GetName(targetid), NetStats_PacketLossPercent(targetid));
	return 1;
}

CMD:ranks(playerid,params[])
{
	new string[700];
	format(string,sizeof(string),
	"Ranks\tDescription\n\
	"COL_WHITE"Beginner\t"COL_SERVER"50 Score\n\
	"COL_WHITE"Junior\t"COL_SERVER"100 Score\n\
	"COL_WHITE"Senior\t"COL_SERVER"500 Score\n\
	"COL_WHITE"Pro\t"COL_SERVER"1000 Score\n\
	"COL_WHITE"Extension\t"COL_SERVER"2500 Score\n\
	"COL_WHITE"Legendary\t"COL_SERVER"5000 Score\n\
	"COL_WHITE"Executive\t"COL_SERVER"10000 Score\n\
	"COL_WHITE"General\t"COL_SERVER"25000 Score\n\
	"COL_WHITE"Master\t"COL_SERVER"50000 Score\n\
	"COL_WHITE"Veteran\t"COL_SERVER"100000 Score\n\
	"COL_WHITE"Notes\t"COL_SERVER"Every kill streak you will get 5 score keep pust it to up your ranks");
	ShowPlayerDialog(playerid, DIALOG_UNUSED, DIALOG_STYLE_TABLIST_HEADERS, ""COL_SERVER"F:DM "COL_WHITE"- Rank list", string, "Okay", "Close");
	return 1;
}

CMD:admins(playerid, params[])
{
	new count = 0;
	SendClientMessage(playerid, COLOR_WHITE, " ");
	SendClientMessage(playerid, COLOR_SERVER, "----------| Online Admins |----------");
	for(new i = 0; i < MAX_PLAYERS; i ++)
	{
		if(IsPlayerConnected(i))
		{
			if(pData[i][pAdmin] >= 1)
			{
				new aduty[48];
				if(AdminDuty[i]) 
				{
					aduty = ""COL_GREEN"On Duty"COL_WHITE"";
				}
				else if(!AdminDuty[i]) 
				{
					aduty = ""COL_RED"Off Duty"COL_WHITE"";
				}
				SendClientMessage(playerid, COLOR_WHITE, "-> Admin ID: %s [%d], Admin Rank: %s, Admin Status: %s", GetName(i), i, GetAdminRank(i), aduty);
				SendClientMessage(playerid, COLOR_WHITE, " ");
				count++;
			}
		}
	}
	if(count == 0)
	{
		SendClientMessage(playerid, COLOR_RED,"No admins are online right now!");
		SendClientMessage(playerid, COLOR_WHITE, " ");
	}
    return 1;
}

CMD:pm(playerid, params[])
{
    new targetid;
    if(sscanf(params, "us[75]", targetid, params[2])) 
		return UsageMessage(playerid, "/pm [playerid, message]");

	if(targetid == playerid)
		return ErrorMessage(playerid, "Don't to your self!");
		
	if(!pData[targetid][pLoggedIn])
		return ErrorMessage(playerid, "The player is not connected!");

    SendClientMessage(targetid, COLOR_YELLOW, "[Private Message] "COL_WHITE"recieved from %s [%d]: %s", GetName(playerid), playerid, params[2]); 
	SendClientMessage(playerid, COLOR_YELLOW, "[Private Message] "COL_WHITE"sent to %s [%d]: %s", GetName(targetid), targetid, params[2]);
	return 1;
}

CMD:report(playerid, params[])
{
	new id = Iter_Free(Reports);
	
	new kontol[50];
	if(sscanf(params, "s[50]", kontol)) 
		return UsageMessage(playerid, "/report [reports]");

	if(id == -1)
	    return ReportMessage(playerid, "Report box is full, please try again!");

	if(strlen(kontol) < 10 || strlen(kontol) > 50)
        return ErrorMessage(playerid, "Your report must more than 10 words and no more than 50 words");
	    
	if(IsPlayerHasReport(playerid))
	    return ReportMessage(playerid, "You can only report 1 question. You can report again after getting an answer");

	rData[id][rID] = playerid;
	format(rData[id][rName], MAX_PLAYER_NAME, "%s", g_player_name[playerid]);
	format(rData[id][rText], 50, "%s", kontol);
	
	ReportMessage(playerid, "You successfully sent a reports to the administrator");
	SendMessageToAdmins("-> Report: someone has made a report, use {FFFF00}/reports");
	
	Iter_Add(Reports, id);
	return 1;
}

CMD:ask(playerid, params[])
{
	new id = Iter_Free(Asks);

	new kontol[50];
	if(sscanf(params, "s[50]", kontol)) 
		return UsageMessage(playerid, "/ask [question]");

	if(id == -1)
	    return AskMessage(playerid, "Question box is full, please try again!");

	if(strlen(kontol) < 10 || strlen(kontol) > 50)
        return ErrorMessage(playerid, "Your question must more than 10 words and no more than 50 words");
	    
	if(IsPlayerHasAsk(playerid))
	    return AskMessage(playerid, "You can only ask 1 question. You can ask again after getting an answer");
	    
	aData[id][aID] = playerid;
	format(aData[id][aName], MAX_PLAYER_NAME, "%s", g_player_name[playerid]);
	format(aData[id][aText], 50, "%s", kontol);

	AskMessage(playerid, "You successfully sent a question to the administrator");
	SendMessageToAdmins("-> Ask: someone has made a question, use {FFFF00}/asks");
	
	Iter_Add(Asks, id);
	return 1;
}

CMD:cancelreport(playerid)
{	
	if(!IsPlayerHasAsk(playerid))
	    return AskMessage(playerid, "You not send any reports.");
	    
	InfoMessage(playerid, "You has cancel your reports");
	ClearBoxReport(playerid);
	return 1;
}

CMD:cancelask(playerid)
{
	if(!IsPlayerHasAsk(playerid))
	    return AskMessage(playerid, "You not send any question.");
	    
	InfoMessage(playerid, "You has cancel your question");
	ClearBoxAsk(playerid);
	return 1;
}

CMD:pay(playerid, params[])
{
	new targetid, cash;
	if(sscanf(params, "ui", targetid, cash)) 
		return UsageMessage(playerid, "/pay [playerid, cash]");

	if(targetid == playerid)
		return ErrorMessage(playerid, "Don't to your self!");

	if(!pData[targetid][pLoggedIn])
		return ErrorMessage(playerid, "The player is not connected!");

	if(cash < 0 || cash > 10000) 
		return ErrorMessage(playerid, "You amount must than $0 and no more than $10000.");

	pData[playerid][pAdmin] -= cash;
	pData[targetid][pAdmin] += cash;
	SetPlayerMoney(playerid, pData[playerid][pMoney]);
	SetPlayerMoney(targetid, pData[targetid][pMoney]);
	InfoMessage(targetid, "You get $%d from %s.",cash, GetName(playerid));
	InfoMessage(playerid, "You give $%d to %s.",cash, GetName(targetid));
  	return 1;
}

CMD:globalchat(playerid, params[])
{
	new string[256];
	if(sscanf(params, "s[256]", string)) 
		return UsageMessage(playerid, "/globalchat [text]");

	new str[100];
	if(pData[playerid][pAdmin] > 1)
	{
		str = ""COL_WHITE"["COL_GREEN"A"COL_WHITE"]";
	}
	else
	{
		str = "";
	}

	for(new i = 0; i < MAX_PLAYERS; i++)
    {
		SendClientMessage(i, COLOR_SERVER, "[G]"COL_WHITE"%s[%s] %s [%d]: "COL_SERVER"%s", str, GetPlayerRank(playerid), GetName(playerid), playerid, string);
	}
  	return 0;
}
alias:globalchat("g")

//=========================================================================
CMD:duel(playerid, params[])
{
	if(!pData[playerid][pLobbyIn]) 
		return ErrorMessage(playerid, "You must in lobby!");

	if(!strcmp(params, "accept", false, 6))
	{
		new
			targetid;

		if(sscanf(params[7], "u", targetid))
			return UsageMessage(playerid, "/duel [menu]");

		if(targetid == playerid)
			return ErrorMessage(playerid, "Don't to your self!");

		if(!pData[targetid][pLoggedIn])
			return ErrorMessage(playerid, "The player is not connected!");

		if(gettime() > p_duelInvitation[targetid][playerid])
			return ErrorMessage(playerid, "You are not invited to duel or duel has expired, so you cant accept it.");

		if(IsPlayerDueling(playerid))
			return DuelMessage(playerid, "You cannot accept this invite as you are currently dueling.");

		DuelMessage(targetid, ""COL_DUEL"%s"COL_WHITE" has accepted your duel invitation.", GetName(playerid));
		DuelMessage(playerid, "You have accepted "COL_DUEL"%s"COL_WHITE" duel invitation.", GetName(targetid));

		pData[targetid][pDuelIn] = true;
		pData[playerid][pDuelIn] = true;

		pData[targetid][pLobbyIn] = false;
		pData[playerid][pLobbyIn] = false;

		g_duelData[targetid][E_PLAYER] = playerid;
		g_duelData[playerid][E_PLAYER] = targetid;

		new
			iLocation = g_duelData[targetid][E_LOCATION_ID];

		ResetPlayerWeapons(targetid);
		RemovePlayerFromVehicle(targetid);
		SetPlayerArmour(targetid, g_duelData[targetid][E_ARMOUR]);
		SetPlayerHealth(targetid, g_duelData[targetid][E_HEALTH]);
		SetPlayerVirtualWorld(targetid, targetid + 1);
		SetPlayerPos(targetid, g_duelLocationData[iLocation][E_POS_TWO][0], g_duelLocationData[iLocation][E_POS_TWO][1], g_duelLocationData[iLocation][E_POS_TWO][2] );
		SetPlayerFacingAngle(targetid, g_duelLocationData[iLocation][E_POS_TWO][3]);
		SetCameraBehindPlayer(targetid);

		ResetPlayerWeapons(playerid);
		RemovePlayerFromVehicle(playerid);
		SetPlayerArmour(playerid, g_duelData[targetid][E_ARMOUR]);
		SetPlayerHealth(playerid, g_duelData[targetid][E_HEALTH]);
		SetPlayerVirtualWorld(playerid, targetid + 1);
		SetPlayerPos(playerid, g_duelLocationData[iLocation][E_POS_ONE][0], g_duelLocationData[iLocation][E_POS_ONE][1], g_duelLocationData[iLocation][E_POS_ONE][2]);
		SetPlayerFacingAngle(playerid, g_duelLocationData[iLocation][E_POS_ONE][3]);
		SetCameraBehindPlayer(playerid);
		// freeze
		TogglePlayerControllable(playerid, false);
		TogglePlayerControllable(targetid, false);

		// start countdown
		g_duelData[targetid][E_COUNTDOWN] = 4;
		g_duelData[targetid][E_TIMER] = SetTimerEx("OnDuelTimer", 960, true, "d", targetid);

		// give weapon
		GivePlayerWeapon(playerid, WEAPON:g_duelData[targetid][E_WEAPON][0], 5000);
		GivePlayerWeapon(targetid, WEAPON:g_duelData[targetid][E_WEAPON][0], 5000);
		GivePlayerWeapon(playerid, WEAPON:g_duelData[targetid][E_WEAPON][1], 5000);
		GivePlayerWeapon(targetid, WEAPON:g_duelData[targetid][E_WEAPON][1], 5000);
		new string[126];
        format(string, sizeof(string), "Duel: "COL_WHITE"A Duel has started between "COL_DUEL"%s"COL_WHITE" and "COL_DUEL"%s"COL_WHITE"", GetName(playerid), GetName(targetid));
		SendClientMessageToAll(COLOR_SERVER, string);
		// clear invites for safety
		for(new i = 0; i < MAX_PLAYERS; i ++) {
			p_duelInvitation[playerid][i] = 0;
			p_duelInvitation[targetid][i] = 0;
		}
		return 1;
	}
	else if(strmatch(params, "decline"))
	{
		if(ClearDuelInvites(playerid))
		{
			return DuelMessage(playerid, "You have decline every duel offer that you have made.");
		}
		else
		{
			return ErrorMessage(playerid, "You have not made any duel offers recently.");
		}
	}
	else if(strmatch(params, "menu")) {
		return ShowPlayerDuelMenu(playerid);
	}
	return UsageMessage(playerid, "/duel [accept, decline, menu]");
}


//=========================================================================
CMD:dm(playerid)
{
	if(!pData[playerid][pLobbyIn]) 
		return ErrorMessage(playerid, "You must in lobby!"); 

	new string[2000];
	new plvpd, alvpd, pgt, agt, wh, tbe, ghs;
	foreach(Player, i)
	{
		if(IsPlayerConnected(i))
		{
			if(pData[i][pDmIn] == 1)
			{
				plvpd++;
			}
			if(pData[i][pDmIn] == 2)
			{
				alvpd++;
			}
			if(pData[i][pDmIn] == 3)
			{
				pgt++;
			}
			if(pData[i][pDmIn] == 4)
			{
				agt++;
			}
			if(pData[i][pDmIn] == 5)
			{
				wh++;
			}
			if(pData[i][pDmIn] == 6)
			{
				tbe++;
			}
			if(pData[i][pDmIn] == 7)
			{
				ghs++;
			}
		}
	}
	format(string,sizeof(string),
	"Arena\tPlayers\tDevice\n\
	"COL_WHITE"LVPD HQ - "COL_SERVER"deagle, shotgun\t"COL_WHITE"%d\t"COL_RED"PC\n\
	"COL_WHITE"LVPD HQ - "COL_SERVER"deagle, shotgun\t"COL_WHITE"%d\t"COL_GREEN"Android\n\
	"COL_WHITE"Ghost Town - "COL_SERVER"deagle, shotgun\t"COL_WHITE"%d\t"COL_RED"PC\n\
	"COL_WHITE"Ghost Town - "COL_SERVER"deagle, shotgun\t"COL_WHITE"%d\t"COL_GREEN"Android\n\
	"COL_WHITE"[CBUG] Warehouses - "COL_SERVER"deagle\t"COL_WHITE"%d\t"COL_YELLOW"All\n\
	"COL_WHITE"[CBUG] The Big Ear - "COL_SERVER"deagle\t"COL_WHITE"%d\t"COL_YELLOW"All\n\
	"COL_WHITE"[Headshot] Ganton - "COL_SERVER"deagle\t"COL_WHITE"%d\t"COL_YELLOW"All", plvpd, alvpd, pgt, agt, wh, tbe, ghs);
	ShowPlayerDialog(playerid, DIALOG_DEATHMATCH, DIALOG_STYLE_TABLIST_HEADERS, ""COL_SERVER"F:DM "COL_WHITE"- Arena Map", string, "Select", "Cancel");
	return 1;
}

//=========================================================================
CMD:freeroam(playerid)
{
	if(!pData[playerid][pLobbyIn]) 
		return ErrorMessage(playerid, "You must in lobby!"); 
	
	pData[playerid][pLobbyIn] = false;
	pData[playerid][pFreeroamIn] = true;
	SpawnEx(playerid);
	return 1;
}

CMD:godmode(playerid,params[])
{
	if(!pData[playerid][pFreeroamIn]) 
		return ErrorMessage(playerid, "You must in freeroam!"); 

	if(!Godmode[playerid])
	{
		Godmode[playerid] = true;
		InfoMessage(playerid, "You has been enable godmode in freeroam!");
	}
	else if(Godmode[playerid])
	{
		Godmode[playerid] = false;
		InfoMessage(playerid, "You has been disable godmode in freeroam!");
	}
	return 1;
}

CMD:warp(playerid, params[])
{
	if(!pData[playerid][pFreeroamIn]) 
		return ErrorMessage(playerid, "You must in freeroam!"); 

	new targetid;
	if(sscanf(params, "u", targetid))
	    return UsageMessage(playerid, "/warp [playerid]");

	if(targetid == playerid)
		return ErrorMessage(playerid, "Don't to your self!");
    	
	if(!pData[targetid][pLoggedIn])
		return ErrorMessage(playerid, "The player is not connected!");

    new Float:x, Float:y, Float:z, Float:a;
	GetPlayerPos(targetid, x, y, z);
	GetPlayerFacingAngle(targetid, a);
	SetPlayerPos(playerid, x+1, y+1, z);
	SetPlayerFacingAngle(playerid, a);
	SetPlayerInterior(playerid, GetPlayerInterior(targetid));
	SetPlayerVirtualWorld(playerid, GetPlayerVirtualWorld(targetid));
	AdminMessage(targetid, "Administrator %s warp you to him.", GetName(playerid));
	AdminMessage(playerid, "You warp %s to you.", GetName(targetid));
    return 1;
}

CMD:vehicles(playerid, params[])
{
	if(!pData[playerid][pFreeroamIn]) 
		return ErrorMessage(playerid, "You must in freeroam!"); 

	ShowPlayerDialog(playerid, DIALOG_VEHICLES, DIALOG_STYLE_LIST, ""COL_SERVER"F:DM "COL_WHITE"- Vehicle Spawner", "Airplane\nBike\nBoat\nConvertible\nHelicopter\nIndustrial\nLowrider\nOff Road\nPublic Service\nRC Vehicles\nSaloon\nSport\nStation Wagon\nTrailer\nUnique", "Select", "Close");
	return 1;
}

//=========================================================================
CMD:makemeadmin(playerid, params[])
{
	if(!IsPlayerAdmin(playerid))
		return ErrorMessage(playerid, "You must login rcon first");

	if(pData[playerid][pAdmin] == 3)
		return ErrorMessage(playerid, "You already have founders role");

	pData[playerid][pAdmin] = 3;
	AdminMessage(playerid, "You has been login as Founders!");
	return 1;
}

CMD:servers(playerid)
{
	if(pData[playerid][pAdmin] < 2)
		return NoAdmin(playerid);

	if(!AdminDuty[playerid])
		return ErrorMessage(playerid, "You must on duty to use commands");

	new string[256];
	format(string,sizeof(string),
	"Option\tDescription\n\
	"COL_WHITE"Clear Ask\t"COL_SERVER"To clear ask box.\n\
	"COL_WHITE"Clear Report\t"COL_SERVER"To clear report box.");
	ShowPlayerDialog(playerid, SERVER_SETTINGS, DIALOG_STYLE_TABLIST_HEADERS, ""COL_SERVER"F:DM "COL_WHITE"- Server Settings", string, "Select", "Close");
	return 1;
}

CMD:setadmin(playerid,params[])
{
	if(pData[playerid][pAdmin] < 3)
		return NoAdmin(playerid);

	if(!AdminDuty[playerid])
		return ErrorMessage(playerid, "You must on duty to use commands");

	new targetid, level;
	if(sscanf(params, "ui", targetid, level)) 
		return UsageMessage(playerid, "/setadmin [playerid, level]");

	if(targetid == playerid)
		return ErrorMessage(playerid, "Don't to your self!");

	if(!pData[targetid][pLoggedIn])
		return ErrorMessage(playerid, "The player is not connected!");

	if(level < 0 || level > 3) 
		return ErrorMessage(playerid, "Level 2 is the highest admin level.");

	AdminMessage(targetid, "You been set administrator level %d by an founders.",level);
	AdminMessage(playerid, "You set that player administrator level %d.",level);
	pData[targetid][pAdmin] = level;
	return 1;
}

CMD:giveallcash(playerid, params[])
{
	if(pData[playerid][pAdmin] < 3)
		return NoAdmin(playerid);

	if(!AdminDuty[playerid])
		return ErrorMessage(playerid, "You must on duty to use commands");

	new cash;
	if(sscanf(params, "i", cash)) 
		return UsageMessage(playerid, "/giveallcash [cash]");

	for(new i = 0; i < MAX_PLAYERS; i ++)
	{
		if(IsPlayerConnected(i))
		{
			pData[i][pMoney] += cash;
			SetPlayerMoney(i, pData[i][pMoney]);	
		}
	}
	SendClientMessageToAll(COLOR_ANTICHEAT, "A: "COL_WHITE"Administrator %s has given everyone $%d cash", GetName(playerid), cash);
  	return 1;
}

CMD:giveallscore(playerid, params[])
{
	if(pData[playerid][pAdmin] < 3)
		return NoAdmin(playerid);

	if(!AdminDuty[playerid])
		return ErrorMessage(playerid, "You must on duty to use commands");

	new score;
	if(sscanf(params, "i", score)) 
		return UsageMessage(playerid, "/giveallscore [score]");

	for(new i = 0; i < MAX_PLAYERS; i ++)
	{
		if(IsPlayerConnected(i))
		{
			pData[i][pScore] += score;
			SetPlayerScore(i, pData[i][pScore]);
		}
	}
	SendClientMessageToAll(COLOR_ANTICHEAT, "A: "COL_WHITE"Administrator %s has given everyone %d score", GetName(playerid), score);
  	return 1;
}

CMD:gotopos(playerid,params[]) 
{
	if(pData[playerid][pAdmin] < 3)
		return NoAdmin(playerid);

	if(!AdminDuty[playerid])
		return ErrorMessage(playerid, "You must on duty to use commands");

	new
	  Float:pos1,
	  Float:pos2,
	  int;
	if(sscanf(params,"fffd",floatstr(params),pos1,pos2,int)) return UsageMessage(playerid, "/gotopos [x, y, z, int]");
	if(IsPlayerInAnyVehicle(playerid))
	{
		SetVehiclePos(GetPlayerVehicleID(playerid),floatstr(params),pos1,pos2);
		LinkVehicleToInterior(GetPlayerVehicleID(playerid),int);
	}
	else
 	{
		SetPlayerPos(playerid,floatstr(params),pos1,pos2);
		SetPlayerInterior(playerid,int);
	}
  	return 1;
}

CMD:mute(playerid, params[])
{
	if(pData[playerid][pAdmin] < 1)
		return NoAdmin(playerid);

	if(!AdminDuty[playerid])
		return ErrorMessage(playerid, "You must on duty to use commands");

	new targetid, time, reason[256];
	if(sscanf(params,"dds[256]", targetid, time, reason)) 
		return UsageMessage(playerid, "/mute [playerid, time (in minutes), reason]");

	if(targetid == playerid)
		return ErrorMessage(playerid, "Don't to your self!");

	if(!pData[targetid][pLoggedIn])
		return ErrorMessage(playerid, "The player is not connected!");

	if(pData[playerid][pAdmin] < pData[targetid][pAdmin]) 
		return ErrorMessage(playerid, "You can not do that to admin bigger level.");

	if(pData[targetid][pMuted] == 1)
		return ErrorMessage(playerid, "The player already muted");

	pData[targetid][pMuted] = 1;
    pData[targetid][pMuteTime] = time*60000;

	SendClientMessageToAll(COLOR_ANTICHEAT, "A: "COL_WHITE"Administrator %s has muted %s for %i minutes, reason: %s", GetName(playerid), GetName(targetid), time, reason);
   	return 1;
}

CMD:kick(playerid, params[])
{
	if(pData[playerid][pAdmin] < 1)
		return NoAdmin(playerid);

	if(!AdminDuty[playerid])
		return ErrorMessage(playerid, "You must on duty to use commands");

	new targetid, reason[256];
	if(sscanf(params,"ds[256]", targetid, reason)) 
		return UsageMessage(playerid, "/kick [playerid, reason]");

	if(targetid == playerid)
		return ErrorMessage(playerid, "Don't to your self!");

	if(!pData[targetid][pLoggedIn])
		return ErrorMessage(playerid, "The player is not connected!");

	if(pData[playerid][pAdmin] < pData[targetid][pAdmin]) 
		return ErrorMessage(playerid, "You can not do that to admin bigger level.");

	SendClientMessageToAll(COLOR_ANTICHEAT, "A: "COL_WHITE"Administrator %s has kicked %s from servers, reason: %s", GetName(playerid), GetName(targetid), reason);
   	SetTimerEx("KickPlayer", 3000, true, "%d", targetid);
	return 1;
}

CMD:unmute(playerid, params[])
{
	if(pData[playerid][pAdmin] < 1)
		return NoAdmin(playerid);

	if(!AdminDuty[playerid])
		return ErrorMessage(playerid, "You must on duty to use commands");

	new targetid;
	if(sscanf(params,"d", targetid)) 
		return UsageMessage(playerid, "/unmute [playerid]");

	if(targetid == playerid)
		return ErrorMessage(playerid, "Don't to your self!");

	if(!pData[targetid][pLoggedIn])
		return ErrorMessage(playerid, "The player is not connected!");

	if(pData[playerid][pAdmin] < pData[targetid][pAdmin]) 
		return ErrorMessage(playerid, "You can not do that to admin bigger level.");

	if(pData[targetid][pMuted] == 0)
		return ErrorMessage(playerid, "The player not muted");

	pData[targetid][pMuted] = 0;
	pData[targetid][pMuteTime] = 0;
	SendClientMessageToAll(COLOR_ANTICHEAT, "A: "COL_WHITE"Administrator %s has unmuted %s", GetName(playerid), GetName(targetid));
	return 1;
}

CMD:givecash(playerid,params[])
{
	if(pData[playerid][pAdmin] < 3)
		return NoAdmin(playerid);

	if(!AdminDuty[playerid])
		return ErrorMessage(playerid, "You must on duty to use commands");

	new targetid, cash;
	if(sscanf(params, "ui", targetid, cash)) 
		return UsageMessage(playerid, "/givecash [playerid, cash]");

	if(targetid == playerid)
		return ErrorMessage(playerid, "Don't to your self!");

	if(!pData[targetid][pLoggedIn])
		return ErrorMessage(playerid, "The player is not connected!");

	AdminMessage(targetid, "You been give cash to %s as much as $%d by Administrator.", GetName(playerid), cash);
	AdminMessage(playerid, "You give cash to %s as much as $%d.", GetName(targetid), cash);
	pData[targetid][pMoney] += cash;
	SetPlayerMoney(targetid, pData[targetid][pMoney]);
	return 1;
}

CMD:givescore(playerid,params[])
{
	if(pData[playerid][pAdmin] < 3)
		return NoAdmin(playerid);

	if(!AdminDuty[playerid])
		return ErrorMessage(playerid, "You must on duty to use commands");

	new targetid, score;
	if(sscanf(params, "ui", targetid, score)) 
		return UsageMessage(playerid, "/givescore [playerid, score]");

	if(targetid == playerid)
		return ErrorMessage(playerid, "Don't to your self!");

	if(!pData[targetid][pLoggedIn])
		return ErrorMessage(playerid, "The player is not connected!");

	AdminMessage(targetid, "You been give score to %s as much as %d by Administrator.", GetName(playerid), score);
	AdminMessage(playerid, "You give score to %s as much as %d.", GetName(targetid), score);
	pData[targetid][pScore] += score;
	SetPlayerScore(targetid, pData[targetid][pScore]);
	return 1;
}

CMD:bring(playerid, params[])
{
	if(pData[playerid][pAdmin] < 1)
		return NoAdmin(playerid);

	if(!AdminDuty[playerid])
		return ErrorMessage(playerid, "You must on duty to use commands");

	new targetid;
	if(sscanf(params, "u", targetid))
	    return UsageMessage(playerid, "/bring [playerid]");

	if(targetid == playerid)
		return ErrorMessage(playerid, "Don't to your self!");
    	
	if(!pData[targetid][pLoggedIn])
		return ErrorMessage(playerid, "The player is not connected!");

	if(pData[playerid][pAdmin] < pData[targetid][pAdmin]) 
		return ErrorMessage(playerid, "You can not do that to admin bigger level.");

    new Float:x, Float:y, Float:z, Float:a;
	GetPlayerPos(playerid, x, y, z);
	GetPlayerFacingAngle(playerid, a);
	SetPlayerPos(targetid, x+1, y+1, z);
	SetPlayerFacingAngle(targetid, a);
	SetPlayerInterior(targetid, GetPlayerInterior(playerid));
	SetPlayerVirtualWorld(targetid, GetPlayerVirtualWorld(playerid));
	AdminMessage(targetid, "Administrator %s bring to you.", GetName(playerid));
	AdminMessage(playerid, "You bring to %s.", GetName(targetid));
    return 1;
}

CMD:goto(playerid, params[])
{
	if(pData[playerid][pAdmin] < 2)
		return NoAdmin(playerid);

	if(!AdminDuty[playerid])
		return ErrorMessage(playerid, "You must on duty to use commands");

	new targetid;
	if(sscanf(params, "u", targetid))
	    return UsageMessage(playerid, "/goto [playerid]");

	if(targetid == playerid)
		return ErrorMessage(playerid, "Don't to your self!");
    	
	if(!pData[targetid][pLoggedIn])
		return ErrorMessage(playerid, "The player is not connected!");

	if(pData[playerid][pAdmin] < pData[targetid][pAdmin]) 
		return ErrorMessage(playerid, "You can not do that to admin bigger level.");

    new Float:x, Float:y, Float:z, Float:a;
	GetPlayerPos(targetid, x, y, z);
	GetPlayerFacingAngle(targetid, a);
	SetPlayerPos(playerid, x+1, y+1, z);
	SetPlayerFacingAngle(playerid, a);
	SetPlayerInterior(playerid, GetPlayerInterior(targetid));
	SetPlayerVirtualWorld(playerid, GetPlayerVirtualWorld(targetid));
	AdminMessage(targetid, "Administrator %s goto you to him.", GetName(playerid));
	AdminMessage(playerid, "You goto %s to you.", GetName(targetid));
    return 1;
}

CMD:spawn(playerid, params[])
{
	if(pData[playerid][pAdmin] < 2)
		return NoAdmin(playerid);

	if(!AdminDuty[playerid])
		return ErrorMessage(playerid, "You must on duty to use commands");

	new targetid, reason[256];
	if(sscanf(params, "us[256]", targetid, reason)) 
		return UsageMessage(playerid, "/spawn [playerid, reason]");

	if(targetid == playerid)
		return ErrorMessage(playerid, "Don't to your self!");

	if(!pData[targetid][pLoggedIn])
		return ErrorMessage(playerid, "The player is not connected!");

	if(pData[playerid][pAdmin] < pData[targetid][pAdmin]) 
		return ErrorMessage(playerid, "You can not do that to admin bigger level.");

	if(pData[playerid][pLobbyIn]) 
		return ErrorMessage(playerid, "The player already in lobby!"); 

	callcmd::lobby(targetid);
	SendClientMessageToAll(COLOR_ANTICHEAT, "A: "COL_WHITE"Administrator %s teleported %s to lobby", GetName(playerid), GetName(targetid));
	AdminMessage(targetid, "Administrator %s has teleported you to lobby.", GetName(playerid));
	return 1;
}

CMD:dmkick(playerid, params[])
{
	if(pData[playerid][pAdmin] < 2)
		return NoAdmin(playerid);

	if(!AdminDuty[playerid])
		return ErrorMessage(playerid, "You must on duty to use commands");

	new targetid, reason[256];
	if(sscanf(params, "us[256]", targetid, reason)) 
		return UsageMessage(playerid, "/dmkick [playerid, reason]");

	if(targetid == playerid)
		return ErrorMessage(playerid, "Don't to your self!");

	if(!pData[targetid][pLoggedIn])
		return ErrorMessage(playerid, "The player is not connected!");

	if(pData[playerid][pAdmin] < pData[targetid][pAdmin]) 
		return ErrorMessage(playerid, "You can not do that to admin bigger level.");

	if(pData[playerid][pLobbyIn]) 
		return ErrorMessage(playerid, "The player already in lobby!"); 

	if(pData[playerid][pDmIn] < 1) 
		return ErrorMessage(playerid, "The player not in deathmatch!"); 

	callcmd::lobby(targetid);
	SendClientMessageToAll(COLOR_ANTICHEAT, "A: "COL_WHITE"Administrator %s kick %s from deathmatch, reason: %s", GetName(playerid), GetName(targetid), reason);
	AdminMessage(targetid, "Administrator %s has kick you from deathmatch, reason: %s", GetName(playerid), reason);
	return 1;
}

CMD:cc(playerid)
{
	if(pData[playerid][pAdmin] < 2)
		return NoAdmin(playerid);

	if(!AdminDuty[playerid])
		return ErrorMessage(playerid, "You must on duty to use commands");

	foreach(Player, i)
	{
		if(IsPlayerConnected(i))
		{
			ClearChat(i, 25);
		}
	}
	SendClientMessageToAll(COLOR_ANTICHEAT, "A: "COL_WHITE"Administrator %s has cleared chats!", GetName(playerid));
	return 1;
}

CMD:setskin(playerid, params[])
{
	if(pData[playerid][pAdmin] < 2)
		return NoAdmin(playerid);

	if(!AdminDuty[playerid])
		return ErrorMessage(playerid, "You must on duty to use commands");

	new targetid, skin;
	if(sscanf(params, "ui", targetid, skin)) 
		return UsageMessage(playerid, "/setskin [playerid, 0 - 299]");

	if(skin < 1 || skin > 299) 
		return ErrorMessage(playerid, "Invalid skin! [ 0 - 299 ]");
		
	pData[playerid][pSkin] = skin;
	SetPlayerSkin(targetid, pData[playerid][pSkin]);
	AdminMessage(targetid, "Administrator %s has set your skin to %d.", GetName(playerid), skin);
	AdminMessage(playerid, "You set %s skin to %d.", GetName(targetid), skin);
   	return 1;
}

CMD:aduty(playerid,params[])
{
	if(pData[playerid][pAdmin] < 1)
		return NoAdmin(playerid);

	if(!AdminDuty[playerid])
	{
		AdminDuty[playerid] = true;
		SendClientMessageToAll(COLOR_ANTICHEAT, "A: "COL_WHITE"Administrator %s is now on admin duty.", GetName(playerid));
		AdminMessage(playerid, "Don't forget manage {FFFF00}/reports"COL_WHITE", {FFFF00}/asks"COL_WHITE"");
		AdminMessage(playerid, "Please fast response so that players feel helped and not harmed.");
		TurnOnPlayerColor(playerid);
	}
	else if(AdminDuty[playerid])
	{
		AdminDuty[playerid] = false;
		SendClientMessageToAll(COLOR_ANTICHEAT, "A: "COL_WHITE"Administrator %s off admin duty.", GetName(playerid));
		TurnOnPlayerColor(playerid);
	}
	return 1;
}

CMD:ann(playerid, params[])
{
	if(pData[playerid][pAdmin] < 2)
		return NoAdmin(playerid);

	if(!AdminDuty[playerid])
		return ErrorMessage(playerid, "You must on duty to use commands");

	new info[256];
	if(sscanf(params, "s[256]", info)) 
		return UsageMessage(playerid, "/ann [information]");

	SendClientMessage(playerid, COLOR_WHITE, " ");
	SendClientMessage(playerid, COLOR_SERVER, "----------| Server Announcement |----------");
	SendClientMessage(playerid, COLOR_WHITE, "-> [%s] %s [%d]: %s", GetAdminRank(playerid), GetName(playerid), playerid, info);
	SendClientMessage(playerid, COLOR_WHITE, " ");
	return 1;
}

CMD:mutelist(playerid, params[])
{
	if(pData[playerid][pAdmin] < 2)
		return NoAdmin(playerid);

	if(!AdminDuty[playerid])
		return ErrorMessage(playerid, "You must on duty to use commands");

	new count = 0;
	SendClientMessage(playerid, COLOR_WHITE, " ");
	SendClientMessage(playerid, COLOR_SERVER, "----------| Muted Players |----------");
	for(new i = 0; i < MAX_PLAYERS; i ++)
	{
		if(IsPlayerConnected(i))
		{
			if(pData[i][pMuted] == 1)
			{
				new minutes, seconds, time[24];
		    	ConvertTime(gettime() - pData[i][pMuteTime], minutes, seconds);
				format(time, sizeof(time), "%02d:%02d", minutes, seconds);
				SendClientMessage(playerid, COLOR_WHITE, "-> Players ID: %s [%d], Time Left: %s", GetName(i), i, time);
				SendClientMessage(playerid, COLOR_WHITE, " ");
				count++;
			}
		}
	}
	if(count == 0)
	{
		SendClientMessage(playerid, COLOR_RED,"No players mute right now!");
		SendClientMessage(playerid, COLOR_WHITE, " ");
	}
    return 1;
}

CMD:reports(playerid, params[])
{
	if(pData[playerid][pAdmin] < 1)
		return NoAdmin(playerid);

	if(!AdminDuty[playerid])
		return ErrorMessage(playerid, "You must on duty to use commands");

	if(GetReportTotal() == -1)
	    return ReportMessage(playerid, "There are no reports at this time");
	
	new temp_string[75], dialog[300];
	new count;

	format(temp_string, 75, "Players\tReports\n");
	foreach(new id : Reports)
	{
	    format(temp_string, 75, "%s-> "COL_SERVER"%s [%d]\t"COL_WHITE"%s", temp_string, rData[id][rName], rData[id][rID], rData[id][rText]);
	    strcat(dialog, temp_string);
	    g_player_listitem[playerid][count] = rData[id][rID];
	    count++;
	}
	ShowPlayerDialog(playerid, DIALOG_REPORT, DIALOG_STYLE_TABLIST_HEADERS, ""COL_SERVER"F:DM "COL_WHITE"- Reports List", dialog, "Select", "Cancel");
	return 1;
}

CMD:asks(playerid, params[])
{
	if(pData[playerid][pAdmin] < 1)
		return NoAdmin(playerid);

	if(!AdminDuty[playerid])
		return ErrorMessage(playerid, "You must on duty to use commands");

	if(GetAskTotal() == -1)
	    return AskMessage(playerid, "There are no question at this time");

	new temp_string[75], dialog[300];
	new count;

	format(temp_string, 75, "Players\tQuestion\n");
	foreach(new id : Asks)
	{
	    format(temp_string, 75, "%s-> "COL_SERVER"%s [%d]\t"COL_WHITE"%s", temp_string, aData[id][aName], aData[id][aID], aData[id][aText]);
	    strcat(dialog, temp_string);
	    g_player_listitem[playerid][count] = aData[id][aID];
	    count++;
	}
	ShowPlayerDialog(playerid, DIALOG_ASK, DIALOG_STYLE_TABLIST_HEADERS, ""COL_SERVER"F:DM "COL_WHITE"- Question List", dialog, "Select", "Cancel");
	return 1;
}