
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <stdio.h>
#include <random>
#include <vector>
#include <cmath>

#include "perlin.h"

#undef main
using namespace std;



extern struct ControlButton;
extern struct ControlPoint;
extern struct Tile;

using maplist = vector<vector<Tile>>;
using buttonlist = vector<ControlButton*>;
using pointlist = vector<ControlPoint*>;


//---- ���� ----
namespace Contants {
	const int SCREEN_WIDTH = 900;
	const int SCREEN_HEIGHT = 800;
	const int W_WIDTH = 600;
	const int W_HEIGHT = 792;
	const int LINE_WIDTH = 200;
	const int LINE_START = 620;
	const int TILE_WIDTH = 12;
	const int TILE_HEIGHT = 18;
	enum Direction
	{
		up,
		down,
		left,
		right,
		upleft,
		upright,
		downleft,
		downright,
		middle,
		horizontal,
		vertical,
		null
	};
	enum Civilization {
		dwarf,
		human,
		elf,
		goblin,
		titan,
		orc,
		gnome,
	};
	enum Landform {
		ice_sea,
		sea,
		cold_forest,
		grassland,
		warm_forest,
		hot_forest,
		desert,
		ice_thorn,
		plain,
		hill,
		sakura,
		erosion,
		snow_mountain,
		mountain

	};
}

//---- ��Ⱦ ----
namespace Renderer {
	void InitRender(SDL_Renderer* render);
	void BuilderRender(SDL_Renderer* render);
	void ButtonRender(SDL_Renderer* render, const buttonlist& control_buttons);
	void ControlPointRender(SDL_Renderer* render, pointlist& control_points);
	void MapAdjust(const pointlist& control_points);
	void EmbarkRender(SDL_Renderer* render);
	void GenerationRender(SDL_Renderer* render);
	void MapRender(SDL_Texture* texture , maplist& map);
}

//---- ���๤�� ----
namespace Tool
{
	int checkClick(SDL_Point p, SDL_Rect* r);
	void PutTile(int x, int y, const SDL_Rect* rect);
	int Random(int start,int end);
	
}

//---- �¼����� ----
namespace Input {
	void InputEvent(SDL_Event event);
	void ClickControlPoint(pointlist& control_points);
	void DragControlPoint(pointlist& control_points);
	void ClickControlButton(const buttonlist& control_buttons);
	inline void ReleaseControlPoint(pointlist& control_points);
	void SelectLoaction(int mouseX, int mouseY,maplist& map);
	void RespondClick(vector<buttonlist*> control_button_list);
}

//---- ���°�ť ----
namespace Click {
	void ClickRebuild();
	void ClickPix6();
	void ClickPix12();
	void ClickPix24();
	void ClickEmbark();
	void ClickAccept();
	void ClickReturn();
	void ClickMore();
	void ClickCivil();
}

//---- ��Դ�� ----
class MainContext
{
public:
	SDL_Window* main_windows = nullptr;
	SDL_Renderer* main_render = nullptr;
	SDL_Texture* sprite_texture = nullptr;
	SDL_Event event;
	~MainContext() {
		if (main_windows) {
			SDL_DestroyWindow(main_windows);
		}
		if (main_render) {
			SDL_DestroyRenderer(main_render);
		}
		if (sprite_texture) {
			SDL_DestroyTexture(sprite_texture);
		}
	}
} MainContext;


//---- ���Ƶ� ----
struct ControlPoint
{
	double index;
	SDL_Rect& position;
	SDL_Color color = { 155, 155, 155, 255 };
	bool isDragging = false;
	SDL_Point offset = { 0,0 };

	void dragPosition(int mouseX)
	{
		position.x = mouseX - offset.x;
		if (position.x < Contants::LINE_START)
		{
			position.x = Contants::LINE_START;
		}
		else if (position.x > Contants::LINE_START + Contants::LINE_WIDTH)
		{
			position.x = Contants::LINE_START + Contants::LINE_WIDTH;
		}
		index = double(position.x - Contants::LINE_START) / Contants::LINE_WIDTH;
	}
};

//---- ��ť ----
struct ControlButton
{
	SDL_Rect& position;
	void(*click)() = nullptr;
	const char* lable;
	int tile_width = 12;
	int tile_height = 18;
	bool isclick = false;
	SDL_Color color = { 200, 200, 200, 255 };
};

