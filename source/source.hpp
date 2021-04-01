#ifndef SOURCE_H
#define SOURCE_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <windows.h>
#include <list>
#include <vector>
//#pragma comment (lib, "tinyxml.lib")
#include "source/TinyXML/tinyxml.h"
//#include <xmllite.h>

using namespace sf;

class	Animation {
public:
	std::vector<IntRect> frames, frames_flip;
	float currentFrame, speed;
	bool loop, flip, isPlaying; // loop показвает зациклена ли анимация. Например анимация взрыва должна проиграться один раз и остановиться, loop=false
	Sprite sprite;
	//		 название, лево-верхний угол, ширина, высота, кол-во кадров, шаг, скорость
	Animation() {
		currentFrame = 0;
		isPlaying = true;
		flip = false;
		loop = true;
	}
	void	tick(float time) {
		if (!isPlaying) return;
		currentFrame += speed * time;
		if (currentFrame > frames.size()) {
			currentFrame -= frames.size();
			if (!loop) {
				isPlaying = false;
				return;
			}
		}
		int i = currentFrame;
		sprite.setTextureRect(frames[i]);
		if (flip)sprite.setTextureRect(frames_flip[i]);
	}
};

class	AnimationManager {
public:
	std::string currentAnim;
	std::map<std::string, Animation> animList;
	AnimationManager() {}
	~AnimationManager() {
		animList.clear();
	}
	//создание анимаций вручную
	void	create(std::string name, Texture& texture, int x, int y, int w, int h, int count, float speed, int step = 0, bool Loop = true) {
		Animation a;
		a.speed = speed;
		a.loop = Loop;
		a.sprite.setTexture(texture);
		a.sprite.setOrigin(0, h);

		for (int i = 0; i < count; i++) {
			a.frames.push_back(IntRect(x + i * step, y, w, h));
			a.frames_flip.push_back(IntRect(x + i * step + w, y, -w, h));
		}
		animList[name] = a;
		currentAnim = name;
	}
	//загрузка из файла XML
	void	loadFromXML(std::string fileName, Texture& t) {
		TiXmlDocument animFile(fileName.c_str());

		animFile.LoadFile();

		TiXmlElement* head;
		head = animFile.FirstChildElement("sprites");

		TiXmlElement* animElement;
		animElement = head->FirstChildElement("animation");
		while (animElement) {
			Animation anim;
			currentAnim = animElement->Attribute("title");
			int delay = atoi(animElement->Attribute("delay"));
			anim.speed = 1.0 / delay; anim.sprite.setTexture(t);

			TiXmlElement* cut;
			cut = animElement->FirstChildElement("cut");
			while (cut) {
				int x = atoi(cut->Attribute("x"));
				int y = atoi(cut->Attribute("y"));
				int w = atoi(cut->Attribute("w"));
				int h = atoi(cut->Attribute("h"));

				anim.frames.push_back(IntRect(x, y, w, h));
				anim.frames_flip.push_back(IntRect(x + w, y, -w, h));
				cut = cut->NextSiblingElement("cut");
			}
			anim.sprite.setOrigin(0, anim.frames[0].height);

			animList[currentAnim] = anim;
			animElement = animElement->NextSiblingElement("animation");
		}
	}
	void	set(std::string name) {
		currentAnim = name;
		animList[currentAnim].flip = 0;
	}
	void	draw(RenderWindow& window, int x = 0, int y = 0) {
		animList[currentAnim].sprite.setPosition(x, y);
		window.draw(animList[currentAnim].sprite);
	}
	void	flip(bool b = 1) { animList[currentAnim].flip = b; } //void flip(bool b) { animList[currentAnim].flip = b; }
	void	tick(float time) { animList[currentAnim].tick(time); }
	void	play() {
		animList[currentAnim].isPlaying = true;
	}
	void	pause() {
		animList[currentAnim].isPlaying = false;
	}
	void	play(std::string name) {
		animList[name].isPlaying = true;
	}
	bool	isPlaying() {
		return animList[currentAnim].isPlaying;
	}
	float	getH() {
		return animList[currentAnim].frames[0].height;
	}
	float	getW() {
		return animList[currentAnim].frames[0].width;
	}
};

