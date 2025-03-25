#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
//#include <SDL2/SDL_mixer.h>
//#include <SDL2/SDL_ttf.h>
#include <vector>
#include "perlin.h"
using namespace std;
#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 800
#define W_WIDTH 600
#define W_HEIGHT 792
#define LINE_WIDTH 200

#undef main
typedef struct
{
	double index;
	SDL_Rect rect;
	bool isDragging;
	SDL_Point offest;
} ControlPoint;
typedef struct {
	SDL_Rect* rect;
	bool isClick;
	int t_w;
	int t_h;
} ControlButton;
typedef struct
{
	double noise;
	SDL_Rect maptile;
} Tile;

int tile_width = 12;
int tile_height = 18;
SDL_Window* win01 = NULL;
SDL_Renderer* render = NULL;
SDL_Texture* texture = NULL;
SDL_Event event;
ControlPoint cp1 = { 0.35, 630 + cp1.index * 150,195,5,15, false, {0,0} };
ControlPoint cp2 = { 0.6, 630 + cp2.index * 150,195,5,15, false, {0,0} };
ControlPoint cp3 = { 0.9, 630 + cp3.index * 150,195,5,15, false, {0,0} };
SDL_Rect Rebuild = { 620,400,100,50 };
SDL_Rect Pix6 = { 630,100,40,25 };
SDL_Rect Pix12 = { 680,100,40,25 };
SDL_Rect Pix24 = { 730,100,40,25 };
ControlButton cRebuild = { &Rebuild,false };

ControlButton cPix6 = { &Pix6, false, 6, 8 };
ControlButton cPix12 = { &Pix12, true, 12, 18 };
ControlButton cPix24 = { &Pix24, false, 24 ,36 };

vector<vector<Tile>> map = {};
ControlPoint* cps[3] = { &cp1, &cp2, &cp3 };
ControlButton* cbs[4] = { &cRebuild, &cPix6, &cPix12, &cPix24 };

int mouse_x;
int mouse_y;

class Icon
{
public:
	SDL_Rect Rebuild_icon = { Rebuild.x + 10,Rebuild.y + Rebuild.h / 2 - 18,24,36 };
	SDL_Rect Pix6_icon = { Pix6.x + 10,Pix6.y + Pix6.h / 2 - 9,24,Pix6.h };
	SDL_Rect Pix12_icon = { Pix12.x + 10,Pix12.y + Pix12.h / 2 - 9,24,Pix12.h };
	SDL_Rect Pix24_icon = { Pix24.x + 10,Pix24.y + Pix24.h / 2 - 9,24,Pix24.h };
	SDL_Rect Line = { 630,200,LINE_WIDTH + 10,3 };
};

Icon icon;




void PutTile(int x, int y, const SDL_Rect* rect)
{
	SDL_Rect tile = { 0,0,24,36 };
	tile.y = y * 36;
	tile.x = x * 24;
	SDL_RenderCopy(render, texture, &tile, rect);

}
void ControlPointRender()
{
	cp1.rect = { 630 + int(cp1.index * LINE_WIDTH),195,10,15 };
	cp2.rect = { 630 + int(cp2.index * LINE_WIDTH),195,10,15 };
	cp3.rect = { 630 + int(cp3.index * LINE_WIDTH),195,10,15 };

	SDL_SetRenderDrawColor(render, 255, 255, 255, 255);
	SDL_RenderFillRect(render, &icon.Line);

	SDL_SetRenderDrawColor(render, 0, 255, 255, 255);
	SDL_RenderFillRect(render, &cp1.rect);

	SDL_SetRenderDrawColor(render, 255, 128, 0, 255);
	SDL_RenderFillRect(render, &cp2.rect);

	SDL_SetRenderDrawColor(render, 224, 224, 224, 255);
	SDL_RenderFillRect(render, &cp3.rect);

	//cout << "ControlerRender" << endl;
}
void SelectRectRender()
{

	SDL_SetRenderDrawColor(render, 255, 255, 255, 255);
	SDL_RenderFillRect(render, &Rebuild);

	!cbs[2]->isClick ? SDL_SetRenderDrawColor(render, 155, 155, 155, 255) : SDL_SetRenderDrawColor(render, 255, 255, 255, 255);
	SDL_RenderFillRect(render, &Pix12);

	!cbs[3]->isClick ? SDL_SetRenderDrawColor(render, 155, 155, 155, 255) : SDL_SetRenderDrawColor(render, 255, 255, 255, 255);
	SDL_RenderFillRect(render, &Pix24);

	!cbs[1]->isClick ? SDL_SetRenderDrawColor(render, 155, 155, 155, 255) : SDL_SetRenderDrawColor(render, 255, 255, 255, 255);
	SDL_RenderFillRect(render, &Pix6);

	SDL_SetTextureColorMod(texture, 150, 50, 150);
	PutTile(2, 5, &icon.Rebuild_icon);
	PutTile(12, 4, &icon.Pix6_icon);
	PutTile(13, 4, &icon.Pix12_icon);
	PutTile(3, 5, &icon.Pix24_icon);
}

//void MakeMap()
//{
//	Tile tile;
//	for (int i = title_width / 2;i <= W_WIDTH - title_width / 2;i += title_width)
//		for (int j = title_height / 2;j <= W_HEIGHT - title_height / 2;j += title_height)
//		{
//			tile.maptile = { i - title_width / 2,j - title_height / 2,title_width,title_height };
//			tile.noise = OctavePerlin(double(i) / 145, double(j) / 145, 6, 0.5);
//			map.push_back(tile);
//		}
//}
void MakeMap() {
	Tile tile = {};
	map.resize(W_WIDTH / tile_width);
	for (int i = 0; i < W_WIDTH / tile_width; ++i) {
		map[i].resize(W_HEIGHT / tile_height);
		for (int j = 0; j < W_HEIGHT / tile_height; ++j) {
			tile.maptile = { i * tile_width, j * tile_height, tile_width, tile_height };
			tile.noise = OctavePerlin(double(i * tile_width + tile_width / 2) / 145, double(j * tile_height + tile_height / 2) / 145, 6, 0.5);
			map[i][j] = tile;
		}
	}
}

