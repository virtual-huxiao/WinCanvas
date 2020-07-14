#include "WinCanvas.h"
#include <cmath>
#include <random>

WP::WinCanvas::WinCanvas(const HWND hWnd)
	:_hWnd(hWnd), _isRelesed(false), _isHaveHwnd(true), _renderer(new Renderer()),
	_textFont(new Gdiplus::FontFamily(L"Arial")) {
	_hdc = ::GetDC(_hWnd);
	if (_hWnd != WP::FULL_SCREEN) {
		RECT rect;
		::GetClientRect(_hWnd, &rect);
		this->width = unsigned int(rect.right - rect.left);
		this->height = unsigned int(rect.bottom - rect.top);
	}
	else {
		this->width = ::GetSystemMetrics(SM_CXSCREEN);
		this->height = ::GetSystemMetrics(SM_CYSCREEN);
	}
	_bmp = new Gdiplus::Bitmap(this->width, this->height);
	_ghsptr = new Gdiplus::Graphics(_bmp);
	_ghsptr->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	_judgeVaild();
}

WP::WinCanvas::WinCanvas(unsigned int width, unsigned int height)
	:_hWnd(nullptr), _isRelesed(false), _isHaveHwnd(false), _renderer(new Renderer()),
	width(width), height(height), _textFont(new Gdiplus::FontFamily(L"Arial")) {
	_bmp = new Gdiplus::Bitmap(this->width, this->height);
	_ghsptr = new Gdiplus::Graphics(_bmp);
	_ghsptr->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	_judgeVaild();
}

WP::WinCanvas::~WinCanvas() {
	this->close();
}

void WP::WinCanvas::close() {
	if (!_isRelesed) {
		if (_isHaveHwnd) {
			::ReleaseDC(_hWnd, _hdc);
		}
		_isRelesed = true;
		delete _bmp;
		_bmp = nullptr;
		delete _ghsptr;
		_ghsptr = nullptr;
		delete _renderer;
		_renderer = nullptr;
		delete _textFont;
		_textFont = nullptr;
	}
}

void WP::WinCanvas::display() {
	_judgeVaild();
	if (!_isHaveHwnd) {
		throw WC_expection("应该调用带有HWND参数的display!");
	}
	Gdiplus::Graphics* g = new Gdiplus::Graphics(_hdc);
	Gdiplus::CachedBitmap cachedBmp(_bmp, g);
	g->DrawCachedBitmap(&cachedBmp, 0, 0);
	delete g;
	g = nullptr;
}

void WP::WinCanvas::display(HWND hWnd) {//每次hwnd都是不同的
	_judgeVaild();
	HDC hdc = ::GetDC(hWnd);
	RECT rect;
	::GetClientRect(hWnd, &rect);
	//保护当克隆区域(实时窗口的大小)大于实际的绘制区域(clone会报错)时,不再出现错误
	int offw = this->width - rect.right; offw = offw > 0 ? 0 : offw;
	int offh = this->height - rect.bottom; offh = offh > 0 ? 0 : offh;
	Gdiplus::Bitmap* bmptem = _bmp->Clone(Gdiplus::Rect(rect.left, rect.top, rect.right + offw, rect.bottom + offh), PixelFormatDontCare);
	Gdiplus::Graphics* g = new Gdiplus::Graphics(hdc);
	Gdiplus::CachedBitmap cachedBmp(bmptem, g);
	g->DrawCachedBitmap(&cachedBmp, 0, 0);
	delete bmptem;
	bmptem = nullptr;
	delete g;
	g = nullptr;
	ReleaseDC(hWnd, hdc);
}

WP::WinCanvas* WP::WinCanvas::createGraphics(unsigned int width, unsigned int height) {
	return (new WP::WinCanvas(width, height));
}

void WP::WinCanvas::fill(byte rgb, byte a /*= 255*/) {
	_renderer->fillColor = Gdiplus::Color(a, rgb, rgb, rgb);
	_renderer->isNoFill = false;
}

