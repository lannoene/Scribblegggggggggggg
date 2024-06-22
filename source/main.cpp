#include <iostream>
#include <vector>

#include "sdlwrp.h"

bool pushingDown = false;

int circle_rad = 2;
int draw_color = CLR_RED;
int global_skip = 1;

double distance(double x1, double y1, double x2, double y2)
{
    double square_difference_x = (x2 - x1) * (x2 - x1);
    double square_difference_y = (y2 - y1) * (y2 - y1);
    double sum = square_difference_x + square_difference_y;
    double value = sqrt(sum);
    return value;
}

SDL_Texture *circleTex;
extern SDL_Renderer *rend;

void DrawCircle(int cx, int cy, int radius, int color) {
	extern SDL_Renderer *rend;
	circleTex = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, radius*2, radius*2);
	// Note that there is more to altering the bitrate of this 
	// method than just changing this value.  See how pixels are
	// altered at the following web page for tips:
	//   http://www.libsdl.org/intro.en/usingvideo.html
	static const int BPP = 4;
	cx += radius;
	cy += radius;

	//double ra = (double)radius;
	SDL_SetRenderTarget(rend, circleTex);
	SDL_SetTextureBlendMode(circleTex, SDL_BLENDMODE_BLEND);
	Uint8 alpha = 0xFF;
	SDL_SetTextureAlphaMod(circleTex, alpha);
	for (double dy = 1; dy <= radius; dy += 1.0) {
		double dx = floor(sqrt((2.0 * radius * dy) - (dy * dy)));
		int x = cx - dx;
		Wrp_SetRendColor(color);
		SDL_RenderDrawLine(rend, cx - dx, cy + dy - radius, cx + dx, cy + dy - radius);
		SDL_RenderDrawLine(rend, cx - dx, cy - dy + radius, cx + dx, cy - dy + radius);
	}
	SDL_SetRenderTarget(rend, NULL);
}

void DrawLineWithThickness(int x1, int y1, int x2, int y2, int thickness, int color) {
	float curX = x1;
	float curY = y1;
	SDL_Rect rrr = {x1, y1, thickness*2, thickness*2};
	SDL_RenderCopy(rend, circleTex, NULL, &rrr);
	double dist = distance(x1, y1, x2, y2);
	for (int i = 0; i < dist; i++) {
		curX += (x2 - x1)/dist;
		curY += (y2 - y1)/dist;
		rrr.x = curX;
		rrr.y = curY;
		SDL_RenderCopy(rend, circleTex, NULL, &rrr);
	}
}

void DrawLineAlternatingPattern(int x1, int y1, int x2, int y2, int thickness, int color) {
	float curX = x1;
	float curY = y1;
	SDL_Rect rrr = {x1, y1, circle_rad*2, circle_rad*2};
	SDL_RenderCopy(rend, circleTex, NULL, &rrr);
	double dist = distance(x1, y1, x2, y2);
	for (int i = 0; i < dist; i++) {
		curX += (x2 - x1)/dist;
		curY += (y2 - y1)/dist;
		rrr.x = curX;
		rrr.y = curY;
		SDL_RenderCopy(rend, circleTex, NULL, &rrr);
	}
}

struct ScribblePoint {
public:
	ScribblePoint(int nx, int ny) {x = nx; y = ny;}
	int x, y;
private:
};

enum scribble_types {
	SC_CIRCLE_FILL,
	SC_CIRCLE_ALTERNATING,
};

class Scribble {
public:
	Scribble() {
		scrColor = draw_color;
		scrThickness = circle_rad;
	}
	void AddPoint(int x, int y);
	void Draw();
	void RedrawScribbleAnimated();
	enum scribble_types type;
	int scrColor = (int)draw_color;
	int scrThickness = (int)circle_rad;
	std::vector<ScribblePoint>& GetScribblePoints() {return pts;}
private:
	std::vector<ScribblePoint> pts;
};

void Scribble::AddPoint(int x, int y) {
	pts.emplace_back(x, y);
}

