#pragma once

#pragma pack(push, 1)

struct st_HEADER
{
	unsigned short len;
};

struct st_DRAW_PACKET
{
	int		startX;
	int		startY;
	int		endX;
	int		endY;
};

#pragma pack(pop)