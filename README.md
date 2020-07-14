[TOC]

------

# 1.WinCanvas是什么

​	这只是一个简单的对`Windows GDI+`绘制方式的一个封装.封装为[P5.js](https://p5js.org/zh-Hans/)的绘制方式.

​	方便使用Windows API去制作[openProcessing](https://www.openprocessing.org/)上的内容.

# 2.基本的使用

## 2.1 整体的框架

​	与P5.js绘制方式类似,主要集中在setup和draw函数的内容中.

​	在一个windows API实现的窗口中如下:

```C++
#include<Windows.h>
#include<tchar.h>
#include<string>
#include<vector>
#include "WinCanvas.h"
#define PRINT_TO_VS_CONSOLE	//使用此宏,可以在vs编辑器下打印错误信息

constexpr int WIDTH = 800;
constexpr int HEIGHT = 800;

using namespace WP;
void setup() {//只执行一次
}

void draw(HWND hwnd) {//每次触发绘制时执行
}

//1.注册窗口
LRESULT CALLBACK WinMessageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);//回调函数(系统调用)

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd) {
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	// 加载 GDI+.
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	try {
		setup();
	} catch (const std::exception& e) {
#ifdef PRINT_TO_VS_CONSOLE
		//向visual stuido输出错误信息
		char chInput[1024];
		sprintf_s(chInput, 1024, "error:%s\n", e.what());
		OutputDebugStringA(chInput);
#endif
		exit(0);
	}

	WNDCLASSEX wndclassex;

	wndclassex.cbSize = sizeof(WNDCLASSEX);					  				                       //窗口类的内存大小
	wndclassex.style = 0;									  									                       //样式
	wndclassex.lpfnWndProc = WinMessageProc;					                               //定义窗口处理函数
	wndclassex.cbClsExtra = 0;									  						                       //窗口的类扩展
	wndclassex.cbWndExtra = 0;									  									                 //窗口实例无扩展
	wndclassex.hInstance = hInstance;							  							                   //当前语句把柄
	wndclassex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);	                         //窗口最小化图标(为缺省)
	wndclassex.hCursor = LoadCursor(nullptr, IDC_ARROW);		  					             //窗口的箭头光标
	wndclassex.hbrBackground = (HBRUSH)(GetStockObject(WHITE_BRUSH));                //窗口的背景为白色
	wndclassex.lpszMenuName = nullptr;		                                           //无菜单
	wndclassex.lpszClassName = _T("ClassName");						  	                       //窗口类名
	wndclassex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);	                       //窗口菜单栏图标

	if (!RegisterClassEx(&wndclassex)) {//注册窗口(0为失败)
		MessageBox(nullptr, _T("窗口注册失败"), _T("出错"), MB_OK);
	}
	//设置窗口居中的运算
	int scrWidth = ::GetSystemMetrics(SM_CXSCREEN);
	int scrHeight = ::GetSystemMetrics(SM_CYSCREEN);
	int windowX = (scrWidth - WIDTH) / 2;
	int windowY = (scrHeight - HEIGHT) / 2;

	//2.创建窗口
	HWND newWind = CreateWindowEx(	                        //创建窗口函数,返回为null为失败
		0L,						  	  						                      //是否需要扩展样式,0L不需要
		_T("ClassName"),		  	  			                      //注册窗口类的名字
		_T("WindowTitle"),		  	  		                      //窗口的标题
		WS_OVERLAPPEDWINDOW,	  	  		                      //窗口的样式,一半主窗口都是这个
		windowX, windowY, WIDTH, HEIGHT,			  			        //x,y,width,heigh
		nullptr,				  	  					                      //父窗口
		nullptr,				  	  					                      //菜单的句柄
		hInstance,				  	  				                      //这个是WinMain的第一个参数
		nullptr					  	  					                      //额外数据,不设置,所以为null
	);
	if (!newWind) {					  				                      //注册窗口(0为失败)
		MessageBox(nullptr, _T("窗口注册失败"), _T("出错"), MB_OK);
	}
	ShowWindow(newWind, nShowCmd);
	UpdateWindow(newWind);

	//3.消息循环
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	//关闭 GDI+
	Gdiplus::GdiplusShutdown(gdiplusToken);
	return 0;
}

//4.处理消息
LRESULT CALLBACK WinMessageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_DESTROY: {
		PostQuitMessage(0);
		break;
	}
	case WM_PAINT: {
		try {
			draw(hwnd);
		} catch (const std::exception& e) {
#ifdef PRINT_TO_VS_CONSOLE
			//向visual stuido输出错误信息
			char chInput[1024];
			sprintf_s(chInput, 1024, "error:%s\n", e.what());
			OutputDebugStringA(chInput);
#endif
			exit(0);
		}
		break;
	}
	case WM_ERASEBKGND: {
		return 1;//取消窗口的重置背景
	}
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}
```

​	没办法,创建一个自己的窗口就是那么的多代码....

​	**不过我们之后的所以行为都不会再修改setup和draw之外的内容.这样更好的去专注绘制而不是窗口.**

## 2.2 基本绘制

​	绘制的方式和P5唯一的不同就是WinCanvas需要使用`display()`去将内容绘制到窗口.

​	绘制的方式依旧是有两种:一次绘制,无数次显示; 和一次绘制,一次显示.但是目前存在一个[问题](##使用WinCanvas(width,height)和WinCanvas(hWnd)的效率问题).

### 2.2.1 一次绘制,无数次显示

```C++
using namespace WP;
WinCanvas* w;
void setup() {
	w = new WinCanvas(800, 800);	//创建和窗口同大的画布
	w->background(0);							//画布的绘制是全黑色的
}

void draw(HWND hwnd) {
	w->display(hwnd);							//向指定的窗口绘制
}
```

![image-20200713123310406](https://i.loli.net/2020/07/13/6yUi45dTBGfh3cC.png)



### 2.2.2 一次绘制一次显示

```C++
using namespace WP;
void setup() {
}

void draw(HWND hwnd) {
	WinCanvas w(hwnd);	//获取到窗口,会自动创建和窗口同大的画布
	w.background(0);
	w.display();				//显示到WinCanvas创建时指定的窗口
}
```

​	效果和2.2.1是一致的.

## 2.3 错误提示

​	当你使用了:

```C++
#define PRINT_TO_VS_CONSOLE
```

​	当遇到错误的时候会将错误打印到visual studio的输出栏中.当遇到意外退出或窗口关闭的话可以去看看输出栏中给出的错误提示.

![image-20200713124311076](https://i.loli.net/2020/07/13/oBxlqjDZJI2ifLd.png)



## 2.4 文字的显示

​	因为C++默认的是GDK编码,char*类型的文字类型,但是GDI+需要的是WCHAR(wchar_t)双字节的unicode编码类型,所以需要使用:

```C++
WCHAR * str = L"测试";
```

​	进行字符串的初始化,而且给出了WP::wchar2char和char2wchar两个转化函数使用.



## 2.5 向非窗口的区域绘制

​	GDI+也提供了向非创建的窗口内容绘制(可以是屏幕的任何位置).即hwnd 为 nullptr的时候.所以可以在创建WinCanvas或display(HWND)使用WP::FULL_SCREEN这个宏去向整个屏幕绘制.

```C++
using namespace WP;
WinCanvas* w;
void setup() {
	w = new WinCanvas(800, 800);	//创建800,800的窗口
	w->background(0);				//画布的绘制是全黑色的
}

void draw(HWND hwnd) {
	w->display(FULL_SCREEN);		//使用FULL_SCREEN向整个屏幕进行绘制
   	//绘制的位置是0,0宽高为WinCanvas的大小
}
```

![image-20200713191407680](https://i.loli.net/2020/07/13/6zh1qwnUVF2bvSI.png)

```C++
using namespace WP;
void setup() {
}

void draw(HWND hwnd) {
	WinCanvas w(FULL_SCREEN);
	w.stroke(0, 0, 200);
	w.strokeWeight(5);
	w.line(100, 100, w.width - 100, w.height - 100);
	w.display();
}
```

![image-20200713191956875](https://i.loli.net/2020/07/13/RvZoc8nNOrpGXBC.png)



# 3. 函数的说明

## 3.1 大部分函数的使用说明

​	因为是为了实现P5.js,所以可以在[p5.js参考](https://p5js.org/zh-Hans/reference/)中找到相同函数的使用方式.下面的函数说明均是不同的实现.



## 3.2 dispaly

​	这个函数是最大的不同.因为一个窗口的HWND是变化的,像极了抓不到的致富之遇.所以,在draw函数中,应该使用这个函数去指定将WinCanvas绘制到哪个窗口.

```C++
void display();			//使用WinCanvas(HWND)创建的对象调用
void display(HWND hWnd); //使用WinCanvas(width,height)创建的对象调用
```



## 3.3 push()和pop()

​	这两个函数会将所有的绘制属性(颜色填充字体大小等)以及变形(平移缩放旋转)都将保存和恢复.可以去[p5.js参考](https://p5js.org/zh-Hans/reference/)中查看他们的用法.

​	在WinCanvas中,因为不会再对每帧进行变形的重置,所以请使用他们进行更安全的绘制.



## 3.4 point()

​	绘制点的函数,将不再和p5.js中受到属性的影响(除了socket设置的颜色),point将只对指定的像素单位进行涂色.

​	原因:这是因为GDI+最小的绘制单位是线(以给出的API)而无point的绘制,为了使用,所以获取了BitMap的一个像素进行了处理.



## 3.5 beginShape() 自定义图像

​	其实在WinCanvas中已经给出了两个自定义的图像:quad和triangle.

​	但是要说明的是:beginShape其实会返回一个`Gdiplus::GraphicsPath*`的值,本意是使用GDI+中提供的更加完善的方法去绘制图像.而在endShape中,将去绘制这个图像的路径,vertex只是暂时保存了这个路径的指定点.

​	beginContourv也同样是对beginShape的路径处理(绘制方向相反),使用endContour进行的绘制.



## 3.6 text()函数

​	字符处理给出了WP::wchar2char和char2wchar两个函数去处理字符串.

​	需要注意的是:C++的字符串和其他的类型不再是使用`+`连接.所以可能需要使用到std::to_string()去处理其他的类型到字符串的类型.



# 一些常见的问题

## 为什么要使用`Windows GDI+`而不是`Windows GDI`

​	GDI的绘制方式和HTML5中的Canvas绘制方式很近,但是放弃GDI的最本质的原因是:透明度的支持.

​	GDI+相对GDI给我更大的感觉是更简便和全面.





# 遗留问题

## (已经解决以下问题)

## 使用WinCanvas(width,height)和WinCanvas(hWnd)的效率问题

​	这是一个很奇怪的问题(起码对我来说).使用两种不同的方式去绘制,但是两种方式只是`::GetDC(hWnd)`获取的时间不同.

​	WinCanvas(width,height);的方式是在display(hwnd)进行显示的时候获取并在显示之后释放的,而WinCanvas(hWnd)是在一开始的时候就获取了,但是只是保存了hdc在display()的时候进行使用,在close()中释放.

​	最后测试发现绘制的显示中的`g->DrawCachedBitmap(&cachedBmp, 0, 0);`去绘制的时候两个消耗的时间是WinCanvas(hWnd):WinCanvas(width,height) = 1:4;

> WinCanvas(width,height)的测试结果

```C++
WinCanvas* w;
void setup() {
	w = new WinCanvas(800, 800);
}

void draw(HWND hwnd) {
	w->background(0);
	w->display(hwnd);
}
```

​	以下为测试结果(不取前3次绘时间):

![image-20200713114530956](https://i.loli.net/2020/07/13/6bPoVZwAB1Eury9.png)



![image-20200713114727898](https://i.loli.net/2020/07/13/wGeapxnLcFifZkU.png)



![image-20200713115127850](https://i.loli.net/2020/07/13/FGvasrNb2LfliyV.png)

​	最后测试帧数稳定在43帧.



> WinCanvas(hWnd)的测试结果:

```C++
void setup() {
}

void draw(HWND hwnd) {
	WinCanvas* w = new WinCanvas(hwnd);
	w->background(0);
	w->display();
	delete w;
	w = nullptr;
}
```

​	以下为测试结果(不取前3次绘时间):

![image-20200713115817163](https://i.loli.net/2020/07/13/kMUsZhF8RWbgnzp.png)



![image-20200713115842694](https://i.loli.net/2020/07/13/ulOaMvIoUqrYjFJ.png)



![image-20200713120318068](https://i.loli.net/2020/07/13/m7EJCfjkQ6KOsPg.png)

​	加上其他窗口事件的消耗,那么帧率应该稳定在100帧.



​	我以为是`::GetDc(hWnd)`,获取的时间不同造成的.但是使用以下方式进行测试的时候,发现两者又出奇的一致(任选非前3次绘制):

```C++
#define DRAW_THEN_GETDC

#ifdef DRAW_THEN_GETDC
Gdiplus::Bitmap* bmp;
Gdiplus::Graphics* ghsptr;
#endif // DRAW_THEN_GETDC

void setup() {
#ifdef DRAW_THEN_GETDC
	bmp = new Gdiplus::Bitmap(800, 800);
	ghsptr = new Gdiplus::Graphics(bmp);
#endif
}

void draw(HWND hwnd) {
	HDC hdc = ::GetDC(hwnd);
#ifdef DRAW_THEN_GETDC
	ghsptr->Clear(Gdiplus::Color(0, 0, 0));
#else	//GETDC 后 绘制
	Gdiplus::Bitmap* bmp = new Gdiplus::Bitmap(800, 800);
	Gdiplus::Graphics* ghsptr = new Gdiplus::Graphics(bmp);
	ghsptr->Clear(Gdiplus::Color(0, 0, 0));
#endif
	Gdiplus::Graphics* g = new Gdiplus::Graphics(hdc);
	Gdiplus::CachedBitmap cachedBmp(bmp, g);
	g->DrawCachedBitmap(&cachedBmp, 0, 0);
#ifndef DRAW_THEN_GETDC
	delete bmp;
	delete ghsptr;
#endif
	delete g;
	g = nullptr;
	::ReleaseDC(hwnd, hdc);
}
```

​	但是这次测试两种绘制方式又出奇的一致近1:1.(&*$#&*$&#*$&*#&!)

​	......(希望大佬解答)

> 一次偶然的测试得到

​	如果在WinCanvas(hwnd)的构造函数使用数值直接对width和height赋值的话(不使用GetClientRect获取到窗口的大小),则效率降低为WinCanvas(width,height).

​	但是在display(hwnd)中使用了GetClientRect并没有达到相应的提速效果.

### 已解决

​	不得不说,闭源是真的没好处.

​	谁也想不到一个[GetClientRect](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getclientrect)的使用与否竟然是造成这个问题的关键问题.查了半天的微软文档也没发现说什么,但是实际测试里面确确时时出现了效率4倍的情况.

```C++
#define DRAW_THEN_GETDC

#ifdef DRAW_THEN_GETDC
Gdiplus::Bitmap* bmp_base;
Gdiplus::Graphics* ghsptr;
#endif // DRAW_THEN_GETDC

void setup() {
#ifdef DRAW_THEN_GETDC
	bmp_base = new Gdiplus::Bitmap(800, 800);
	ghsptr = new Gdiplus::Graphics(bmp_base);
#endif
}

void draw(HWND hwnd) {
	RECT rect;
	::GetClientRect(hwnd, &rect);	//获取窗口的信息

#ifdef DRAW_THEN_GETDC
	ghsptr->Clear(Gdiplus::Color(0, 0, 0));
	Gdiplus::Bitmap* bmp = bmp_base->Clone(Gdiplus::Rect(rect.left, rect.top, rect.right, rect.bottom), PixelFormatDontCare);
    HDC hdc = ::GetDC(hwnd);
#else	//GETDC 后 绘制
	HDC hdc = ::GetDC(hwnd);
	Gdiplus::Bitmap* bmp = new Gdiplus::Bitmap(rect.right - rect.left, rect.bottom - rect.top);
	Gdiplus::Graphics* ghsptr = new Gdiplus::Graphics(bmp);
	ghsptr->Clear(Gdiplus::Color(0, 0, 0));
#endif

	Gdiplus::Graphics* g = new Gdiplus::Graphics(hdc);
	Gdiplus::CachedBitmap cachedBmp(bmp, g);
	g->DrawCachedBitmap(&cachedBmp, 0, 0);	//最耗时的一步
	delete g;

	//战后清理
#ifndef DRAW_THEN_GETDC
	delete bmp;
	delete ghsptr;
#else
	delete bmp;
#endif

	::ReleaseDC(hwnd, hdc);
}
```

​	这个是最有说明的内容了,他是上一个代码的优化,只是添加了`GetClientRect`这个函数的使用,**并在其之后创建一个近似同大小的`BitMap`**,那么情况就会出现为:

> 开启了`DRAW_THEN_GETDC`(先绘制后使用GetDC向窗口内显示),老样子不取前3次

![image-20200714111146322](https://i.loli.net/2020/07/14/SCdE7VR3FgOeDzo.png)

![image-20200714111213746](https://i.loli.net/2020/07/14/askfbERdxZiA9qj.png)

![image-20200714111239138](https://i.loli.net/2020/07/14/FTZ2L71XaGmiKAw.png)

> 关闭了`DRAW_THEN_GETDC`(先使用GetDC后绘制向窗口内显示),老样子不取前3次

![image-20200714111518451](https://i.loli.net/2020/07/14/5OTFXjahBSsukwQ.png)

![image-20200714111545049](https://i.loli.net/2020/07/14/a2kdqJOMyANhSpb.png)

![image-20200714111624805](https://i.loli.net/2020/07/14/bY2O7mf5RxCsPFy.png)

​	终于达到了1:1且都是高帧数的效果了.(我太难了)

> 依照此结果修改了display(hwnd)测试结果如下:

![image-20200714112231480](https://i.loli.net/2020/07/14/3A2dDpsaTeFPvN6.png)

​	ohhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh!

### 结案报告

​	只是解决了效率问题,很多人都会问为什么.其实在无法看到GetClientRect和BitMap的内部的时候,我们也不能妄下结论,甚至只能去猜测:GetClientRect不仅获取了窗口的客户的的信息,同时还激活了同等大小的一个"快速绘制区域",这个区域在之后的BitMap创建的时候指定了它,所以加快了效率.

​	但是这样说的话还是有两个问题:

​	1.如果窗口大小是:800X800,那么依旧使用GetClientRect,BitMap在Clone的时候第一个参数修改为:`Gdiplus::Rect(rect.left, rect.top,800, 800)`这个和rect.right,rect.bottom的值是一样的.但是测试后的效率依旧是低效率的.

​	2.如果说是同样大小的话,那么:`Gdiplus::Rect(rect.left, rect.top, rect.right -1, rect.bottom -1)`也是高效率的状态,但是他和"快速绘制区域"大小不等.

​	`dislay(HWND)`使用了第2个问题的情况处理了当使用`Clone`克隆区域大于绘制区域的情况处理:

```C++
	//保护当克隆区域(实时窗口的大小)大于实际的绘制区域(clone会报错)时,不再出现错误
	int offw = this->width - rect.right; offw = offw > 0 ? 0 : offw;
	int offh = this->height - rect.bottom; offh = offh > 0 ? 0 : offh;
	Gdiplus::Bitmap* bmptem = _bmp->Clone(Gdiplus::Rect(rect.left, rect.top, rect.right + offw, rect.bottom + offh), PixelFormatDontCare);
```

​	测试后dislay(HWND)依旧是高效率的绘制状态.