void WP::WinCanvas::fill(byte r, byte g, byte b, byte a /*= 255*/) {
	_renderer->fillColor = Gdiplus::Color(a, r, g, b);
	_renderer->isNoFill = false;
}

void WP::WinCanvas::noFill() {
	_renderer->isNoFill = true;
}

void WP::WinCanvas::stroke(byte rgb, byte a /*= 255*/) {
	_renderer->strokeColor = Gdiplus::Color(a, rgb, rgb, rgb);
	_renderer->isNoStroke = false;
}

void WP::WinCanvas::stroke(byte r, byte g, byte b, byte a /*= 255*/) {
	_renderer->strokeColor = Gdiplus::Color(a, r, g, b);
	_renderer->isNoStroke = false;
}

void WP::WinCanvas::noStroke() {
	_renderer->isNoStroke = true;
}

void WP::WinCanvas::strokeWeight(unsigned int weight) {
	_renderer->strokeWeight = weight;
}

void WP::WinCanvas::translate(float offX, float offY) {
	_ghsptr->TranslateTransform(offX, offY);
}

void WP::WinCanvas::scale(float xy) {
	this->scale(xy, xy);
}

void WP::WinCanvas::scale(float x, float y) {
	_ghsptr->ScaleTransform(x, y);
}

void WP::WinCanvas::rotate(float angle) {
	_ghsptr->RotateTransform(angle);
}

void WP::WinCanvas::push() {
	_styles.push_back((*_renderer));
	_transform.push_back(_ghsptr->Save());
}

void WP::WinCanvas::pop() {
	if (_styles.empty() || _transform.empty()) {
		throw WC_expection("push 和 pop应当成对使用!");
	}
	*_renderer = _styles.back();
	_ghsptr->Restore(_transform.back());
	_styles.pop_back();
	_transform.pop_back();
}

void WP::WinCanvas::background(byte rgb, byte a /*=255*/) {
	this->background(rgb, rgb, rgb, a);
}

void WP::WinCanvas::background(byte r, byte g, byte b, byte a /*=255*/) {
	_ghsptr->Clear(Gdiplus::Color(a, r, g, b));
}

void WP::WinCanvas::point(float x, float y) {
	_bmp->SetPixel(x, y, _renderer->strokeColor);
}

void WP::WinCanvas::arc(float x, float y, float w, float h, float startAngle, float stopAngle) {
	_judgeVaild();
	auto p = this->getNowPen();
	_ghsptr->DrawArc(p, x, y, w, h, startAngle, stopAngle);
	delete p;
	p = nullptr;
}

void WP::WinCanvas::quad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
	_judgeVaild();
	this->beginShape();
	this->vertex(x1, y1);
	this->vertex(x2, y2);
	this->vertex(x3, y3);
	this->vertex(x4, y4);
	this->endShape(WP::CLOSE);
}

void WP::WinCanvas::bezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
	auto p = this->getNowPen();
	_ghsptr->DrawBezier(p, x1, y1, x2, y2, x3, y3, x4, y4);
	delete p;
}

void WP::WinCanvas::triangle(float x1, float y1, float x2, float y2, float x3, float y3) {
	_judgeVaild();
	this->beginShape();
	this->vertex(x1, y1);
	this->vertex(x2, y2);
	this->vertex(x3, y3);
	this->endShape(WP::CLOSE);
}

void WP::WinCanvas::line(float beginX, float beginY, float endX, float endY) {
	_judgeVaild();
	Gdiplus::Pen pen(_renderer->strokeColor, _renderer->strokeWeight);
	_ghsptr->DrawLine(&pen,
		beginX, beginY, endX, endY);
}

void WP::WinCanvas::rect(float x, float y, float width, float height) {
	_judgeVaild();
	if (!_renderer->isNoFill) {
		Gdiplus::SolidBrush sBrush(_renderer->fillColor);
		_ghsptr->FillRectangle(&sBrush,
			x, y,
			width, height);
	}
	if (!_renderer->isNoStroke) {
		Gdiplus::Pen pen(_renderer->strokeColor, _renderer->strokeWeight);
		_ghsptr->DrawRectangle(&pen,
			x, y,
			width, height);
	}
}

