#define _USE_MATH_DEFINES
#include <iostream>
#include <cmath>
#include <random>

using namespace std;
//哈希数组
int p[] = {
	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
};
int seed;//地图种子
int new_p[512];
void make_new_p()
{
	std::default_random_engine e;
	std::uniform_int_distribution<int> u(1000, 10000);
	e.seed(time(0));
	seed = u(e);

	for (int i = 0; i < 512; i++)
	{
		new_p[i] = p[(i * seed) & 255];
	}
}
//二维向量定义
struct V2 {
	double x, y;
	V2(double x = 0, double y = 0) : x(x), y(y) {}//初始化为0
};
const V2 gradient[] = {
	{1,1}, {1,-1}, {-1,1}, {-1,-1},
	{M_SQRT2,0}, {-M_SQRT2,0}, {0,M_SQRT2}, {0,-M_SQRT2}
};


//缓和曲线
double fade(double t)
{
	//return t * t * (3 - 2 * t);
	return t * t * t * (t * (t * 6 - 15) + 10);
}
//计算梯度和点积
/*
double dot(int hash, double x, double y)
{
	switch (hash & 7)
	{
	case 0:
		return x + y;
	case 1:
		return -x + y;
	case 2:
		return x - y;
	case 3:
		return -x - y;
	case 4:
		return sqrt(2) * y;
	case 5:
		return sqrt(2) * x;
	case 6:
		return -sqrt(2) * y;
	case 7:
		return -sqrt(2) * x;
	default:
		break;
	}
}*/
double dot(int hash, double x, double y)
{
	float z = 0.34567;
	switch (hash & 0xF)
	{
	case 0x0: return  x + y;
	case 0x1: return -x + y;
	case 0x2: return  x - y;
	case 0x3: return -x - y;
	case 0x4: return  x + z;
	case 0x5: return -x + z;
	case 0x6: return  x - z;
	case 0x7: return -x - z;
	case 0x8: return  y + z;
	case 0x9: return -y + z;
	case 0xA: return  y - z;
	case 0xB: return -y - z;
	case 0xC: return  y + x;
	case 0xD: return -y + z;
	case 0xE: return  y - x;
	case 0xF: return -y - z;
	default: return 0; // never happens
	}
}


double multdot(const V2& a, const V2& b)
{
	return a.x * b.x + a.y + b.y;

}

V2 getGradient(int x, int y)
{
	int hash = (x + y * 57) % 8;
	return gradient[hash];
}

//取整
int flashfloor(double x)
{
	return x > 0 ? (int)x : (int)x - 1;
}

double lerp(double x, double a, double b)
{
	return a + x * (b - a);
}

//计算噪声
double perlin(double x, double y)
{
	std::default_random_engine e;
	std::uniform_real_distribution<double> un(-1, 1);
	e.seed(time(0));

	x = fmodl(x, 256) + un(e);//添加随机扰动
	y = fmodl(y, 256) + un(e);

	make_new_p();

	int xn = flashfloor(x) & 255;
	int yn = flashfloor(y) & 255;
	double xf = x - flashfloor(x);
	double yf = y - flashfloor(y);

	V2 w1 = { x - xn,y - yn };

	double u = fade(xf);//缓和后的x
	double v = fade(yf);//缓和后的y

	int p1 = new_p[new_p[xn] + yn];//左下角的哈希
	int p2 = new_p[new_p[(xn + 1)] + yn];//右下角的哈希
	int p3 = new_p[new_p[xn] + yn + 1];//左上角的哈希
	int p4 = new_p[new_p[xn + 1] + yn + 1];//右上角的哈希

	double v1 = lerp(u, dot(p1, xf, yf), dot(p2, xf - 1, yf));//下方在x方向上的插值(后两个参数是距离向量)
	double v2 = lerp(u, dot(p3, xf, yf - 1), dot(p4, xf - 1, yf - 1));//上方在x方向上的插值
	double v3 = lerp(v, v1, v2);//在y方向上的插值
	return (v3 / sqrt(2) + 1) / 2;
}

double perlin_pro(double x, double y)
{

	//	x = fmodl(x, 256) + un(e);//添加随机扰动
	//	y = fmodl(y, 256) + un(e);
		//x = x + un(e);
		//y = y + un(e);

	int xn = flashfloor(x) & 255;
	int yn = flashfloor(y) & 255;

	double xf = x - flashfloor(x);
	double yf = y - flashfloor(y);

	double u = fade(xf);//缓和
	double v = fade(yf);

	V2 topRight(xf - 1.0, yf - 1.0);
	V2 topLeft(xf, yf - 1.0);
	V2 bottomRight(xf - 1.0, yf);
	V2 bottomLeft(xf, yf);//四个距离向量

	double v1 = lerp(u, multdot(bottomLeft, getGradient(xn, yn)), multdot(bottomRight, getGradient(xn + 1, yn)));
	double v2 = lerp(u, multdot(topLeft, getGradient(xn, yn + 1)), multdot(topRight, getGradient(xn + 1, yn + 1)));
	double v3 = lerp(v, v1, v2);
	return (v3 / sqrt(2) + 1) / 2;
}

double OctavePerlin(double x, double y, int octaves, double persistence)
{
	double total = 0;
	double frequency = 1;
	double amplitude = 1;
	double maxValue = 0;
	for (int i = 0; i < octaves; i++)
	{
		total += perlin(x * frequency, y * frequency) * amplitude;
		maxValue += amplitude;
		amplitude *= persistence;
		frequency *= 2;
	}
	double result = total / maxValue;
	result = sin(((result - 0.3) / 0.4)*M_PI/2);//分布处理
	return  result;
}