//---- ��ͼ ----
struct Tile
{
	double noise;

	typedef struct
	{
		bool is_river;
		int river_direction;
		double temperature;
		double humidity;
		Contants::Landform landform;
	} states;
	states state;
	SDL_Rect maptile;
	SDL_Point texture;
	SDL_Color color;
};





class Font {
private:

	SDL_Surface* font_surface = nullptr;
	SDL_Texture* font_texture = nullptr;
public:
	TTF_Font* font = nullptr;
	~Font()
	{
		SDL_FreeSurface(font_surface);
		SDL_DestroyTexture(font_texture);
		TTF_CloseFont(font);
	}

	void FontRender(SDL_Renderer* render, const char* string, const SDL_Rect* position, SDL_Color font_color)
	{
		font_surface = TTF_RenderUTF8_Solid(font, string, { font_color.r,font_color.g,font_color.b,font_color.a });
		font_texture = SDL_CreateTextureFromSurface(render, font_surface);
		SDL_RenderCopy(render, font_texture, nullptr, position);
	}
} Font;

//---- ��ť ----
class ControlState
{
private:

	//---- ���ڵ� ----
	double cp1_index = 0.35;
	SDL_Rect cp1_rect = { Contants::LINE_START + int(cp1_index * Contants::LINE_WIDTH),195,10,15 };
	SDL_Color cp1_color = { 0, 255, 255, 255 };
	ControlPoint cp1 = { cp1_index,cp1_rect,cp1_color };

	double cp2_index = 0.6;
	SDL_Rect cp2_rect = { Contants::LINE_START + int(cp2_index * Contants::LINE_WIDTH),195,10,15 };
	SDL_Color cp2_color = { 255, 128, 0, 255 };
	ControlPoint cp2 = { cp2_index,cp2_rect,cp2_color };

	double cp3_index = 0.9;
	SDL_Rect cp3_rect = { Contants::LINE_START + int(cp3_index * Contants::LINE_WIDTH),195,10,15 };
	SDL_Color cp3_color = { 224, 224, 224, 255 };
	ControlPoint cp3 = { cp3_index,cp3_rect,cp3_color };

	//---- ��ť ----
	SDL_Rect rebuild_rect = { 620,400,100,50 };
	ControlButton rebuild = { rebuild_rect,Click::ClickRebuild,"rebuild" };

	SDL_Rect pix6_rect = { 630,100,40,25 };
	ControlButton pix6 = { pix6_rect,Click::ClickPix6,"L",8,12 };

	SDL_Rect pix12_rect = { 680,100,40,25 };
	ControlButton pix12 = { pix12_rect,Click::ClickPix12,"M",12,18,false,{255,255,255,255} };

	SDL_Rect pix24_rect = { 730,100,40,25 };
	ControlButton pix24 = { pix24_rect,Click::ClickPix24,"S",24,36 };

	SDL_Rect embark_rect = { 620,500,100,50 };
	ControlButton embark = { embark_rect,Click::ClickEmbark,"embark" };

	SDL_Rect accept_rect = { 620,500,100,50 };
	ControlButton accept = { embark_rect,Click::ClickAccept,"accept" };

	SDL_Rect return_rect = { 620,400,100,50 };
	ControlButton returnb = { return_rect,Click::ClickReturn,"return" };

	SDL_Rect landform_rect = { 620,100,100,50 };
	ControlButton landform = { landform_rect,Click::ClickMore,"landform" };

	SDL_Rect civil_rect = { 620,200,130,50 };
	ControlButton civil = { civil_rect,Click::ClickCivil,"civilization" };

public:

	pointlist control_points = { &cp1,&cp2,&cp3 };

	buttonlist control_buttons = { &rebuild,&pix6,&pix12, &pix24 ,&accept };

	buttonlist embarks_buttons = { &returnb };

	buttonlist generation_buttons = { &returnb, &embark, &landform, &civil};
	vector<buttonlist*> all_button = {&control_buttons,&generation_buttons,&embarks_buttons};

} Controller;

//---- ��״̬ ----
class MainState {
public:
	int tile_width = 12;
	int tile_height = 18;

	int mouseX = 0;
	int mouseY = 0;

	double sea_level = 0.3;
	double lands_level = 0.65;
	double mountain_level = 0.9;