void Scribble::Draw() {
	DrawCircle(0, 0, scrThickness, scrColor);
	for (size_t i = 0; i < pts.size(); i++) {
		ScribblePoint& p = pts[i];
		ScribblePoint& lastP = (i != 0) ? pts[i - 1] : pts[i];
		DrawLineWithThickness(lastP.x, lastP.y, p.x, p.y, scrThickness, scrColor);
		switch (type) {
			case SC_CIRCLE_FILL:
				
				break;
			case SC_CIRCLE_ALTERNATING:
				//DrawLineAlternatingPattern(lastP.x, lastP.y, p.x, p.y, circle_rad, scrColor);
				break;
		}
	}
}

void Scribble::RedrawScribbleAnimated() {
	DrawCircle(0, 0, scrThickness, scrColor);
	for (size_t i = 0; i < pts.size(); i++) {
		Wrp_CheckDefault();
		ScribblePoint& p = pts[i];
		ScribblePoint& lastP = (i != 0) ? pts[i - 1] : pts[i];

		DrawLineWithThickness(lastP.x, lastP.y, p.x, p.y, scrThickness, scrColor);
		if (i % global_skip == 0)
			Wrp_FinishDrawing();
	}
}

class ScribblePaper {
public:
	void AddNewScribble(int x, int y);
	void AddPointToLastScribble(int x, int y);
	void Draw();
	void DrawLastPoint();
	void RedrawPaperAnimated();
	std::vector<Scribble>& GetScribbles() {return scribs;}
	void AddScribbleObj(Scribble s) {scribs.push_back(s);}
	void ClearScribbles() {scribs.clear();}
private:
	std::vector<Scribble> scribs;
};

void ScribblePaper::AddNewScribble(int x, int y) {
	scribs.push_back(Scribble());
	AddPointToLastScribble(x, y);
}

void ScribblePaper::AddPointToLastScribble(int x, int y) {
	scribs.back().AddPoint(x, y);
}

void ScribblePaper::Draw() {
	for (auto& s : scribs) {
		s.Draw();
	}
}

void ScribblePaper::DrawLastPoint() {
	scribs.back().Draw();
}

void ScribblePaper::RedrawPaperAnimated() {
	Wrp_ClearScreen();
	for (auto& s : scribs) {
		s.RedrawScribbleAnimated();
	}
}

void SaveCurrentDrawing(ScribblePaper& paper);
void LoadDrawingFromFile(ScribblePaper& paper);

std::string currentFilename;

int main(int argc, char *argv[]) {
	puts(
"Welcome! Why don't you draw something:\n\
Controls:\n\
Mouse: Draw\n\
Scroll: Increase/Decrease brush size\n\
Keys:\n\
R: Change color to red\n\
B: Change color to blue\n\
G: Change color to green\n\
Y: Change color to yellow\n\
Q: Change color to black\n\
W: Change color to white\n\
P: Play back drawing\n\
S: Save drawing\n\
L: Load drawing\n\
1-5: Change speed of playback\
");
	Wrp_InitSDL("Draw something!");
	ScribblePaper scrib;
	DrawCircle(0, 0, circle_rad, CLR_RED);
	Wrp_ClearScreen();
	while (true) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					return 0;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.which == 0) {
						scrib.AddNewScribble(event.button.x - circle_rad, event.button.y - circle_rad);
						scrib.DrawLastPoint();
						pushingDown = true;
					}
					break;
				case SDL_MOUSEBUTTONUP:
					if (event.button.which == 0)
						pushingDown = false;
					break;
				case SDL_MOUSEMOTION:
					if (pushingDown) {
						scrib.AddPointToLastScribble(event.motion.x - circle_rad, event.motion.y - circle_rad);
						// redraw
						
						scrib.DrawLastPoint();
					}
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.scancode) {
						default:goto noRedrawCircle;
						case SDL_SCANCODE_P:
							scrib.RedrawPaperAnimated();
							puts("Finished replay");
							goto redrawCircle;
						case SDL_SCANCODE_R:
							draw_color = CLR_RED;
							goto redrawCircle;
						case SDL_SCANCODE_B:
							draw_color = CLR_BLU;
							goto redrawCircle;
						case SDL_SCANCODE_G:
							draw_color = CLR_GRN;
							goto redrawCircle;
						case SDL_SCANCODE_Y:
							draw_color = CLR_YLW;
							goto redrawCircle;
						case SDL_SCANCODE_Q:
							draw_color = CLR_BLK;
							goto redrawCircle;
						case SDL_SCANCODE_W:
							draw_color = CLR_WHT;
							goto redrawCircle;
						case SDL_SCANCODE_S:
							SaveCurrentDrawing(scrib);
							goto noRedrawCircle;
						case SDL_SCANCODE_L:
							LoadDrawingFromFile(scrib);
							goto noRedrawCircle;
						case SDL_SCANCODE_1:
							global_skip = 1;
							goto globalSkipPrint;
						case SDL_SCANCODE_2:
							global_skip = 2;
							goto globalSkipPrint;
						case SDL_SCANCODE_3:
							global_skip = 3;
							goto globalSkipPrint;
						case SDL_SCANCODE_4:
							global_skip = 4;
							goto globalSkipPrint;
						case SDL_SCANCODE_5:
							global_skip = 5;
							goto globalSkipPrint;
					}
