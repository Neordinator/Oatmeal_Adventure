#ifndef POPWAVE_H
#define POPWAVE_H

#include "Entity.hpp"

class	PopWave :public Entity {
public:
	int frame;
	PopWave(AnimationManager& a, Level& lev, int x, int y, bool dir) :Entity(a, x, y) {
		option("PopWave", 0.0, 10, "move");
		if (dir) dx = -0.0;
		//obj = lev.GetObjects("solid");
		frame = 3;
	}
	void	update(float time) {
		x += dx * time;
		frame -= 1;
		//for (int i = 0; i < obj.size(); i++) if (getRect().intersects(obj[i].rect)) { Health = 0; }
		if (frame <= 0) {
			anim.set("explode"); dx = 0;
			if (anim.isPlaying() == false)
				life = false;
		}
		anim.tick(time);
	}
};

#endif POPWAVE_H