	SDL_Rect line_rect = { Contants::LINE_START,200,Contants::LINE_WIDTH + 10,3 };

} MainState;

//---- ҳ�� ----
class PageOpen
{
public:
	enum  Pagelist {
		builder_page,
		generation_page,
		ready_to_embark
	};
	vector<bool> page_open = {true, false, false};
	void ClearPage() {
		page_open = { false, false, false };
	}
	void BuilderPageOpen()
	{
		ClearPage();
		page_open[builder_page] = true;
	}
	void GenerationPageOpen()
	{
		ClearPage();
		page_open[generation_page] = true;
	}
	void EmbarkPageOpen()
	{
		ClearPage();
		page_open[ready_to_embark] = true;
	}
} PageOpen ;



class Maps {
public:
	maplist naked_map = {};//���ͼ���������Ϣ

	maplist rich_map = {};//����ͼ�����������̬��Ϣ

	void MakeMap();//�������ͼ

	void ClearNakedMap()//������ͼ
	{
		naked_map.clear();
	}

	void GenerateLandForm();
	void GenerateRiver();
	vector<int> GetNext_ofRiver(vector<int> now);
	void ClearRichMap();
	void LoadTemperature(Tile& single,int latitude);
	void LodaHumidity(Tile& single,int x,int y);
	void FillLandForm(Tile& single);
	void FillRiver (maplist& map);
} Map;

void Maps::MakeMap() {
	ClearNakedMap();
	Tile tile = { 0,{false,Contants::null,0,0},NULL };
	int width = MainState.tile_width;
	int height = MainState.tile_height;
	naked_map.resize(Contants::W_WIDTH / width);
	for (int i = 0; i < Contants::W_WIDTH / width; ++i) {
		naked_map[i].resize(Contants::W_HEIGHT / height);
		for (int j = 0; j < Contants::W_HEIGHT / height; ++j) {
			tile.maptile = { i * width, j * height, width, height };
			tile.noise = OctavePerlin(double(i * width + width / 2) / 145, double(j * height + height / 2) / 145, 6, 0.5);
			naked_map[i][j] = tile;
		}
	}
}

void Maps::GenerateLandForm()  
{  
	for(int i = 0;i < rich_map.size();i++)
		for (int j = 0;j < rich_map[0].size();j++)
		{
			Tile& single = rich_map[i][j];
			//�¶��ж�
			LoadTemperature(single,j);

			FillLandForm(single);
			//ʪ���ж�
			//LodaHumidity(single,i,j);
			//�ݶ�	
		}
	GenerateRiver();
	FillRiver(rich_map);
}

void Maps::LoadTemperature(Tile& single, int latitude)
{
	double& temp = single.state.temperature;
	double relativeheight = 0;
	if (single.noise<=MainState.sea_level)	{
		relativeheight =   0.4 * single.noise / MainState.sea_level;
	}
	else if (single.noise <= MainState.lands_level)	{
		relativeheight = ( single.noise - MainState.sea_level) / (MainState.lands_level - MainState.sea_level);
	}
	else if (single.noise <= MainState.mountain_level)	{
		relativeheight = (single.noise - MainState.lands_level) / (MainState.mountain_level - MainState.lands_level);
	}
	else {
		relativeheight = (single.noise - MainState.mountain_level) / (1 - MainState.mountain_level);
	}

	temp = (3.* relativeheight + 2. * (latitude) / rich_map[0].size() ) / 5;

	if (single.noise <= MainState.sea_level) {
		temp = ( relativeheight + 2. * (latitude) / rich_map[0].size()) / 3;
	}

	if (temp <= 0.13) {
		temp = 4;//����
	}
	else if (temp <= 0.4) {
		temp = 3;
	}
	else if (temp <= 0.7) {
		temp = 2;
	}
	else if (temp <= 0.87) {
		temp = 1;
	}
	else {
		temp = 0;//����
	}
}

