hellomake: *.cpp
	g++ *.cpp -o example -lsfml-audio -lsfml-system -lpthread -lSDL2 -lsfml-graphics -lsfml-window
