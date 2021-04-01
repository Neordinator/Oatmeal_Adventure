#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <list>
#include <sstream>
#include "Level.hpp"
#include "Animation.hpp"
#include "Entity.hpp"
#include "PopWave.hpp"
#include "Player.hpp"
#include "Enemy.hpp"
#include "MovingPlatform.hpp"
#include "HealthBar.hpp"
using namespace sf;

float offsetX = 0;
float offsetY = 0;

float screenCenterX = 0;
float screenCenterY = 0;

void	RunGame(int resolutionX, int resolutionY) {

	///////////// инициализация ///////////////////////////
	RenderWindow window(VideoMode(resolutionX, resolutionY), "POPcat Adventure");

	View view(FloatRect(0, 0, resolutionX, resolutionY));

	Level lvl;
	lvl.LoadFromFile("xml/levels/lvl1_lastVersion.tmx");

	Texture enemy_t, moveplatform_t, oatmeal_t, pop_t, bg;
	bg.loadFromFile("tiles/Background01.png");
	enemy_t.loadFromFile("tiles/Alien.png");
	moveplatform_t.loadFromFile("tiles/Caterpillar.png");
	oatmeal_t.loadFromFile("tiles/ovsyanka_total.png");
	pop_t.loadFromFile("tiles/pop_wave.png");


	AnimationManager anim;
	anim.loadFromXML("xml/animations/oatmeal_total.xml", oatmeal_t);
	anim.animList["jump"].loop = 0;

	AnimationManager anim2;
	anim2.loadFromXML("xml/animations/pop_wave.xml", pop_t);
	anim2.animList["move"].loop = 0;
	//anim2.create("move", pop_t, 7, 10, 8, 8, 1, 0);
	//anim2.create("explode", pop_t, 27, 7, 18, 18, 4, 0.01, 29, false);

	AnimationManager anim3;
	anim3.loadFromXML("xml/animations/alien.xml", enemy_t);
	//anim3.animList["hit"].loop = 0;
	//anim3.create("move", enemy_t, 0, 0, 16, 16, 2, 0.002, 18);
	//anim3.create("dead", enemy_t, 58, 0, 16, 16, 1, 0);

	AnimationManager anim4;
	anim4.loadFromXML("xml/animations/caterpillar.xml", moveplatform_t);
	//anim4.create("move", moveplatform_t, 0, 0, 95, 22, 1, 0);

	Sprite background(bg);
	//background.setOrigin(bg.getSize().x, bg.getSize().y);
	background.setOrigin(bg.getSize().x / 2, bg.getSize().y / 2);

	std::list<Entity*> entities;
	std::list<Entity*>::iterator it;

	std::vector<Object> e = lvl.GetObjects("Enemy");
	for (int i = 0; i < e.size(); i++)
		entities.push_back(new ENEMY(anim3, lvl, e[i].rect.left, e[i].rect.top));

	e = lvl.GetObjects("MovingPlatform");
	for (int i = 0; i < e.size(); i++)
		entities.push_back(new MovingPlatform(anim4, lvl, e[i].rect.left, e[i].rect.top));

	Object pl = lvl.GetObject("Player");
	PLAYER ovsyanka(anim, lvl, pl.rect.left, pl.rect.top);

	HealthBar healthBar;

	Clock clock;

	Font font;	//переменная шрифта
	font.loadFromFile("fonts/CyrilicOld.ttf");	//переменной шрифта передаём файл шрифта

	Text text("", font, 20); //-------------------------------создаем объект текст. закидываем в объект текст строку, шрифт, размер шрифта(в пикселях);//сам объект текст (не строка)
	//text.setColor(Color::Black); //-------------------------покрасили текст в красный. если убрать эту строку, то по умолчанию он белый
	text.setStyle(sf::Text::Bold | sf::Text::Underlined); //--жирный и подчеркнутый текст. по умолчанию он "худой":)) и не подчеркнутый
	std::ostringstream coordinates;
	
	//view.zoom(0.7);


	/*SoundBuffer pop;
	pop.loadFromFile("pop.ogg");
	Sound sound(pop);*/

	while (window.isOpen())/*	ОСНОВНОЙ ЦИКЛ	*/ {
		float time = clock.getElapsedTime().asMicroseconds();
		clock.restart();
		time = time / 500; //здесь регулируем скорость игры
		if (time > 40) time = 40;
		Event event;
		while (window.pollEvent(event)) {
			if (event.type == Event::Closed)
				window.close();
			if (event.type == Event::KeyPressed) {
				if (event.key.code == Keyboard::Space) {
					//sound.play();
					entities.push_back(new PopWave(anim2, lvl, ovsyanka.x + 60, ovsyanka.y + 20, ovsyanka.dir));
				}
			}
		}
		if (Keyboard::isKeyPressed(Keyboard::Left)) ovsyanka.key["L"] = true;
		if (Keyboard::isKeyPressed(Keyboard::Right)) ovsyanka.key["R"] = true;
		if (Keyboard::isKeyPressed(Keyboard::Up)) ovsyanka.key["Up"] = true;
		if (Keyboard::isKeyPressed(Keyboard::Down)) ovsyanka.key["Down"] = true;
		if (Keyboard::isKeyPressed(Keyboard::Space)) ovsyanka.key["Space"] = true;

////////////////////////////////////	СМЕЩЕНИЕ ЭКРАНА		//////////////////////////////////////////////////
		if (ovsyanka.x < 641)	{ offsetX = ovsyanka.x - resolutionX / 2; }									//
		if (ovsyanka.x > 3999)	{ offsetX = resolutionX / 2 - (4800 - ovsyanka.x); }						//
		if (ovsyanka.y > 1239)	{ offsetY = resolutionY / 2 - (1600 - ovsyanka.y); }						//
		if (ovsyanka.y < 361)	{ offsetY = ovsyanka.y - resolutionY / 2; }									//
		screenCenterX = ovsyanka.x - offsetX;																//
		screenCenterY = ovsyanka.y - offsetY;																//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

		for (it = entities.begin(); it != entities.end();) {
			Entity* b = *it;
			b->update(time);
			if (b->life == false) {
				it = entities.erase(it);
				delete b;
			}
			else it++;
		}
		ovsyanka.update(time);
		healthBar.update(ovsyanka.Health);

		for (it = entities.begin(); it != entities.end(); it++) {
			//1. враги
			if ((*it)->Name == "Enemy") {
				Entity* enemy = *it;

				if (enemy->Health <= 0) continue;

				if (ovsyanka.getRect().intersects(enemy->getRect()))
					if (ovsyanka.dy > 0) { enemy->dx = 0; ovsyanka.dy = -0.2; enemy->Health = 0; }
					else if (!ovsyanka.hit) {
						ovsyanka.Health -= 5; ovsyanka.hit = true;
						if (ovsyanka.dir) ovsyanka.x += 10; else ovsyanka.x -= 10;
					}
				for (std::list<Entity*>::iterator it2 = entities.begin(); it2 != entities.end(); it2++) {
					Entity* popWave = *it2;
					if (popWave->Name == "PopWave")
						if (popWave->Health > 0)
							if (popWave->getRect().intersects(enemy->getRect())) {
								popWave->Health = 0; enemy->Health -= 5;
							}
				}
			}
			//2. движущиеся платформы
			if ((*it)->Name == "MovingPlatform") {
				Entity* movPlat = *it;
				if (ovsyanka.getRect().intersects(movPlat->getRect()))
					if (ovsyanka.dy > 0)
						if (ovsyanka.y + ovsyanka.h < movPlat->y + movPlat->h) {
							ovsyanka.y = movPlat->y - ovsyanka.h + 3; ovsyanka.x += movPlat->dx * time; ovsyanka.dy = 0; ovsyanka.STATE = PLAYER::stay;
						}
			}
			//3.. и т.д.
		}
		
		/////////////////////отображаем на экран/////////////////////
		view.setCenter(screenCenterX, screenCenterY);
		window.setView(view);

		background.setPosition(view.getCenter());
		window.draw(background);

		lvl.Draw(window);

		for (it = entities.begin(); it != entities.end(); it++)
			(*it)->draw(window);

		ovsyanka.draw(window);
		healthBar.draw(window);

		/////////////////////////ПРОВЕРКА КООРДИНАТ И ЭКРАНА//////////////////////////////////////////
		coordinates << "\n\tX: " << ovsyanka.x << " Y: " << ovsyanka.y;								//
		coordinates << "\nСмещение:\n" << "\tX: " << offsetX << " Y: " << offsetY;					//
		coordinates << "\nЦентр Экрана:\n" << "\tX: " << screenCenterX << " Y: " << screenCenterY;	//
		text.setString("Координаты: " + coordinates.str());//задает строку тексту					//
		text.setPosition(view.getCenter());//задаем позицию текста, сверху по центру				//
		window.draw(text);//рисую этот текст														//
		coordinates.str("");																		//
		//////////////////////////////////////////////////////////////////////////////////////////////

		if(ovsyanka.x > 640 && ovsyanka.x < 4000) offsetX = 0;
		if(ovsyanka.y > 360 && ovsyanka.y < 1240) offsetY = 0;

		window.display();
	}
}
#endif GAME_H