void Maps::FillLandForm(Tile& single)
{
	int temp = single.state.temperature;
	//���󲿷�
	if (single.noise <= MainState.sea_level)
	{
		switch (temp)
		{
		case 0:
			single.texture = { 2,11 };
			single.color = { 0,200,200,255 };
			single.state.landform = Contants::ice_sea;
			break;
		case 1://����
			if (single.noise <= MainState.sea_level * 3 / 4)
			{
				single.texture = { 2,11 };
				single.color = { 0,200,200,255 };
				single.state.landform = Contants::ice_sea;
				break;
			}
			//����
			single.texture = { 1,11 };
			single.color = { 0,255,255,255 };
			single.state.landform = Contants::ice_sea;
			break;
		default:
			break;
		}
	}
	else if(single.noise <= MainState.lands_level)
	{
		switch (temp)
		{
		case 0://��Ҷ��
			single.texture = { 4,15 };
			single.color = { 102,255,178,255 };
			single.state.landform = Contants::cold_forest;
			break;
		case 1://ƽԭ
			single.texture = { 12,14 };
			single.color = { 51,255,51,255 };
			single.state.landform = Contants::grassland;
			break;
		case 2://ɭ��
			/*single.texture = { 6,0 };
			single.color = { 0,255,0,255 };*/
			single.state.landform = Contants::warm_forest;
			break;
		case 3://�ȴ���ԭ
			/*single.texture = { 7,14 };
			single.color = { 153,153,0,255 };*/
			single.state.landform = Contants::hot_forest;
			break;
		case 4://ɳĮ
			single.texture = { 12,14 };
			single.color = { 255,255,0,255 };
			single.state.landform = Contants::desert;
			break;
		default:
			break;
		}
	}
	else if (single.noise <= MainState.mountain_level)
	{
		switch (temp)
		{
		case 0://����֮��
			single.texture = { 15,7 };
			single.color = { 0,128,128,255 };
			single.state.landform = Contants::ice_thorn;
			break;
		case 1://�ݵ�
			single.texture = { 7,14 };
			//single.color = { 51,102,0,255 };
			single.state.landform = Contants::plain;
			break;
		case 3://ӣ��ɭ��
			if (single.noise <= MainState.lands_level * 5 / 4)
			{
				single.texture = { 5,0 };
				single.color = { 255,153,204,255 };
				single.state.landform = Contants::sakura;
				break;
			}
			single.state.landform = Contants::hill;
			break;
		//case 4://ϡ����
		//	single.texture = { 7,2 };
		//	single.color = { 153,153,0,255 };
			break;
		case 4://��ʴ���
			single.texture = { 2,9 };
			single.color = { 200,200,0,255 };
			single.state.landform = Contants::erosion;
			break;
		default:
			break;
		}
	}
	else
	{
		switch (temp)
		{
		case 0://����֮��
			single.texture = { 15,7 };
			single.color = { 0,128,128,255 };
			single.state.landform = Contants::ice_thorn;
			break;
		case 1:
		case 2://��ѩɽ��
			single.texture = { 14,1 };
			single.color = { 250,250,250,255 };
			single.state.landform = Contants::snow_mountain;
			break;
		case 3://ɽ��
		case 4:
			single.texture = { 14,1 };
			single.color = { 192,192,192,255 };
			single.state.landform = Contants::mountain;
			break;
		default:
			break;
		}
	}
}

void Maps::LodaHumidity(Tile& single,int x, int y)
{
	if (single.noise <= MainState.sea_level)
	{
		single.state.humidity = 4;
		return;
	}
}

void Maps::GenerateRiver()
{
	int river_num = Tool::Random(10,30);
	vector<int> pos = { 0,0 };
	while (river_num)
	{
		//����Դͷ������
		do {
			pos[0] = Tool::Random(1, rich_map.size() - 1);
			pos[1] = Tool::Random(1, rich_map[0].size() - 1);
		} while (rich_map[pos[0]][pos[1]].noise <= MainState.sea_level || rich_map[pos[0]][pos[1]].state.temperature == 4);


		while (rich_map[pos[0]][pos[1]].noise > MainState.sea_level)
		{
			//����ֹͣ
			if (pos[0] <= 0 || pos[0] >= rich_map.size() || pos[1] <= 0 || pos[1] >= rich_map[0].size())
				break;
			//���µ���ֹͣ
			if (rich_map[pos[0]][pos[1]].state.temperature == 4)
				break;

			rich_map[pos[0]][pos[1]].state.is_river = true;


			vector<int> nextpos = GetNext_ofRiver(pos);
			//ѡ���������
			if (nextpos == pos )
			{
				//��Χû�и��͵ģ����ɺ���
				break;
			}
			else
			{
				//��Χ�и��͵ģ���������
				
				pos = nextpos;
			}
			
		} 
		river_num--;
	}

}