void MapRender()
{
	double noise;
	float t;
	float t1;
	float t2;
	for (int i = 0;i < map.size();i++)
	{
		for (const Tile& tile : map[i])
		{
			noise = tile.noise;
	

			//cout << noise << endl;
			if (noise <= cp1.index)//海洋
			{

				t = noise / cp1.index;
				SDL_SetTextureColorMod(texture, 0, 50 + 100 * t, 150 + 100 * t);
				PutTile(7, 15, &tile.maptile);

			}
			else if (noise <= cp2.index)//林地
			{

				t = (noise - cp1.index) / (cp2.index - cp1.index);
				t1 = double(1) / 5;
				t2 = double(4) / 5;
				SDL_SetTextureColorMod(texture, 100 - 50 * (1 - t), 50 + 130 * (1 - t), 20);
				if (t <= t1)
				{
					PutTile(2, 14, &tile.maptile);//
				}
				else if (t <= t2)
				{
					PutTile(6, 0, &tile.maptile);//
				}
				else
				{
					PutTile(7, 14, &tile.maptile);//
				}

			}
			else if (noise <= cp3.index)//陆地
			{

				t = (noise - cp2.index) / (cp3.index - cp2.index);
				t1 = double(1) / 5;
				t2 = double(4) / 5;
				SDL_SetTextureColorMod(texture, 180 - 60 * t, 150 - 70 * t, 100 - 50 * t);
				if (t <= t1)
				{
					PutTile(14, 6, &tile.maptile);//
				}
				else if (t <= t2)
				{
					PutTile(15, 14, &tile.maptile);//
				}
				else
				{
					PutTile(15, 7, &tile.maptile);//
				}

			}
			else//山
			{
				t = (noise - cp3.index) / (1 - cp3.index);
				t2 = double(1) / 3;
				SDL_SetTextureColorMod(texture, 150 + 90 * t, 150 + 90 * t, 150 + 90 * t);
				if (t <= t2)
				{
					PutTile(15, 7, &tile.maptile);//
				}
				else
				{
					PutTile(14, 1, &tile.maptile);//
				}


			}
	}
	

	}

}

void Render()
{
	SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
	SDL_RenderClear(render);//


	
	ControlPointRender();
	SelectRectRender();
	MapRender();
	
	SDL_RenderPresent(render);
}
void Init()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
	//TTF_Init();
	win01 = SDL_CreateWindow("SDLlearning1", 20, 20, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	render = SDL_CreateRenderer(win01, -1, 0);
	texture = IMG_LoadTexture(render, "assest/Cures.png");
	//font = TTF_OpenFont("D:/Vitual Studio/SDLlearning1/SDLlearning1/assest/Silver.ttf", 20);
	MakeMap();
	Render();
}

void Quit()
{
	SDL_DestroyRenderer(render);
	SDL_DestroyTexture(texture);
	SDL_DestroyWindow(win01);

	SDL_Quit();
}

int checkClick(SDL_Point p, SDL_Rect* r)
{
	if (p.x > r->x && p.x < r->x + r->w && p.y > r->y && p.y < r->y + r->h)
		return 1;
	else
		return 0;
}
void Input()
{
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			exit(0);
			break;
		case SDL_MOUSEBUTTONDOWN:
			mouse_x = event.button.x;
			mouse_y = event.button.y;
			for (int i = 0;i < 3;i++)
			{
				if (checkClick({ mouse_x,mouse_y }, &cps[i]->rect))
				{
					cps[i]->isDragging = true;
					//cout << "click" << i << endl;
					cps[i]->offest.x = mouse_x - cps[i]->rect.x;
					break;
				}
			}

			for (int i = 1;i < 4;i++)
			{
				if (checkClick({ mouse_x,mouse_y }, cbs[0]->rect))
				{
					//cout << "click rebuild" << endl;
					map.clear();
					MakeMap();
					Render();
					break;
				}
				if (checkClick({ mouse_x,mouse_y }, cbs[i]->rect))
				{
					tile_width = cbs[i]->t_w;
					tile_height = cbs[i]->t_h;
					cbs[1]->isClick = false;
					cbs[2]->isClick = false;
					cbs[3]->isClick = false;
					cbs[i]->isClick = true;
					map.clear();
					MakeMap();
					Render();
					break;
				}

			}

			break;
		case SDL_MOUSEMOTION:
			for (int i = 0;i < 3;i++)
			{
				if (cps[i]->isDragging)
				{
					cps[i]->rect.x = event.motion.x - cps[i]->offest.x;
					cps[i]->index = double(cps[i]->rect.x - 630) / LINE_WIDTH;
					if (cps[i]->rect.x < 630)
					{
						cps[i]->rect.x = 630;
						cps[i]->index = 0;
					}
					if (cps[i]->rect.x > 630 + LINE_WIDTH)
					{
						cps[i]->rect.x = 630 + LINE_WIDTH;
						cps[i]->index = 1;
					}
					Render();
				}

			}
			
			break;
		case SDL_MOUSEBUTTONUP:
			for (int i = 0;i < 3;i++)
			{
				cps[i]->isDragging = false;
			}
			//Render();

			break;
		default:
			break;
		}

	}
}

int main()
{
	Init();
	while (true)
	{
		Input();
		//Render();
		SDL_Delay(30);
	}
	Quit();
	return 0;
}