void WP::WinCanvas::ellipse(float x, float y, float d) {
	this->ellipse(x, y, d, d);
}

void WP::WinCanvas::ellipse(float x, float y, float xd, float yd) {
	_judgeVaild();
	if (!_renderer->isNoFill) {
		Gdiplus::SolidBrush sBrush(_renderer->fillColor);
		_ghsptr->FillEllipse(&sBrush,
			x, y,
			xd, yd);
	}
	if (!_renderer->isNoStroke) {
		Gdiplus::Pen pen(_renderer->strokeColor, _renderer->strokeWeight);
		_ghsptr->DrawEllipse(&pen,
			x, y,
			xd, yd);
	}
}

void WP::WinCanvas::image(WP::Image* img, float x, float y) {
	Gdiplus::CachedBitmap cachedBitmap(img->getGdiplusImage(), _ghsptr);
	_ghsptr->DrawCachedBitmap(&cachedBitmap, x, y);
}

void WP::WinCanvas::image(WP::Image* img, float x, float y, float w, float h) {
	_ghsptr->DrawImage(img->getGdiplusImage(), x, y, w, h);
}

void WP::WinCanvas::image(WP::WinCanvas* other, float x, float y) {
	Gdiplus::CachedBitmap cachedBitmap(other->_bmp, _ghsptr);
	_ghsptr->DrawCachedBitmap(&cachedBitmap, x, y);
}

void WP::WinCanvas::image(WP::WinCanvas* other, float x, float y, float w, float h) {
	_ghsptr->DrawImage(other->_bmp, x, y, w, h);
}

Gdiplus::GraphicsPath* WP::WinCanvas::beginShape() {
	auto path = new Gdiplus::GraphicsPath();
	_shapePath = new WP::ShapePath(path, true);
	return path;
}

void WP::WinCanvas::vertex(float x, float y) {
	if (_shapePath->getIsClockwise()) {//beginShape
		_shapePath->linePoints.push_back(Gdiplus::Point(x, y));
	}
	else {//beginContour
		_shapePath->r_linePoints.push_back(Gdiplus::Point(x, y));
	}
}

void WP::WinCanvas::endShape(bool isClose /*= false*/) {
	/*
		有权利删除_shapePath成员.
	*/
	if (_shapePath->linePoints.empty()) {
		delete _shapePath;
		_shapePath = nullptr;
		return;
	}
	if (isClose) {
		_shapePath->linePoints.push_back(_shapePath->linePoints.front());
	}
	_shapePath->getGraphicsPath()->StartFigure();//之后的绘制不连接之前的最后一个点(否则就是一笔画,在绘制之前的路径都要最后一点作为新路径的起始点)
	_shapePath->getGraphicsPath()->AddLines(_shapePath->linePoints.data(), _shapePath->linePoints.size());
	_shapePath->linePoints.clear();
	if (!_renderer->isNoFill) {
		auto b = getNowSolidBrush();
		_ghsptr->FillPath(b, _shapePath->getGraphicsPath());
		delete b;
	}
	if (!_renderer->isNoStroke) {
		auto p = getNowPen();
		_ghsptr->DrawPath(p, _shapePath->getGraphicsPath());
		delete p;
	}
	delete _shapePath;
	_shapePath = nullptr;
}

Gdiplus::GraphicsPath* WP::WinCanvas::beginContour() {
	_shapePath->setIsClockwise(false);
	return _shapePath->getGraphicsPath();
}

void WP::WinCanvas::endContour(bool isClose /*=false*/) {
	/*
		无权删除_shapePath,只是修改了路径方向
	*/
	if (_shapePath->r_linePoints.empty()) return;
	if (isClose) {
		_shapePath->r_linePoints.push_back(_shapePath->r_linePoints.front());
	}
	_shapePath->getGraphicsPath()->AddLines(_shapePath->r_linePoints.data(), _shapePath->r_linePoints.size());
	_shapePath->r_linePoints.clear();
	_shapePath->getGraphicsPath()->Reverse();//反置绘制方向
	_shapePath->setIsClockwise(true);
}