vector<int> Maps::GetNext_ofRiver(vector<int> now)
{
	vector<int> next = now;

	if ((now[0] + 1) < rich_map.size())
	{
		if (rich_map[now[0] + 1][now[1]].noise < rich_map[next[0]][next[1]].noise)////��1,0��ʼ
		{
			next = { now[0] + 1,now[1] };
		}
	}
	if ((now[1] + 1) < rich_map[0].size())
	{
		if (rich_map[now[0]][now[1] + 1].noise < rich_map[next[0]][next[1]].noise)//0,1
		{
			next = { now[0],now[1] + 1 };
		}
	}
	if ((now[1] - 1) >= 0)
	{
		if (rich_map[now[0]][now[1] - 1].noise < rich_map[next[0]][next[1]].noise)//0,-1
		{
			next = { now[0],now[1] - 1 };
		}
	}
	if ((now[0] - 1) >= 0)
	{
		if (rich_map[now[0] - 1][now[1]].noise < rich_map[next[0]][next[1]].noise)//-1,0
		{
			next = { now[0] - 1,now[1] };
		}
	}
	return next;
}

void Maps::FillRiver(maplist& map)
{
	for (int i = 1;i < map.size() - 1;i++)
		for (int j = 1;j < map[0].size() - 1;j++)
		{
			if (!map[i][j].state.is_river)
				continue;
			//�жϺ�������
			if (map[i - 1][j].state.is_river && map[i + 1][j].state.is_river) {
				map[i][j].state.river_direction = Contants::horizontal;//
			}
			else if (map[i][j - 1].state.is_river && map[i][j + 1].state.is_river) {
				map[i][j].state.river_direction = Contants::vertical;//
			}
			else if (map[i][j - 1].state.is_river && map[i + 1][j].state.is_river) {
				map[i][j].state.river_direction = Contants::downright;//
			}
			else if (map[i][j - 1].state.is_river && map[i - 1][j].state.is_river) {
				map[i][j].state.river_direction = Contants::downleft;//
			}
			else if (map[i][j + 1].state.is_river && map[i - 1][j].state.is_river) {
				map[i][j].state.river_direction = Contants::upleft;//
			}
			else if (map[i][j + 1].state.is_river && map[i + 1][j].state.is_river) {
				map[i][j].state.river_direction = Contants::upright;//
			}
			else {
				map[i][j].state.river_direction = Contants::middle;
			}

			//���Ϻ���
			map[i][j].color = { 0,220,220 };
			if (map[i][j].state.river_direction == Contants::horizontal) {
				map[i][j].texture = { 13,12 };
			}
			else if (map[i][j].state.river_direction == Contants::vertical)	{
				map[i][j].texture = { 10,11 };
			}
			else if (map[i][j].state.river_direction == Contants::upright) {
				map[i][j].texture = { 9,12 };
			}
			else if (map[i][j].state.river_direction == Contants::upleft) {
				map[i][j].texture = { 11,11 };
			}
			else if (map[i][j].state.river_direction == Contants::downleft) {
				map[i][j].texture = { 12,11 };
			}
			else if (map[i][j].state.river_direction == Contants::downright)	{
				map[i][j].texture = { 8,12 };
			}
			else if (map[i][j].state.river_direction == Contants::middle)
			{
				map[i][j].texture = { 15,6 };
			}


		}

}

void Maps::ClearRichMap()
{
	for (int i = 0;i < rich_map.size();i++)
		for (Tile& tile : rich_map[i])
		{
			tile.state = { false,Contants::middle,0,0 };
		}

}

int Tool::checkClick(SDL_Point p, SDL_Rect* r)
{
	if (p.x > r->x && p.x < r->x + r->w && p.y > r->y && p.y < r->y + r->h)
		return 1;
	else
		return 0;
}

inline void Tool::PutTile(int x, int y, const SDL_Rect* rect)
{
	SDL_Rect tile = { 0,0,24,36 };
	tile.y = y * 36;
	tile.x = x * 24;
	SDL_RenderCopy(MainContext.main_render, MainContext.sprite_texture, &tile, rect);

}

