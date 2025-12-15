#pragma once

#pragma pack(push, 1)
struct st_set_id
{
	int Type;
	int ID;
	int no_use1;
	int no_use2;
};

struct st_create_star
{
	int Type;
	int ID;
	int X;
	int Y;
};

struct st_delete_star
{
	int Type;
	int ID;
	int no_use1;
	int no_use2;
};

struct st_move_star
{
	int Type;
	int ID;
	int X;
	int Y;
};
#pragma pack(pop)