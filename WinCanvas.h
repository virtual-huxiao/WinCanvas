#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <exception>
#include <vector>
#pragma comment (lib,"Gdiplus.lib")

namespace WP {	//Win pack
	//class WinCanvas
	class WinCanvas;
	class ShapePath;
	class Image;
	//Renderer
	struct Renderer;

	//WC_expection
	class WC_expection;

	//const number
	constexpr HWND FULL_SCREEN = nullptr;
	constexpr bool CLOSE = true;

	//static function
	extern WP::Image* loadImage(const WCHAR* path);
	extern WP::Image* loadImage(const WCHAR* path, float partX, float partY, float partW, float partH);
	//static function math
	extern int abs(int num);
	extern float abs(float num);
	extern double abs(double num);
	extern float constrain(float num, float min, float max);
	extern float dist(float x1, float y1, float x2, float y2);
	extern float lerp(float start, float stop, float amt);
	extern float log(float n);
	extern float mag(float x, float y);/*=dist(0,0,x,y)*/
	extern float map(float value, float start1, float stop1, float start2, float stop2);
	extern float norm(float value, float start, float stop);/*=map(value,start,stop,0,1)*/
	extern float pow(float n, float e);
	extern float sq(float n);  //平方
	extern float sqrt(float n);//开方
	extern float random();
	extern float random(float min, float max);
	extern float sin(float angle);
	extern float cos(float angle);
	extern float tan(float angle);
	//static function time
	extern unsigned int  year();       //1601 - 30827
	extern unsigned int  month();      //1 - 12
	extern unsigned int  week();       //0(Sunday) - 6(Staurday)
	extern unsigned int  day();        //1-31
	extern unsigned int  hour();       //0-23
	extern unsigned int  minute();     //0-59
	extern unsigned int  second();     //0-59
	extern unsigned int  millis();     //0-999
	//static function string
	extern char* wchar2char(const WCHAR* wchar);
	extern WCHAR* char2wchar(const char* cchar);
}

class WP::WinCanvas {
public:
	//绘制区的宽高
	unsigned int width;
	unsigned int height;

	WinCanvas() = delete;
	explicit WinCanvas(const HWND hWnd);
	WinCanvas(unsigned int width, unsigned int height);
	~WinCanvas();
	void close();
	void display();
	void display(HWND hWnd);

	WP::WinCanvas* createGraphics(unsigned int width, unsigned int height);

	void fill(byte rgb, byte a = 255);
	void fill(byte r, byte g, byte b, byte a = 255);
	void noFill();

	void stroke(byte rgb, byte a = 255);
	void stroke(byte r, byte g, byte b, byte a = 255);
	void noStroke();

	void strokeWeight(unsigned int weight);

	void translate(float offX, float offY);
	void scale(float xy);
	void scale(float x, float y);
	void rotate(float angle);

	void push();
	void pop();

	void background(byte rgb, byte a = 255);
	void background(byte r, byte g, byte b, byte a = 255);

	void point(float x, float y);	//这是一个特别的绘制.它不局限在任何的属性,除了strock的颜色.只对单个像素作用
	void arc(float x, float y, float w, float h, float startAngle, float stopAngle);
	void quad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
	void bezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
	void triangle(float x1, float y1, float x2, float y2, float x3, float y3);
	void line(float beginX, float beginY, float endX, float endY);
	void rect(float x, float y, float width, float height);
	void ellipse(float x, float y, float d);
	void ellipse(float x, float y, float xd, float yd);

	void image(WP::Image* img, float x, float y);
	void image(WP::Image* img, float x, float y, float w, float h);
	void image(WP::WinCanvas* other, float x, float y);
	void image(WP::WinCanvas* other, float x, float y, float w, float h);

	Gdiplus::GraphicsPath* beginShape();
	void vertex(float x, float y);
	//void curveVertex(float x, float y);
	void endShape(bool isClose = false);
	Gdiplus::GraphicsPath* beginContour();
	void endContour(bool isClose = false);

	void textFont(const WCHAR* family);
	void textWidth(const WCHAR* str);
	void textSize(unsigned int size);
	void text(const WCHAR* str, float x, float y);
	//void text(const const WCHAR* str, float x, float y, float w, float h);

	//extension
	Gdiplus::Pen* getNowPen()const;
	Gdiplus::SolidBrush* getNowSolidBrush() const;

private:
	void _judgeVaild();                            //如果存在无效的对象,则立即抛出exception
	//push pop(vecotr是深拷贝)
	std::vector<Renderer> _styles;	               //负责绘制的属性
	std::vector<Gdiplus::GraphicsState> _transform;//负责变化的属性

	WP::Renderer* _renderer;                       //渲染器
	HWND _hWnd;                                    //可以为null
	bool _isRelesed;                               //判断资源是否已经释放了
	bool _isHaveHwnd;                              //判断初始化的方式(决定display的函数模式)
	Gdiplus::Graphics* _ghsptr;                    //绘制对象
	HDC _hdc;
	Gdiplus::Bitmap* _bmp;				                 //位图,防止闪屏

	//beginShape(中初始化),endShape(中销毁).

	WP::ShapePath* _shapePath;					 //自定义图形路径
	//fontFamily
	Gdiplus::FontFamily* _textFont;			 //字体样式
};

class WP::Image {
public:
	float width;
	float heigth;
	Gdiplus::Bitmap* getGdiplusImage() const;
	WP::Image* cut(float partX, float partY, float partW, float partH);		          //剪切图片
	Image() = delete;
	Image(const WCHAR* imgPath);                                                    //加载图片
	Image(const WCHAR* imgPath, float partX, float partY, float partW, float partH);//加载并剪切图片
	Image(WP::Image* other, float partX, float partY, float partW, float partH);    //剪切已有图片
	~Image();
private:
	Gdiplus::Bitmap* _img;
};

class WP::ShapePath {
public:
	std::vector<Gdiplus::Point> linePoints;  //顺时针的点
	std::vector<Gdiplus::Point> r_linePoints;//逆时针的点
	ShapePath() = delete;
	ShapePath(Gdiplus::GraphicsPath* shape_path, bool is_clockwise);
	~ShapePath();

	Gdiplus::GraphicsPath* getGraphicsPath();
	void setIsClockwise(bool isClockwise);
	bool getIsClockwise() const;
private:
	Gdiplus::GraphicsPath* _shapePath;
	bool _isClockwise;
};

struct WP::Renderer {
	//Gdiplus::Color(r,g,b)|(a,r,g,b)
	Gdiplus::Color fillColor;
	Gdiplus::Color strokeColor;
	//是否使用填充和描边,使用noFill和noStroke
	bool isNoFill;
	bool isNoStroke;
	//原点坐标,可以通过translate修改
	//int originX;
	//int originY;
	//线条宽度
	unsigned int strokeWeight;
	//字体属性
	//Gdiplus::FontFamily* textFont;此属性不可push和pop,故不属于Renderer
	unsigned int textSize;
	//constructor 初始化默认的一切
	Renderer();
	Renderer(const Renderer& other) = default;
	//WP::Renderer& operator=(const WP::Renderer& other) = default;
	~Renderer();
};

class WP::WC_expection :public std::exception {
public:
	WC_expection() = default;
	WC_expection(const char* msg) noexcept;
	WC_expection(const WC_expection& other) noexcept;
	~WC_expection() = default;
	virtual const char* what() const override;
private:
	const char* _msg = nullptr;
};