int Tool::Random(int start, int end)
{
	static std::default_random_engine rd;
	static std::mt19937 gener(rd());
	std::uniform_int_distribution<>u(start, end);
	return u(gener);
}


void Renderer::InitRender(SDL_Renderer* render)
{
	SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
	SDL_RenderClear(render);
	//cout << "initRender" << endl;
}

void Renderer::BuilderRender(SDL_Renderer* render)
{
	InitRender(render);
	ButtonRender(render, Controller.control_buttons);
	ControlPointRender(render, Controller.control_points);
	MapAdjust(Controller.control_points);
	MapRender(MainContext.sprite_texture,Map.naked_map);
	SDL_RenderPresent(render);
}

void Renderer::ButtonRender(SDL_Renderer* render, const buttonlist& control_buttons)
{
	SDL_Rect lable_rect = {};
	for (const ControlButton* control_button : control_buttons)
	{
		SDL_SetRenderDrawColor(render, control_button->color.r, control_button->color.g, control_button->color.b, control_button->color.a);
		SDL_RenderFillRect(render, &control_button->position);
		lable_rect.w = control_button->position.w / 5 * 4;
		lable_rect.x = control_button->position.x + control_button->position.w / 8;
		lable_rect.y = control_button->position.y;
		lable_rect.h = control_button->position.h;
		Font.FontRender(render, control_button->lable, &lable_rect, { 255,5,5,255 });
		//cout << "controlbutton" << endl;
	}
}

void Renderer::ControlPointRender(SDL_Renderer* render, pointlist& control_points)
{
	SDL_SetRenderDrawColor(render, 255, 255, 255, 255);
	SDL_RenderFillRect(render, &MainState.line_rect);

	for (const ControlPoint* control_point : control_points)
	{
		SDL_SetRenderDrawColor(render, control_point->color.r, control_point->color.g, control_point->color.b, control_point->color.a);
		SDL_RenderFillRect(render, &control_point->position);
		//cout << "controlPoint" << endl;
	}
}

void Renderer::EmbarkRender(SDL_Renderer* render)
{
	const SDL_Rect rect = { 610,200,250,50 };
	InitRender(render);
	ButtonRender(render, Controller.embarks_buttons);
	MapRender(MainContext.sprite_texture,Map.rich_map);
	Font.FontRender(MainContext.main_render, "select somewhere to embark", &rect, { 0,255,0,255 });
	SDL_RenderPresent(render);
}

void Renderer::GenerationRender(SDL_Renderer* render)
{
	InitRender(render);
	MapRender(MainContext.sprite_texture,Map.rich_map);
	ButtonRender(render, Controller.generation_buttons);
	//Renderer::RiverRender(MainContext.main_render, MainContext.sprite_texture,Map.rich_map);
	SDL_RenderPresent(render);
}



void Renderer::MapAdjust(const pointlist& control_points)
{
	double noise = 0;
	float t = 0;
	float t1 = 0;
	float t2 = 0;
	for (int i = 0;i < Map.naked_map.size();i++)
	{
		for (Tile& tile : Map.naked_map[i])
		{
			noise = tile.noise;


			//cout << noise << endl;
			if (noise <= control_points[0]->index)//����
			{

				t = noise / control_points[0]->index;
	
				tile.texture = { 7, 15 };
                tile.color = { 0, Uint8(50 + 100 * t), Uint8(150 + 100 * t) };

			}
			else if (noise <= control_points[1]->index)//�ֵ�
			{

				t = (noise - control_points[0]->index) / (control_points[1]->index - control_points[0]->index);
				t1 = 1. / 5;
				t2 = 4. / 5;
				
				tile.color = { Uint8(100 - 50 * (1 - t)), Uint8(100 + 130 * (1 - t)), 20 };
				if (t <= t1)
				{
					tile.texture = { 2, 14 };
					
				}
				else if (t <= t2)
				{
					tile.texture = { 6, 0 };
					
				}
				else
				{
					tile.texture = { 7, 14 };
					
				}

			}
			else if (noise <= control_points[2]->index)//½��
			{

				t = (noise - control_points[1]->index) / (control_points[2]->index - control_points[1]->index);
				t1 = 1. / 5;
				t2 = 4. / 5;
				tile.color = { Uint8(180 - 60 * t), Uint8(150 - 70 * t), Uint8(100 - 50 * t) };
				if (t <= t1)
				{
					tile.texture = { 14, 6 };
					
				}
				else if (t <= t2)
				{
					tile.texture = { 15, 14 };
					
				}
				else
				{
					tile.texture = { 15, 7 };
					
				}

			}
			else//ɽ
			{
				t = (noise - control_points[2]->index) / (1 - control_points[2]->index);
				t2 = 1. / 3;
				tile.color = { Uint8(150 + 90 * t), Uint8(150 + 90 * t), Uint8(150 + 90 * t) };
				if (t <= t2)
				{
					tile.texture = { 15, 7 };
					
				}
				else
				{
					tile.texture = { 14, 1 };
					
				}


			}
		}

		//cout << "mapRender" << endl;
	}
}


