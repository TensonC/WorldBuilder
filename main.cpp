
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <stdio.h>

#include <vector>

#include "perlin.h"

#undef main
using namespace std;

extern struct ControlButton;
extern struct ControlPoint;

//---- 常量 ----
namespace Contants {
	const int SCREEN_WIDTH = 900;
	const int SCREEN_HEIGHT = 800;
	const int W_WIDTH = 600;
	const int W_HEIGHT = 792;
	const int LINE_WIDTH = 200;
	const int LINE_START = 620;
	const int TILE_WIDTH = 12;
	const int TILE_HEIGHT = 18;
} 

//---- 渲染 ----
namespace Renderer {
	void InitRender(SDL_Renderer* render);
	void ReRender(SDL_Renderer* render);
	void ButtonRender(SDL_Renderer* render, const vector<ControlButton*>& control_buttons);
	void ControlPointRender(SDL_Renderer* render,vector<ControlPoint*>& control_points);
	void MapRender(SDL_Texture* texture, const vector<ControlPoint*>& control_points);
	void EmbarkRender(SDL_Renderer* render);
}

//---- 杂类工具 ----
namespace Tool
{
	int checkClick(SDL_Point p, SDL_Rect* r);
	void PutTile(int x, int y, const SDL_Rect* rect);
}

//---- 事件处理 ----
namespace Input {
	void InputEvent(SDL_Event event);
	void ClickControlPoint(vector<ControlPoint*>& control_points);
	void DragControlPoint(vector<ControlPoint*>& control_points);
	void ClickControlButton(const vector<ControlButton*>& control_buttons);
	inline void ReleaseControlPoint(vector<ControlPoint*>& control_points);
	void SelectLoaction(int mouseX, int mouseY);
}

//---- 按下按钮 ----
namespace Click {
	void ClickRebuild();
	void ClickPix6();
	void ClickPix12();
	void ClickPix24();
	void ClickEmbark();
}

//---- 资源类 ----
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


//---- 控制点 ----
 struct ControlPoint
{
	double index;
	SDL_Rect& position;
	SDL_Color color = { 155, 155, 155, 255 };
	bool isDragging = false;
	SDL_Point offset = {0,0};

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

//---- 按钮 ----
 struct ControlButton
{
	SDL_Rect& position;
	void(*click)() = nullptr;
	const char* lable;
	int tile_width = 12;
	int tile_height = 18;
	SDL_Color color = { 155, 155, 155, 255 };
} ;

//---- 贴图 ----
typedef struct
{
	double noise;
	SDL_Rect maptile;
} Tile;



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

	void FontRender(SDL_Renderer* render,const char* string, const SDL_Rect* position,SDL_Color font_color)
	{
		font_surface = TTF_RenderUTF8_Solid(font, string, { font_color.r,font_color.g,font_color.b,font_color.a });
		font_texture = SDL_CreateTextureFromSurface(render, font_surface);
		SDL_RenderCopy(render, font_texture, nullptr, position);
	}
} Font;

//---- 按钮 ----
class ControlState
{
private:
	
	//---- 调节点 ----
	double cp1_index = 0.35;
	SDL_Rect cp1_rect = { Contants::LINE_START + int(cp1_index * Contants::LINE_WIDTH),195,10,15 };
	SDL_Color cp1_color = { 0, 255, 255, 255 };
	ControlPoint cp1 = { cp1_index,cp1_rect,cp1_color};

	double cp2_index = 0.6;
	SDL_Rect cp2_rect = { Contants::LINE_START + int(cp2_index * Contants::LINE_WIDTH),195,10,15 };
	SDL_Color cp2_color = { 255, 128, 0, 255 };
	ControlPoint cp2 = { cp2_index,cp2_rect,cp2_color };

	double cp3_index = 0.9;
	SDL_Rect cp3_rect = { Contants::LINE_START + int(cp3_index * Contants::LINE_WIDTH),195,10,15 };
	SDL_Color cp3_color = { 224, 224, 224, 255 };
	ControlPoint cp3 = { cp3_index,cp3_rect,cp3_color };

	//---- 按钮 ----
	SDL_Rect rebuild_rect = { 620,400,100,50 };
	ControlButton rebuild = {rebuild_rect,Click::ClickRebuild,"rebuild"};

	SDL_Rect pix6_rect = { 630,100,40,25 };
	ControlButton pix6 = { pix6_rect,Click::ClickPix6,"L",6,9 };

	SDL_Rect pix12_rect = { 680,100,40,25 };
	ControlButton pix12 = { pix12_rect,Click::ClickPix12,"M",12,18,{255,255,255,255} };

