new PlayerText:StatsTD[MAX_PLAYERS][7];
new PlayerText:infotd[MAX_PLAYERS],
	bool:InfoPlayer[MAX_PLAYERS],
	TimerInfo[MAX_PLAYERS];

new Text:HitMarker;
new timarker;

createGTextdraw()
{
	HitMarker = TextDrawCreate(337.000000, 175.000000, "X");
	TextDrawFont(HitMarker, TEXT_DRAW_FONT_1);
	TextDrawLetterSize(HitMarker, 0.224999, 0.949999);
	TextDrawTextSize(HitMarker, 400.000000, 17.000000);
	TextDrawSetOutline(HitMarker, 0);
	TextDrawSetShadow(HitMarker, 0);
	TextDrawAlignment(HitMarker, TEXT_DRAW_ALIGN_LEFT);
	TextDrawColour(HitMarker, -16776961);
	TextDrawBackgroundColour(HitMarker, 255);
	TextDrawBoxColour(HitMarker, 50);
	TextDrawUseBox(HitMarker, false);
	TextDrawSetProportional(HitMarker, true);
	TextDrawSetSelectable(HitMarker, false);
	return 1;
}

createLTextdraw(playerid)
{
	infotd[playerid] = CreatePlayerTextDraw(playerid, 323.000031, 363.377899, "~r~#FlashDM ~w~- Welcome to Flash Deathmatch!");
	PlayerTextDrawFont(playerid, infotd[playerid], TEXT_DRAW_FONT_1);
	PlayerTextDrawLetterSize(playerid, infotd[playerid], 0.309000, 1.417481);
	PlayerTextDrawAlignment(playerid, infotd[playerid], TEXT_DRAW_ALIGN_CENTER);
	PlayerTextDrawColour(playerid, infotd[playerid], 0xFFFFFFFFFFFFFFFF);
	PlayerTextDrawSetProportional(playerid, infotd[playerid], true);
	PlayerTextDrawUseBox(playerid, infotd[playerid], true);
	PlayerTextDrawBoxColour(playerid, infotd[playerid], 0x00000033);
	PlayerTextDrawTextSize(playerid, infotd[playerid], -114.333313, 300.325867);
	PlayerTextDrawSetShadow(playerid, infotd[playerid], 0);
	PlayerTextDrawSetOutline(playerid, infotd[playerid], 255);
	PlayerTextDrawBackgroundColour(playerid, infotd[playerid], 0x000000FF);
	PlayerTextDrawSetSelectable(playerid, infotd[playerid], false);

	StatsTD[playerid][0] = CreatePlayerTextDraw(playerid, 6.000000, 238.000000, "~w~FPS: 100 ~n~PING: 100 ~n~PL: 0.0%");
	PlayerTextDrawFont(playerid, StatsTD[playerid][0], TEXT_DRAW_FONT_2);
	PlayerTextDrawLetterSize(playerid, StatsTD[playerid][0], 0.150000, 1.000000);
	PlayerTextDrawTextSize(playerid, StatsTD[playerid][0], 400.000000, 17.000000);
	PlayerTextDrawSetOutline(playerid, StatsTD[playerid][0], 1);
	PlayerTextDrawSetShadow(playerid, StatsTD[playerid][0], 0);
	PlayerTextDrawAlignment(playerid, StatsTD[playerid][0], TEXT_DRAW_ALIGN_LEFT);
	PlayerTextDrawColour(playerid, StatsTD[playerid][0], -1);
	PlayerTextDrawBackgroundColour(playerid, StatsTD[playerid][0], 255);
	PlayerTextDrawBoxColour(playerid, StatsTD[playerid][0], 50);
	PlayerTextDrawUseBox(playerid, StatsTD[playerid][0], false);
	PlayerTextDrawSetProportional(playerid, StatsTD[playerid][0], true);
	PlayerTextDrawSetSelectable(playerid, StatsTD[playerid][0], false);

	StatsTD[playerid][1] = CreatePlayerTextDraw(playerid, 6.000000, 265.000000, "~w~KILLS: 0 ~n~DEATHS: 0 ~n~RATIO: 0.0");
	PlayerTextDrawFont(playerid, StatsTD[playerid][1], TEXT_DRAW_FONT_2);
	PlayerTextDrawLetterSize(playerid, StatsTD[playerid][1], 0.150000, 1.000000);
	PlayerTextDrawTextSize(playerid, StatsTD[playerid][1], 400.000000, 17.000000);
	PlayerTextDrawSetOutline(playerid, StatsTD[playerid][1], 1);
	PlayerTextDrawSetShadow(playerid, StatsTD[playerid][1], 0);
	PlayerTextDrawAlignment(playerid, StatsTD[playerid][1], TEXT_DRAW_ALIGN_LEFT);
	PlayerTextDrawColour(playerid, StatsTD[playerid][1], -1);
	PlayerTextDrawBackgroundColour(playerid, StatsTD[playerid][1], 255);
	PlayerTextDrawBoxColour(playerid, StatsTD[playerid][1], 50);
	PlayerTextDrawUseBox(playerid, StatsTD[playerid][1], false);
	PlayerTextDrawSetProportional(playerid, StatsTD[playerid][1], true);
	PlayerTextDrawSetSelectable(playerid, StatsTD[playerid][1], false);

	StatsTD[playerid][2] = CreatePlayerTextDraw(playerid, 302.000000, 2.000000, "F");
	PlayerTextDrawFont(playerid, StatsTD[playerid][2], TEXT_DRAW_FONT_1);
	PlayerTextDrawLetterSize(playerid, StatsTD[playerid][2], 0.741666, 4.299999);
	PlayerTextDrawTextSize(playerid, StatsTD[playerid][2], 400.000000, 17.000000);
	PlayerTextDrawSetOutline(playerid, StatsTD[playerid][2], 0);
	PlayerTextDrawSetShadow(playerid, StatsTD[playerid][2], 0);
	PlayerTextDrawAlignment(playerid, StatsTD[playerid][2], TEXT_DRAW_ALIGN_RIGHT);
	PlayerTextDrawColour(playerid, StatsTD[playerid][2], 887863295);
	PlayerTextDrawBackgroundColour(playerid, StatsTD[playerid][2], 255);
	PlayerTextDrawBoxColour(playerid, StatsTD[playerid][2], 50);
	PlayerTextDrawUseBox(playerid, StatsTD[playerid][2], 0);
	PlayerTextDrawSetProportional(playerid, StatsTD[playerid][2], 1);
	PlayerTextDrawSetSelectable(playerid, StatsTD[playerid][2], 0);

	StatsTD[playerid][3] = CreatePlayerTextDraw(playerid, 342.000000, 5.000000, "LASH");
	PlayerTextDrawFont(playerid, StatsTD[playerid][3], TEXT_DRAW_FONT_1);
	PlayerTextDrawLetterSize(playerid, StatsTD[playerid][3], 0.504166, 2.699999);
	PlayerTextDrawTextSize(playerid, StatsTD[playerid][3], 400.000000, 17.000000);
	PlayerTextDrawSetOutline(playerid, StatsTD[playerid][3], 0);
	PlayerTextDrawSetShadow(playerid, StatsTD[playerid][3], 0);
	PlayerTextDrawAlignment(playerid, StatsTD[playerid][3], TEXT_DRAW_ALIGN_RIGHT);
	PlayerTextDrawColour(playerid, StatsTD[playerid][3], 887863295);
	PlayerTextDrawBackgroundColour(playerid, StatsTD[playerid][3], 255);
	PlayerTextDrawBoxColour(playerid, StatsTD[playerid][3], 50);
	PlayerTextDrawUseBox(playerid, StatsTD[playerid][3], false);
	PlayerTextDrawSetProportional(playerid, StatsTD[playerid][3], true);
	PlayerTextDrawSetSelectable(playerid, StatsTD[playerid][3], false);

	StatsTD[playerid][4] = CreatePlayerTextDraw(playerid, 297.000000, 23.000000, "DEATHMATCH");
	PlayerTextDrawFont(playerid, StatsTD[playerid][4], TEXT_DRAW_FONT_1);
	PlayerTextDrawLetterSize(playerid, StatsTD[playerid][4], 0.208333, 1.500000);
	PlayerTextDrawTextSize(playerid, StatsTD[playerid][4], 400.000000, 17.000000);
	PlayerTextDrawSetOutline(playerid, StatsTD[playerid][4], 0);
	PlayerTextDrawSetShadow(playerid, StatsTD[playerid][4], 0);
	PlayerTextDrawAlignment(playerid, StatsTD[playerid][4], TEXT_DRAW_ALIGN_LEFT);
	PlayerTextDrawColour(playerid, StatsTD[playerid][4], -1);
	PlayerTextDrawBackgroundColour(playerid, StatsTD[playerid][4], 255);
	PlayerTextDrawBoxColour(playerid, StatsTD[playerid][4], 50);
	PlayerTextDrawUseBox(playerid, StatsTD[playerid][4], false);
	PlayerTextDrawSetProportional(playerid, StatsTD[playerid][4], true);
	PlayerTextDrawSetSelectable(playerid, StatsTD[playerid][4], false);

	StatsTD[playerid][5] = CreatePlayerTextDraw(playerid, 318.000000, 34.000000, "00:00:00");
	PlayerTextDrawFont(playerid, StatsTD[playerid][5], TEXT_DRAW_FONT_2);
	PlayerTextDrawLetterSize(playerid, StatsTD[playerid][5], 0.200000, 1.098999);
	PlayerTextDrawTextSize(playerid, StatsTD[playerid][5], 400.000000, 17.000000);
	PlayerTextDrawSetOutline(playerid, StatsTD[playerid][5], 0);
	PlayerTextDrawSetShadow(playerid, StatsTD[playerid][5], 0);
	PlayerTextDrawAlignment(playerid, StatsTD[playerid][5], TEXT_DRAW_ALIGN_CENTER);
	PlayerTextDrawColour(playerid, StatsTD[playerid][5], -1);
	PlayerTextDrawBackgroundColour(playerid, StatsTD[playerid][5], 255);
	PlayerTextDrawBoxColour(playerid, StatsTD[playerid][5], 50);
	PlayerTextDrawUseBox(playerid, StatsTD[playerid][5], false);
	PlayerTextDrawSetProportional(playerid, StatsTD[playerid][5], true);
	PlayerTextDrawSetSelectable(playerid, StatsTD[playerid][5], false);

	StatsTD[playerid][6] = CreatePlayerTextDraw(playerid, 318.000000, 42.000000, "00.00.0000");
	PlayerTextDrawFont(playerid, StatsTD[playerid][6], TEXT_DRAW_FONT_2);
	PlayerTextDrawLetterSize(playerid, StatsTD[playerid][6], 0.200000, 1.098999);
	PlayerTextDrawTextSize(playerid, StatsTD[playerid][6], 400.000000, 17.000000);
	PlayerTextDrawSetOutline(playerid, StatsTD[playerid][6], 0);
	PlayerTextDrawSetShadow(playerid, StatsTD[playerid][6], 0);
	PlayerTextDrawAlignment(playerid, StatsTD[playerid][6], TEXT_DRAW_ALIGN_CENTER);
	PlayerTextDrawColour(playerid, StatsTD[playerid][6], -1);
	PlayerTextDrawBackgroundColour(playerid, StatsTD[playerid][6], 255);
	PlayerTextDrawBoxColour(playerid, StatsTD[playerid][6], 50);
	PlayerTextDrawUseBox(playerid, StatsTD[playerid][6], false);
	PlayerTextDrawSetProportional(playerid, StatsTD[playerid][6], true);
	PlayerTextDrawSetSelectable(playerid, StatsTD[playerid][6], false);
    return 1;
}