void Renderer::MapRender(SDL_Texture* texture,maplist& map)
{
	for (int i = 0;i < map.size();i++)
	{
		for (Tile& tile : map[i])
		{
			SDL_SetTextureColorMod(texture,tile.color.r, tile.color.g, tile.color.b);
			Tool::PutTile(tile.texture.x,tile.texture.y,&tile.maptile);
		}
	}
}

void Input::InputEvent(SDL_Event event)
{
	while (SDL_PollEvent(&event))
	{
		MainState.mouseX = event.button.x;
		MainState.mouseY = event.button.y;
		switch (event.type)
		{
		case SDL_QUIT:
			exit(0);
			break;
		case SDL_MOUSEBUTTONDOWN:

			if (PageOpen.page_open[PageOpen::builder_page])
			{
				ClickControlButton(Controller.control_buttons);
				ClickControlPoint(Controller.control_points);
				Renderer::BuilderRender(MainContext.main_render);
				break;
			}
			else if (PageOpen.page_open[PageOpen::generation_page])
			{
				ClickControlButton(Controller.generation_buttons);
				Renderer::GenerationRender(MainContext.main_render);
				break;
			}
			else if (PageOpen.page_open[PageOpen::ready_to_embark])
			{
				ClickControlButton(Controller.embarks_buttons);
				Renderer::EmbarkRender(MainContext.main_render);
				break;
			}
			break;
		case SDL_MOUSEMOTION:
			if (PageOpen.page_open[PageOpen::builder_page])
				DragControlPoint(Controller.control_points);
			if (PageOpen.page_open[PageOpen::ready_to_embark])
				SelectLoaction(MainState.mouseX, MainState.mouseY,Map.rich_map);
			break;
		case SDL_MOUSEBUTTONUP:
			if (PageOpen.page_open[PageOpen::builder_page])
				ReleaseControlPoint(Controller.control_points);
			Input::RespondClick(Controller.all_button);

		}

	}
}

void Input::DragControlPoint(pointlist& control_points)
{
	for (ControlPoint* control_point : control_points)
	{
		if (control_point->isDragging)
		{
			control_point->dragPosition(MainState.mouseX);
			Renderer::BuilderRender(MainContext.main_render);
		}
	}
}

void Input::ClickControlButton(const buttonlist& control_buttons)
{
	for (ControlButton* control_button : control_buttons)
	{
		if (Tool::checkClick({ MainState.mouseX,MainState.mouseY }, &control_button->position))
		{
			control_button->isclick = true;
			control_button->color = {155,155,155,255};
			//control_button->click();
		}
	}
}

void Input::ClickControlPoint(pointlist& control_points)
{
	for (ControlPoint* control_point : control_points)
	{
		if (Tool::checkClick({ MainState.mouseX,MainState.mouseY }, &control_point->position))
		{
			control_point->isDragging = true;
			control_point->offset.x = MainState.mouseX - control_point->position.x;
			break;
		}
	}
}

inline void Input::ReleaseControlPoint(pointlist&control_points)
{
	for (ControlPoint* control_point : control_points)
	{
		control_point->isDragging = false;
	}
}

void Input::RespondClick(vector<buttonlist*> control_button_list)
{
	for (const buttonlist* buttonl : control_button_list)
	{
		for (ControlButton* button : *buttonl)
		{
			if (button->isclick)
			{
				button->isclick = false;
				button->color = { 200,200,200,255 };
				button->click();
			}

		}
	}
}