	SDL_Rect pix24_rect = { 730,100,40,25 };
	ControlButton pix24 = { pix24_rect,Click::ClickPix24,"S",24,36 };

	SDL_Rect embark_rect = { 620,500,100,50 };
	ControlButton embark = {embark_rect,Click::ClickEmbark,"embark"};
	
public:

	std::vector<ControlPoint*> control_points = { &cp1,&cp2,&cp3 };

	std::vector<ControlButton*> control_buttons = { &rebuild,&pix6,&pix12, &pix24 ,&embark};

	std::vector<ControlButton*> embarks = { &embark };

} Controller;

//---- 主状态 ----
class MainState {
public:
	int tile_width = 12;
	int tile_height = 18;

	int mouseX;
	int mouseY;

	bool ready_to_embark = false;

	SDL_Rect line_rect = { Contants::LINE_START,200,Contants::LINE_WIDTH + 10,3 };

} MainState ;

class Map {
public:
	vector<vector<Tile>> map = {};

	void MakeMap();//生成地图

	void ClearMap()//清空地图
	{
		map.clear();
	}

} Map;

void Map::MakeMap() {
	ClearMap();
	Tile tile = {};
	int width = MainState.tile_width;
	int height = MainState.tile_height;
	map.resize(Contants::W_WIDTH / width);
	for (int i = 0; i < Contants::W_WIDTH / width; ++i) {
		map[i].resize(Contants::W_HEIGHT / height);
		for (int j = 0; j < Contants::W_HEIGHT / height; ++j) {
			tile.maptile = { i * width, j * height, width, height };
			tile.noise = OctavePerlin(double(i * width + width / 2) / 145, double(j * height + height / 2) / 145, 6, 0.5);
			map[i][j] = tile;
		}
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


void Renderer::InitRender(SDL_Renderer* render) 
{
	SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
	SDL_RenderClear(render);
	//cout << "initRender" << endl;
}

void Renderer::ReRender(SDL_Renderer* render)
{
	InitRender(render);
	ButtonRender(render,Controller.control_buttons);
	ControlPointRender(render,Controller.control_points);
	MapRender(MainContext.sprite_texture,Controller.control_points);
	SDL_RenderPresent(render);
}

void Renderer::ButtonRender(SDL_Renderer* render, const vector<ControlButton*>& control_buttons)
{
	SDL_Rect lable_rect = {};
	for (const ControlButton* control_button : control_buttons)
	{
		SDL_SetRenderDrawColor(render,control_button->color.r, control_button->color.g, control_button->color.b, control_button->color.a);
		SDL_RenderFillRect(render, &control_button->position);
		lable_rect.w = control_button->position.w / 3 * 2;
		lable_rect.x = control_button->position.x + control_button->position.w / 6;
		lable_rect.y = control_button->position.y + control_button->position.h / 6;
		lable_rect.h = control_button->position.h;
		Font.FontRender(render, control_button->lable, &lable_rect, {255,5,5,255});
		//cout << "controlbutton" << endl;
	}
}

void Renderer::ControlPointRender(SDL_Renderer* render,vector<ControlPoint*>& control_points)
{
	SDL_SetRenderDrawColor(render,255,255,255,255);
	SDL_RenderFillRect(render,&MainState.line_rect);

	for (const ControlPoint* control_point : control_points)
	{
		SDL_SetRenderDrawColor(render, control_point->color.r, control_point->color.g, control_point->color.b, control_point->color.a);
		SDL_RenderFillRect(render, &control_point->position);
		//cout << "controlPoint" << endl;
	}
}

void Renderer::EmbarkRender(SDL_Renderer* render)
{
	const SDL_Rect rect = { 610,600,250,50 };
	InitRender(render);
	ButtonRender(render, Controller.embarks);
	//ControlPointRender(render, Controller.control_points);
	MapRender(MainContext.sprite_texture, Controller.control_points);
	Font.FontRender(MainContext.main_render, "select somewhere to embark", &rect, { 0,255,0,255 });
	SDL_RenderPresent(render);
}

void Renderer::MapRender(SDL_Texture* texture, const vector<ControlPoint*>& control_points)
{
	double noise = 0;
	float t = 0;
	float t1 = 0;
	float t2 = 0;
	for (int i = 0;i < Map.map.size();i++)
	{
		for (const Tile& tile : Map.map[i])
		{
			noise = tile.noise;


			//cout << noise << endl;
			if (noise <= control_points[0]->index)//海洋
			{

				t = noise / control_points[0]->index;
				SDL_SetTextureColorMod(texture, 0, 50 + 100 * t, 150 + 100 * t);
				Tool::PutTile(7, 15, &tile.maptile);

			}
			else if (noise <= control_points[1]->index)//林地
			{

				t = (noise - control_points[0]->index) / (control_points[1]->index - control_points[0]->index);
				t1 = 1. / 5;
				t2 = 4. / 5;
				SDL_SetTextureColorMod(texture, 100 - 50 * (1 - t), 100 + 130 * (1 - t), 20);
				if (t <= t1)
				{
					Tool::PutTile(2, 14, &tile.maptile);//低
				}
				else if (t <= t2)
				{
					Tool::PutTile(6, 0, &tile.maptile);//中
				}
				else
				{
					Tool::PutTile(7, 14, &tile.maptile);//高
				}

			}
			else if (noise <= control_points[2]->index)//陆地
			{

				t = (noise - control_points[1]->index) / (control_points[2]->index - control_points[1]->index);
				t1 = 1. / 5;
				t2 = 4. / 5;
				SDL_SetTextureColorMod(texture, 180 - 60 * t, 150 - 70 * t, 100 - 50 * t);
				if (t <= t1)
				{
					Tool::PutTile(14, 6, &tile.maptile);//低
				}
				else if (t <= t2)
				{
					Tool::PutTile(15, 14, &tile.maptile);//中
				}
				else
				{
					Tool::PutTile(15, 7, &tile.maptile);//高
				}

			}
			else//山
			{
				t = (noise - control_points[2]->index) / (1 - control_points[2]->index);
				t2 = 1. / 3;
				SDL_SetTextureColorMod(texture, 150 + 90 * t, 150 + 90 * t, 150 + 90 * t);
				if (t <= t2)
				{
					Tool::PutTile(15, 7, &tile.maptile);//低
				}
				else
				{
					Tool::PutTile(14, 1, &tile.maptile);//高
				}


			}
		}

		//cout << "mapRender" << endl;
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
			
			ClickControlButton(Controller.control_buttons);
			if(!MainState.ready_to_embark)
				ClickControlPoint(Controller.control_points);
			break;
		case SDL_MOUSEMOTION:
			if (!MainState.ready_to_embark)
				DragControlPoint(Controller.control_points);
			if (MainState.ready_to_embark)
				SelectLoaction(MainState.mouseX,MainState.mouseY);
			break;
		case SDL_MOUSEBUTTONUP:
			if (!MainState.ready_to_embark)
				ReleaseControlPoint(Controller.control_points);

		}

	}
}

void Input::DragControlPoint(vector<ControlPoint*>& control_points)
{
	for (ControlPoint* control_point : control_points)
	{
		if (control_point->isDragging)
		{
			control_point->dragPosition(MainState.mouseX);
			Renderer::ReRender(MainContext.main_render);
		}
	}
}

void Input::ClickControlButton(const vector<ControlButton*>& control_buttons)
{
	for (const ControlButton* control_button : control_buttons)
	{
		if (Tool::checkClick({MainState.mouseX,MainState.mouseY},&control_button->position))
		{
			control_button->click();
		}
	}
}

void Input::ClickControlPoint(vector<ControlPoint*>& control_points)
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

inline void Input::ReleaseControlPoint(vector<ControlPoint*>& control_points)
{
	for (ControlPoint* control_point : control_points)
	{
		control_point->isDragging = false;
	}
}

void Input::SelectLoaction(int mouseX,int mouseY)
{
	
	int X = mouseX / MainState.tile_width;
	int Y = mouseY / MainState.tile_height;

	if (MainState.tile_width == 24)
	{
		
		if (X > 0 && X < Map.map.size() && Y > 0 && Y < Map.map.size())
		{
			if (Map.map[X][Y].noise <= Controller.control_points[0]->index)
				return;

			Renderer::EmbarkRender(MainContext.main_render);
			SDL_SetTextureColorMod(MainContext.sprite_texture, 255, 255, 0);
			Tool::PutTile(8, 5, &Map.map[X][Y].maptile);
			SDL_RenderPresent(MainContext.main_render);
			
		}
		
	}
	else if (MainState.tile_width == 12)
	{
		if (X > 0 && X < Map.map.size() - 1 && Y > 0 && Y < Map.map.size() - 1)
		{
			if (Map.map[X][Y].noise <= Controller.control_points[0]->index)
				return;

			Renderer::EmbarkRender(MainContext.main_render);
			SDL_SetTextureColorMod(MainContext.sprite_texture, 255, 255, 0);
			Tool::PutTile(8, 5, &Map.map[X][Y].maptile);
			Tool::PutTile(8, 5, &Map.map[X + 1][Y].maptile);
			Tool::PutTile(8, 5, &Map.map[X][Y + 1].maptile);
			Tool::PutTile(8, 5, &Map.map[X + 1][Y + 1].maptile);
			SDL_RenderPresent(MainContext.main_render);
		}
	}
	else if (MainState.tile_width == 6)
	{
		if (X > 1 && X < Map.map.size() - 1 && Y > 1 && Y < Map.map.size() - 1)
		{
			if (Map.map[X][Y].noise > Controller.control_points[0]->index)
				return;

			Renderer::EmbarkRender(MainContext.main_render);
			SDL_SetTextureColorMod(MainContext.sprite_texture, 255, 255, 0);
			Tool::PutTile(8, 5, &Map.map[X - 1][Y].maptile);
			Tool::PutTile(8, 5, &Map.map[X][Y - 1].maptile);
			Tool::PutTile(8, 5, &Map.map[X - 1][Y - 1].maptile);
			Tool::PutTile(8, 5, &Map.map[X + 1][Y - 1].maptile);
			Tool::PutTile(8, 5, &Map.map[X - 1][Y + 1].maptile);
			Tool::PutTile(8, 5, &Map.map[X][Y].maptile);
			Tool::PutTile(8, 5, &Map.map[X + 1][Y].maptile);
			Tool::PutTile(8, 5, &Map.map[X][Y + 1].maptile);
			Tool::PutTile(8, 5, &Map.map[X + 1][Y + 1].maptile);
			SDL_RenderPresent(MainContext.main_render);
		}
	}

}

void Click::ClickRebuild()
{
	if (!MainState.ready_to_embark)
	{
		Map.MakeMap();
		Renderer::ReRender(MainContext.main_render);
	}
	
}

void Click::ClickPix6()
{
	if (!MainState.ready_to_embark)
	{
		MainState.tile_width = Controller.control_buttons[1]->tile_width;
		MainState.tile_height = Controller.control_buttons[1]->tile_height;
		Controller.control_buttons[1]->color = { 255,255,255,255 };
		Controller.control_buttons[2]->color = { 155,155,155,255 };
		Controller.control_buttons[3]->color = { 155,155,155,255 };
		Map.MakeMap();
		Renderer::ReRender(MainContext.main_render);
	}
	

}

void Click::ClickPix12()
{
	if (!MainState.ready_to_embark)
	{
		MainState.tile_width = Controller.control_buttons[2]->tile_width;
		MainState.tile_height = Controller.control_buttons[2]->tile_height;
		Controller.control_buttons[2]->color = { 255,255,255,255 };
		Controller.control_buttons[1]->color = { 155,155,155,255 };
		Controller.control_buttons[3]->color = { 155,155,155,255 };
		Map.MakeMap();
		Renderer::ReRender(MainContext.main_render);
	}
	
}

void Click::ClickPix24()
{
	if (!MainState.ready_to_embark)
	{
		MainState.tile_width = Controller.control_buttons[3]->tile_width;
		MainState.tile_height = Controller.control_buttons[3]->tile_height;
		Controller.control_buttons[3]->color = { 255,255,255,255 };
		Controller.control_buttons[2]->color = { 155,155,155,255 };
		Controller.control_buttons[1]->color = { 155,155,155,255 };
		Map.MakeMap();
		Renderer::ReRender(MainContext.main_render);
	}
	
}

void Click::ClickEmbark()
{
	MainState.ready_to_embark = !MainState.ready_to_embark;
	MainState.ready_to_embark ? Controller.control_buttons[4]->color = { 255,255,255,255 } :
		Controller.control_buttons[4]->color = { 155,155,155,255 };
	if (MainState.ready_to_embark)
	{
		Renderer::EmbarkRender(MainContext.main_render);
	}
	else
	{
		Renderer::ReRender(MainContext.main_render);
	}
	
}

static void Init()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
	TTF_Init();

	MainContext.main_windows = SDL_CreateWindow("WorldBuilder", 20, 20, Contants::SCREEN_WIDTH, Contants::SCREEN_HEIGHT, 0);
	MainContext.main_render = SDL_CreateRenderer(MainContext.main_windows, -1, 0);
	MainContext.sprite_texture = IMG_LoadTexture(MainContext.main_render, "assest/Cures.png");

	Font.font = TTF_OpenFont("assest/Silver.ttf",20);
	
	Map.MakeMap();
	Renderer::ReRender(MainContext.main_render);
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