struct	Object {
	int GetPropertyInt(std::string name);
	float GetPropertyFloat(std::string name);
	std::string GetPropertyString(std::string name);

	std::string name;
	std::string type;
	sf::Rect<float> rect;
	std::map<std::string, std::string> properties;

	sf::Sprite sprite;
};

struct	Layer {
	int opacity;
	std::vector<sf::Sprite> tiles;
};

class	Level {
public:
	bool LoadFromFile(std::string filename);
	Object GetObject(std::string name);
	std::vector<Object> GetObjects(std::string name);
	std::vector<Object> GetAllObjects();
	void Draw(sf::RenderWindow& window);
	sf::Vector2i GetTileSize();
private:
	int width, height, tileWidth, tileHeight;
	int firstTileID;
	sf::Rect<float> drawingBounds;
	sf::Texture tilesetImage;
	std::vector<Object> objects;
	std::vector<Layer> layers;
};

class	HealthBar {
public:
	Texture t;
	Sprite s;
	int max;
	RectangleShape bar;

	HealthBar() {
		t.loadFromFile("tiles/HealthBar.png");
		s.setTexture(t);

		bar.setFillColor(Color(0, 0, 0));
		max = 100;
	}
	void update(int k) {
		if (k > 0) if (k < max) bar.setSize(Vector2f(10, (max - k) * 70 / max));
	}
	void draw(RenderWindow& window) {
		Vector2f center = window.getView().getCenter();
		Vector2f size = window.getView().getSize();

		s.setPosition(center.x - size.x / 2 + 10, center.y - size.y / 2 + 10);
		bar.setPosition(center.x - size.x / 2 + 11, center.y - size.y / 2 + 13);

		window.draw(s);
		window.draw(bar);
	}
};

class	Entity {
public:
	float x, y, dx, dy, w, h;
	AnimationManager anim;
	std::vector<Object> obj;
	bool life, dir;
	float timer, timer_end;
	std::string Name;
	int Health;

	Entity(AnimationManager& A, int X, int Y) {
		anim = A;
		x = X;
		y = Y;
		dir = 0;

		life = true;
		dx = dy = 0;
	}
	virtual void	update(float time) = 0;//вирт.функ. равна 0 - ЕЁ ОБЯЗАТЕЛЬНО ДОЛЖНЫ РЕАЛИЗОВАТЬ НАСЛЕДНИКИ
	void			draw(RenderWindow& window) {
		anim.draw(window, x, y + h);
	}
	FloatRect		getRect() {
		return FloatRect(x, y, w, h);
	}
	void			option(std::string NAME, float SPEED = 0, int HEALTH = 10, std::string FIRST_ANIM = "") {
		Name = NAME;
		if (FIRST_ANIM != "") anim.set(FIRST_ANIM);
		w = anim.getW();
		h = anim.getH();
		dx = SPEED;
		Health = HEALTH;
	}
};

class	MovingPlatform : public Entity {
public:
	MovingPlatform(AnimationManager& a, Level& lev, int x, int y) :Entity(a, x, y) {
		option("MovingPlatform", 0.05, 0, "move_left");
	}
	void update(float time) {
		x += dx * time;
		timer += time;
		if (timer > 6000) {
			dx *= -1;
			timer = 0;
		}
		anim.tick(time);
	}
};

class	PopWave :public Entity {
public:
	PopWave(AnimationManager& a, Level& lev, int x, int y, bool dir) :Entity(a, x, y) {
		option("PopWave", 0.3, 10, "move");
		if (dir) dx = -0.3;
		obj = lev.GetObjects("solid");
	}
	void	update(float time) {
		x += dx * time;
		for (int i = 0; i < obj.size(); i++)
			if (getRect().intersects(obj[i].rect)) {
				Health = 0;
			}
		if (Health <= 0) {
			anim.set("explode"); dx = 0;
			if (anim.isPlaying() == false)
				life = false;
		}
		anim.tick(time);
	}
};

class	ENEMY : public Entity {
public:
	ENEMY(AnimationManager& a, Level& lev, int x, int y) :Entity(a, x, y) {
		option("Enemy", 0.01, 15, "move");
	}
	void			update(float time){
		x += dx * time;
		timer += time;
		if (timer > 3200) { dx *= -1; timer = 0; }
		if (Health <= 0) {
			anim.set("dead"); dx = 0;
			timer_end += time;
			if (timer_end > 4000) life = false;
		}
		anim.tick(time);
	}
};