void WP::WinCanvas::textFont(const WCHAR* family) {
	delete _textFont;
	_textFont = new Gdiplus::FontFamily(family);
}

void WP::WinCanvas::textWidth(const WCHAR* str) {
	//todo
}

void WP::WinCanvas::textSize(unsigned int size) {
	_renderer->textSize = size;
}

void WP::WinCanvas::text(const WCHAR* str, float x, float y) {
	Gdiplus::Font font(_textFont, _renderer->textSize, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	auto b = getNowSolidBrush();
	Gdiplus::PointF pf(x, y);
	_ghsptr->DrawString(str, -1, &font, pf, b);
	delete b;
}

Gdiplus::Pen* WP::WinCanvas::getNowPen()const {
	return (new Gdiplus::Pen(_renderer->strokeColor, _renderer->strokeWeight));
}

Gdiplus::SolidBrush* WP::WinCanvas::getNowSolidBrush() const {
	return (new Gdiplus::SolidBrush(_renderer->fillColor));
}

void WP::WinCanvas::_judgeVaild() {
	if (_isRelesed) {
		throw WP::WC_expection("资源已经释放!");
	}
	if (_isHaveHwnd) {
		if (_hdc == nullptr) {
			throw WP::WC_expection("无效资源!");
		}
	}
	if (_ghsptr == nullptr) {
		throw WP::WC_expection("gdiplus对象为null!");
	}
}

WP::ShapePath::ShapePath(Gdiplus::GraphicsPath* shape_path, bool is_clockwise)
	: _shapePath(shape_path), _isClockwise(is_clockwise) {
}

WP::ShapePath::~ShapePath() {
	delete _shapePath;
	_shapePath = nullptr;
}

Gdiplus::GraphicsPath* WP::ShapePath::getGraphicsPath() {
	return _shapePath;
}

void WP::ShapePath::setIsClockwise(bool isClockwise) {
	_isClockwise = isClockwise;
}

bool WP::ShapePath::getIsClockwise() const {
	return _isClockwise;
}

WP::Renderer::Renderer()
	: fillColor(Gdiplus::Color(255, 255, 255)),
	strokeColor(Gdiplus::Color(0, 0, 0)),
	isNoFill(false),
	isNoStroke(false),
	//originX(0),
	//originY(0),
	strokeWeight(1),
	textSize(12) {//初始化所有的默认值
}

WP::Renderer::~Renderer() {
}

WP::WC_expection::WC_expection(const char* msg) noexcept
	:_msg(msg) {
}

WP::WC_expection::WC_expection(const WC_expection& other) noexcept {
	_msg = other._msg;
}

const char* WP::WC_expection::what() const {
	return _msg;
}

/**
 * class WP::Image
 */
Gdiplus::Bitmap* WP::Image::getGdiplusImage() const {
	return _img;
}

WP::Image* WP::Image::cut(float partX, float partY, float partW, float partH) {
	return (new Image(this, partX, partY, partW, partH));
}

WP::Image::Image(const WCHAR* imgPath) {
	_img = new Gdiplus::Bitmap(imgPath);
	this->width = _img->GetWidth();
	this->heigth = _img->GetHeight();
}

WP::Image::Image(const WCHAR* imgPath, float partX, float partY, float partW, float partH) {
	Gdiplus::Bitmap* img = new Gdiplus::Bitmap(imgPath);
	Gdiplus::RectF sourceRect(partX, partY, partW, partH);
	_img = img->Clone(sourceRect, PixelFormatDontCare);
	this->heigth = _img->GetHeight();
	this->width = _img->GetWidth();
	delete img;
}

WP::Image::Image(WP::Image* other, float partX, float partY, float partW, float partH) {
	Gdiplus::RectF sourceRect(partX, partY, partW, partH);
	_img = other->_img->Clone(sourceRect, PixelFormatDontCare);
	this->heigth = _img->GetHeight();
	this->width = _img->GetWidth();
}

WP::Image::~Image() {
	delete _img;
	_img = nullptr;
}

WP::Image* WP::loadImage(const WCHAR* path) {
	return (new WP::Image(path));
}

WP::Image* WP::loadImage(const WCHAR* path, float partX, float partY, float partW, float partH) {
	return (new WP::Image(path, partX, partY, partW, partH));
}

int WP::abs(int num) {
	return std::abs(num);
}

float WP::abs(float num) {
	return std::fabs(num);
}

double WP::abs(double num) {
	return std::fabs(num);
}

float WP::constrain(float num, float min, float max) {
	if (num < min) {
		return min;
	}
	else if (num > max) {
		return max;
	}
	else {
		return num;
	}
}

float WP::dist(float x1, float y1, float x2, float y2) {
	return WP::sqrt(sq(x1 - x2) + sq(y1 - y2));
}

float WP::lerp(float start, float stop, float amt) {
	return (stop - start) * amt + start;
}

float WP::log(float n) {
	return std::log(n);
}

float WP::mag(float x, float y) {
	return WP::dist(0, 0, x, y);
}

float WP::map(float value, float start1, float stop1, float start2, float stop2) {
	if (value == 0) return start2;
	return ((stop1 - start1) / value) * (stop2 - start2) + start2;
}

float WP::norm(float value, float start, float stop) {
	return WP::map(value, start, stop, 0, 1);
}

float WP::pow(float n, float e) {
	return std::powf(n, e);
}

float WP::sq(float n) {
	return std::powf(n, 2);
}

float WP::sqrt(float n) {
	return std::sqrtf(n);
}

float WP::random() {
	return std::rand();
}

float WP::random(float min, float max) {
	return WP::lerp(min, max, WP::random());
}

float WP::sin(float angle) {
	return std::sin(angle);
}

float WP::cos(float angle) {
	return std::cos(angle);
}

float WP::tan(float angle) {
	return std::tan(angle);
}

#define GET_LOCAL_TIME(arg) unsigned int result;\
														SYSTEMTIME systime;\
														::GetLocalTime(&systime);\
														result = (unsigned int)systime.##arg;\
														return result;

unsigned int  WP::year() {
	GET_LOCAL_TIME(wYear);
}

unsigned int  WP::month() {
	GET_LOCAL_TIME(wMonth);
}

unsigned int  WP::week() {
	GET_LOCAL_TIME(wDayOfWeek);
}

unsigned int  WP::day() {
	GET_LOCAL_TIME(wDay);
}

unsigned int  WP::hour() {
	GET_LOCAL_TIME(wHour);
}

unsigned int  WP::minute() {
	GET_LOCAL_TIME(wMinute);
}

unsigned int  WP::second() {
	GET_LOCAL_TIME(wSecond);
}

unsigned int  WP::millis() {
	GET_LOCAL_TIME(wMilliseconds);
}

#undef GET_LOCAL_TIME

char* WP::wchar2char(const WCHAR* wchar) {
	char* m_char;
	int len = ::WideCharToMultiByte(CP_ACP, 0, wchar, wcslen(wchar), NULL, 0, NULL, NULL);
	m_char = new char[len + 1];
	::WideCharToMultiByte(CP_ACP, 0, wchar, wcslen(wchar), m_char, len, NULL, NULL);
	m_char[len] = '\0';
	return m_char;
}

WCHAR* WP::char2wchar(const char* cchar) {
	wchar_t* m_wchar;
	int len = ::MultiByteToWideChar(CP_ACP, 0, cchar, strlen(cchar), NULL, 0);
	m_wchar = new wchar_t[len + 1];
	::MultiByteToWideChar(CP_ACP, 0, cchar, strlen(cchar), m_wchar, len);
	m_wchar[len] = '\0';
	return m_wchar;
}