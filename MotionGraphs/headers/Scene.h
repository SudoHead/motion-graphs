#pragma once

#define GREY	0
#define RED		1
#define GREEN	2
#define BLUE	3
#define CYAN	4
#define MAGENTA	5
#define YELLOW	6
#define BLACK	7

class Scene
{
public:
	int useRGB = 1;
	int useLighting = 1;
	int useFog = 0;
	int useDB = 1;
	int useLogo = 0;
	int useQuads = 1;

	Scene();

	void drawCheckPlane(int w, int h, int evenColor, int oddColor);
	

	~Scene();

	void setColor(int c);
	void buildColormap(void);

	//vars
	float materialColor[8][4] =
	{
	  {0.8, 0.8, 0.8, 1.0},
	  {0.8, 0.0, 0.0, 1.0},
	  {0.0, 0.8, 0.0, 1.0},
	  {0.0, 0.0, 0.8, 1.0},
	  {0.0, 0.8, 0.8, 1.0},
	  {0.8, 0.0, 0.8, 1.0},
	  {0.8, 0.8, 0.0, 1.0},
	  {0.0, 0.0, 0.0, 0.6},
	};

	float lightPos[4] =
	{ 2.0, 4.0, 2.0, 1.0 };
#if 0
	float lightDir[4] =
	{ -2.0, -4.0, -2.0, 1.0 };
#endif
	float lightAmb[4] =
	{ 0.2, 0.2, 0.2, 1.0 };
	float lightDiff[4] =
	{ 0.8, 0.8, 0.8, 1.0 };
	float lightSpec[4] =
	{ 0.4, 0.4, 0.4, 1.0 };

	float groundPlane[4] =
	{ 0.0, 1.0, 0.0, 1.499 };
	float backPlane[4] =
	{ 0.0, 0.0, 1.0, 0.899 };

	float fogColor[4] =
	{ 0.0, 0.0, 0.0, 0.0 };
	float fogIndex[1] =
	{ 0.0 };

	unsigned char shadowPattern[128] =
	{
	  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,  /* 50% Grey */
	  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	  0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55
	};

	unsigned char sgiPattern[128] =
	{
	  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  /* SGI Logo */
	  0xff, 0xbd, 0xff, 0x83, 0xff, 0x5a, 0xff, 0xef,
	  0xfe, 0xdb, 0x7f, 0xef, 0xfd, 0xdb, 0xbf, 0xef,
	  0xfb, 0xdb, 0xdf, 0xef, 0xf7, 0xdb, 0xef, 0xef,
	  0xfb, 0xdb, 0xdf, 0xef, 0xfd, 0xdb, 0xbf, 0x83,
	  0xce, 0xdb, 0x73, 0xff, 0xb7, 0x5a, 0xed, 0xff,
	  0xbb, 0xdb, 0xdd, 0xc7, 0xbd, 0xdb, 0xbd, 0xbb,
	  0xbe, 0xbd, 0x7d, 0xbb, 0xbf, 0x7e, 0xfd, 0xb3,
	  0xbe, 0xe7, 0x7d, 0xbf, 0xbd, 0xdb, 0xbd, 0xbf,
	  0xbb, 0xbd, 0xdd, 0xbb, 0xb7, 0x7e, 0xed, 0xc7,
	  0xce, 0xdb, 0x73, 0xff, 0xfd, 0xdb, 0xbf, 0xff,
	  0xfb, 0xdb, 0xdf, 0x87, 0xf7, 0xdb, 0xef, 0xfb,
	  0xf7, 0xdb, 0xef, 0xfb, 0xfb, 0xdb, 0xdf, 0xfb,
	  0xfd, 0xdb, 0xbf, 0xc7, 0xfe, 0xdb, 0x7f, 0xbf,
	  0xff, 0x5a, 0xff, 0xbf, 0xff, 0xbd, 0xff, 0xc3,
	  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
	};
};