class	PLAYER :public Entity {
public:
	enum { stay, walk, duck, jump, climb, swim } STATE;
	bool onLadder, pop, hit;
	std::map<std::string, bool> key;

	PLAYER(AnimationManager& a, Level& lev, int x, int y) :Entity(a, x, y) {
		option("Player", 0, 100, "stay");
		STATE = stay; hit = false;
		obj = lev.GetAllObjects();
	}
	void			KeyCheck() {
		if (key["L"]) {
			dir = 1;
			if (STATE != duck) dx = -0.1;
			if (STATE == stay) STATE = walk;
		}
		if (key["R"]) {
			dir = 0;
			if (STATE != duck) dx = -0.1;
			if (STATE == stay) STATE = walk;
		}
		if (key["Up"]) {
			if (STATE == swim || STATE == climb) { dy = -0.05; }
			if (STATE == stay || STATE == walk) { dy = -0.27; STATE = jump; }
		}
		if (key["Up"]) {
			if (STATE == swim || STATE == climb) { dy = 0.05; }
			if (STATE == stay || STATE == walk) { dx = 0; STATE = duck; }
		}
		if (key["Up"]) {
			pop = true;
		}
		//ЕСЛИ КЛАВИША ОТПУЩЕНА:
		if (!(key["R"] || key["L"])) {
			dx = 0;
			if (STATE == walk) { STATE = stay; }
		}
		if (!(key["Up"] || key["Down"])) {
			if (STATE == swim || STATE == climb) { dy = 0; }
		}
		if (!key["Down"]) {
			if (STATE == duck) { STATE = stay; }
		}
		if (!key["Space"]) {
			pop = false;
		}
	}
	void			update(float time) {
		KeyCheck();
		if (STATE == stay) anim.set("stay");
		if (STATE == walk) anim.set("walk");
		if (STATE == jump) anim.set("jump");
		if (STATE == duck) anim.set("duck");
		if (STATE == climb) {
			anim.set("climb");
			anim.pause();
			if (dy != 0) { anim.play(); }
			if (!onLadder) { STATE = stay; }
		}
		if (pop) {
			anim.set("pop");
			if (STATE == walk) { anim.set("popAndWalk"); }
		}
		if (dir) {
			anim.flip();
			if (STATE == walk) { anim.set("popAndWalk"); }
		}
		x += dx * time;
		Collision(0);
		dy += 0.0005 * time;
		y += dy * time;
		Collision(1);
		anim.tick(time);
		key["R"] = key["L"] = key["Up"] = key["Down"] = key["Space"] = false;
	}
	void			Collision(int num) {
		for (int i = 0; i < obj.size(); i++)
			if (getRect().intersects(obj[i].rect)) {
				if (obj[i].name == "solid") {
					if (dy > 0 && num == 1) { y = obj[i].rect.top - h;  dy = 0;   STATE = stay; }
					if (dy < 0 && num == 1) { y = obj[i].rect.top + obj[i].rect.height;   dy = 0; }
					if (dx > 0 && num == 0) { x = obj[i].rect.left - w; }
					if (dx < 0 && num == 0) { x = obj[i].rect.left + obj[i].rect.width; }
				}
				if (obj[i].name == "ladder") {
					onLadder = true;
					if (STATE == climb)
						x = obj[i].rect.left - 10;
				}
				if (obj[i].name == "SlopeLeft") {
					FloatRect r = obj[i].rect;
					int y0 = (x + w / 2 - r.left) * r.height / r.width + r.top - h;
					if (y > y0)
						if (x + w / 2 > r.left) {
							y = y0; dy = 0; STATE = stay;
						}
				}
				if (obj[i].name == "SlopeRight") {
					FloatRect r = obj[i].rect;
					int y0 = -(x + w / 2 - r.left) * r.height / r.width + r.top + r.height - h;
					if (y > y0)
						if (x + w / 2 < r.left + r.width) {
							y = y0; dy = 0; STATE = stay;
						}
				}
			}
	}
};

#endif