void Input::SelectLoaction(int mouseX, int mouseY,maplist& map)
{

	int X = mouseX / MainState.tile_width;
	int Y = mouseY / MainState.tile_height;
	if (X > 0 && X < map.size() && Y > 0 && Y < map.size())
	{
		if (map[X][Y].noise <= MainState.sea_level || map[X][Y].state.is_river)
			return;

		Renderer::EmbarkRender(MainContext.main_render);
		SDL_SetTextureColorMod(MainContext.sprite_texture, 255, 255, 0);
		Tool::PutTile(8, 5, &map[X][Y].maptile);
		SDL_RenderPresent(MainContext.main_render);

	}

}

void Click::ClickRebuild()
{
	Map.MakeMap();
	Renderer::BuilderRender(MainContext.main_render);

}

void Click::ClickPix6()
{
	MainState.tile_width = Controller.control_buttons[1]->tile_width;
	MainState.tile_height = Controller.control_buttons[1]->tile_height;
	Controller.control_buttons[1]->color = { 255,255,255,255 };
	Controller.control_buttons[2]->color = { 155,155,155,255 };
	Controller.control_buttons[3]->color = { 155,155,155,255 };
	Map.MakeMap();
	Renderer::BuilderRender(MainContext.main_render);


}

void Click::ClickPix12()
{
	MainState.tile_width = Controller.control_buttons[2]->tile_width;
	MainState.tile_height = Controller.control_buttons[2]->tile_height;
	Controller.control_buttons[2]->color = { 255,255,255,255 };
	Controller.control_buttons[1]->color = { 155,155,155,255 };
	Controller.control_buttons[3]->color = { 155,155,155,255 };
	Map.MakeMap();
	Renderer::BuilderRender(MainContext.main_render);
}

void Click::ClickPix24()
{
	MainState.tile_width = Controller.control_buttons[3]->tile_width;
	MainState.tile_height = Controller.control_buttons[3]->tile_height;
	Controller.control_buttons[3]->color = { 255,255,255,255 };
	Controller.control_buttons[2]->color = { 155,155,155,255 };
	Controller.control_buttons[1]->color = { 155,155,155,255 };
	Map.MakeMap();
	Renderer::BuilderRender(MainContext.main_render);

}

void Click::ClickEmbark()
{
	PageOpen.EmbarkPageOpen();
	Renderer::EmbarkRender(MainContext.main_render);

}

void Click:: ClickAccept()
{
	MainState.sea_level = Controller.control_points[0]->index;
	MainState.lands_level = Controller.control_points[1]->index;
	MainState.mountain_level = Controller.control_points[2]->index;
	Map.rich_map = Map.naked_map;
	PageOpen.GenerationPageOpen();
	
	Renderer::GenerationRender(MainContext.main_render);

}

void Click::ClickReturn()
{
	if (PageOpen.page_open[PageOpen::generation_page])
	{
		Map.ClearRichMap();
		PageOpen.BuilderPageOpen();
		Renderer::BuilderRender(MainContext.main_render);
	}
	else if (PageOpen.page_open[PageOpen::ready_to_embark])
	{
		PageOpen.GenerationPageOpen();
		Renderer::GenerationRender(MainContext.main_render);
	}
	
}

void Click::ClickMore()
{
	Map.GenerateLandForm();
	Renderer::GenerationRender(MainContext.main_render);
	SDL_RenderPresent(MainContext.main_render);
}

void Click::ClickCivil()
{

}

static void Init()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
	TTF_Init();

	MainContext.main_windows = SDL_CreateWindow("WorldBuilder", 20, 20, Contants::SCREEN_WIDTH, Contants::SCREEN_HEIGHT, 0);
	MainContext.main_render = SDL_CreateRenderer(MainContext.main_windows, -1, 0);
	MainContext.sprite_texture = IMG_LoadTexture(MainContext.main_render, "assest/Cures.png");

	Font.font = TTF_OpenFont("assest/FSEX300.ttf", 15);

	Map.MakeMap();
	Renderer::BuilderRender(MainContext.main_render);
	SDL_RenderPresent(MainContext.main_render);
}

int main()
{
	Init();
	while (true)
	{
		Input::InputEvent(MainContext.event);
		SDL_Delay(30);
	}
	MainContext.~MainContext();
	Font.~Font();
	return 0;
}