globalSkipPrint:
					std::cout << global_skip << "x speed" << std::endl;
					goto noRedrawCircle;
redrawCircle:
					DrawCircle(0, 0, circle_rad, draw_color);
noRedrawCircle:
					break;
				case SDL_MOUSEWHEEL:
					circle_rad += event.wheel.y;
					DrawCircle(0, 0, circle_rad, draw_color);
					std::cout << circle_rad << std::endl;
					break;
			}

			Wrp_FinishDrawing();
		}
	}

	return 0;
}

#include <fstream>

// outside: num of scribbles
// then
// for each scribble
// num of points
// then
// x then y

void SaveCurrentDrawing(ScribblePaper& paper) {
	std::string name;
	if (currentFilename.length() == 0) {
		printf("Drawing title: ");
		std::getline(std::cin, name);
		name += ".larp";
	} else {
		name = currentFilename;
	}
	std::fstream outFile(name, std::ios::out | std::ios::binary); // using fstream cuz c++
	currentFilename = name;
	int numScrib = paper.GetScribbles().size();
	// number of scribbles
	//std::cout << "outputting num scrib: " << numScrib << std::endl;
	outFile.write((char*)&numScrib, sizeof(int));
	for (auto& scribble : paper.GetScribbles()) {
		unsigned int numPts = scribble.GetScribblePoints().size();
		outFile.write((char*)&numPts, sizeof(int)); // number of points in scribble
		outFile.write((char*)&scribble.scrColor, sizeof(int)); // color of scribble
		outFile.write((char*)&scribble.scrThickness, sizeof(int)); // thickness of scribble
		for (auto& scrPt : scribble.GetScribblePoints()) {
			outFile.write((char*)&scrPt.x, sizeof(int));  // x, y pts from the scribble for playback & loading
			outFile.write((char*)&scrPt.y, sizeof(int));
		}
	}
	outFile.close();
	puts("Saved!");
}

// now just do saving in reverse
void LoadDrawingFromFile(ScribblePaper& paper) {
	paper.ClearScribbles();
	std::string name;
	if (currentFilename == "") {
		printf("Enter filename: ");
		std::getline(std::cin, name);
	} else {
		name = currentFilename;
	}
	std::fstream inFile(name, std::ios::in | std::ios::binary);
	int numScrib;
	inFile.read((char*)&numScrib, sizeof(int));
	//std::cout << "num scrib " << numScrib << std::endl;
	for (int i = 0; i < numScrib; i++) {
		Scribble scrib;
		unsigned int numPts;
		inFile.read((char*)&numPts, sizeof(int));
		inFile.read((char*)&scrib.scrColor, sizeof(int));
		inFile.read((char*)&scrib.scrThickness, sizeof(int));

		//std::cout << "num points " << numPts << std::endl;
		//printf("num points: %u\n", numPts);
		for (size_t j = 0; j < numPts; j++) {
			int x, y;
			inFile.read((char*)&x, sizeof(int));
			inFile.read((char*)&y, sizeof(int));
			scrib.AddPoint(x, y);
		}
		paper.AddScribbleObj(scrib);
	}
	inFile.close();
	Wrp_ClearScreen();
	paper.Draw();
	puts("Loaded!");
}