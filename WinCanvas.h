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
	extern float sq(float n);  //ƽ��
	extern float sqrt(float n);//����
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
	//�������Ŀ��
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

	void point(float x, float y);	//����һ���ر�Ļ���.�����������κε�����,����strock����ɫ.ֻ�Ե�����������
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
	void _judgeVaild();                            //���������Ч�Ķ���,�������׳�exception
	//push pop(vecotr�����)
	std::vector<Renderer> _styles;	               //������Ƶ�����
	std::vector<Gdiplus::GraphicsState> _transform;//����仯������

	WP::Renderer* _renderer;                       //��Ⱦ��
	HWND _hWnd;                                    //����Ϊnull
	bool _isRelesed;                               //�ж���Դ�Ƿ��Ѿ��ͷ���
	bool _isHaveHwnd;                              //�жϳ�ʼ���ķ�ʽ(����display�ĺ���ģʽ)
	Gdiplus::Graphics* _ghsptr;                    //���ƶ���
	HDC _hdc;
	Gdiplus::Bitmap* _bmp;				                 //λͼ,��ֹ����

	//beginShape(�г�ʼ��),endShape(������).

	WP::ShapePath* _shapePath;					 //�Զ���ͼ��·��
	//fontFamily
	Gdiplus::FontFamily* _textFont;			 //������ʽ
};

class WP::Image {
public:
	float width;
	float heigth;
	Gdiplus::Bitmap* getGdiplusImage() const;
	WP::Image* cut(float partX, float partY, float partW, float partH);		          //����ͼƬ
	Image() = delete;
	Image(const WCHAR* imgPath);                                                    //����ͼƬ
	Image(const WCHAR* imgPath, float partX, float partY, float partW, float partH);//���ز�����ͼƬ
	Image(WP::Image* other, float partX, float partY, float partW, float partH);    //��������ͼƬ
	~Image();
private:
	Gdiplus::Bitmap* _img;
};

class WP::ShapePath {
public:
	std::vector<Gdiplus::Point> linePoints;  //˳ʱ��ĵ�
	std::vector<Gdiplus::Point> r_linePoints;//��ʱ��ĵ�
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
	//�Ƿ�ʹ���������,ʹ��noFill��noStroke
	bool isNoFill;
	bool isNoStroke;
	//ԭ������,����ͨ��translate�޸�
	//int originX;
	//int originY;
	//�������
	unsigned int strokeWeight;
	//��������
	//Gdiplus::FontFamily* textFont;�����Բ���push��pop,�ʲ�����Renderer
	unsigned int textSize;
	//constructor ��ʼ��Ĭ�ϵ�һ��
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
