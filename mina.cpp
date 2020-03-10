#include <fstream>
#include <iostream>
#include <string>
#include <Windows.h>
#include <conio.h>

const int MAX = 50;

typedef enum tCasilla { LIBRE, TIERRA, GEMA, PIEDRA, MURO, SALIDA, MINERO, DIMANITA };
typedef enum tTecla { ARRIBA, ABAJO, DCHA, IZDA, SALIR, NADA, TNT };

typedef tCasilla tCuadricula[MAX][MAX];

typedef struct tPlano {
	tCuadricula cuadricula;
	int nfilas;
	int nColumnas;
};

typedef struct tMina {
	tPlano plano;
	int playerX;
	int playerY;
	int score;
	int tnt;
};

boolean cargarMina(std::ifstream& fichero, tMina& mina, const std::string& level);
tTecla getAction();
void drawGame(const tMina& mina);
int getColorOfCasilla(const tCasilla& c);
boolean excuteAction(tMina& mina, const tTecla& action);
boolean move(tMina& mina, int x, int y);
void explosion(tMina& mina);

int main() {
	std::ifstream fichero;
	tMina mina;
	mina.score = 0;
	mina.tnt = 0;
	int level = 1;
	cargarMina(fichero, mina, std::to_string(level) +".txt");
	drawGame(mina);
	tTecla tecla = getAction();
	while (tecla != SALIR) {
		if (excuteAction(mina, tecla)) {
			level++;
			if (!cargarMina(fichero, mina, std::to_string(level) + ".txt")) {
				std::cout << "Can't load level file !" << std::endl;
				break;
			};
		}
		drawGame(mina);
		tecla = getAction();
	}
	system("pause");
	return 0;
}

boolean cargarMina(std::ifstream& fichero, tMina& mina, const std::string& level) {
	fichero.open(level);
	if (fichero.is_open()) {
		fichero >> mina.plano.nfilas;
		fichero >> mina.plano.nColumnas;
		char c;
		for (int i = 0; i < mina.plano.nfilas;i++) {
			for (int j = 0;j < mina.plano.nColumnas;j++) {
				do {
					fichero.get(c);
				} while (c == '\n');
				switch (c) {
				case ' ': mina.plano.cuadricula[i][j] = LIBRE; break;
				case 'T': mina.plano.cuadricula[i][j] = TIERRA; break;
				case 'G': mina.plano.cuadricula[i][j] = GEMA; break;
				case 'P': mina.plano.cuadricula[i][j] = PIEDRA; break;
				case 'M': mina.plano.cuadricula[i][j] = MURO; break;
				case 'S': mina.plano.cuadricula[i][j] = SALIDA; break;
				case 'J':
					mina.plano.cuadricula[i][j] = MINERO;
					mina.playerX = i;
					mina.playerY = j;
					break;
				default:
					break;
				}
			}
		}
		fichero.close();
		return true;
	}else {
		fichero.close();
		return false;
	}
}

tTecla getAction() {
	int input;
	while (true) {
		input = _getch();
		if (input == 224 || input == 0) {
			input = _getch();
			switch (input) {
			case 72: return ARRIBA;
			case 80: return ABAJO;
			case 75: return IZDA;
			case 77: return DCHA;
			default: return NADA;
			}
		} else {
			switch (input) {
			case 27: return SALIR;
			case 116: return TNT;
			default: return NADA;
			}
		}
	}
}

boolean excuteAction(tMina& mina, const tTecla& action) {
	boolean nextLevel = false;
	switch (action) {
	case ARRIBA:nextLevel = move(mina, -1, 0);break;
	case ABAJO:nextLevel = move(mina, 1, 0);break;
	case IZDA:nextLevel = move(mina, 0, -1);break;
	case DCHA:nextLevel = move(mina, 0, 1);break;
	case SALIR:break;
	case TNT:
		if (mina.tnt > 0) {
			explosion(mina);
			mina.tnt--;
		} else {
			std::cout << "Need Gem to make TNT" << std::endl;
		}
		break;
	}
	return nextLevel;
}


// return true if player found exit else return false.
boolean move(tMina& mina, int x, int y) {
	int tempX = mina.playerX + x;
	int tempY = mina.playerY + y;
	if (tempX >= 0 && tempX < MAX && tempY >= 0 && tempY < MAX) {
		tCasilla c = mina.plano.cuadricula[tempX][tempY];
		if (c == LIBRE || c == GEMA) {
			if (c == GEMA) {
				mina.score++;
				mina.tnt++;
			}
				
			mina.plano.cuadricula[tempX][tempY] = MINERO;
			mina.plano.cuadricula[mina.playerX][mina.playerY] = LIBRE;
			mina.playerX = tempX;
			mina.playerY = tempY;
		}else if (c == SALIDA) {
			return true;
		}
	}
	return false;
}

void explosion(tMina& mina) {
	int n[3] = { -1, 0, 1 };
	for (int i = 0; i < 3;i++) {
		for (int j = 0; j < 3;j++) {
			if (n[i] != 0 || n[j] != 0) {
				tCasilla c = mina.plano.cuadricula[mina.playerX + n[i]][mina.playerY + n[j]];
				if (c == TIERRA || c == GEMA) {
					mina.plano.cuadricula[mina.playerX + n[i]][mina.playerY + n[j]] = LIBRE;
				}
			}
		}
	}
	

}

void drawGame(const tMina& mina) {
	system("cls");
	const HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD wOldColorAttrs;
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	GetConsoleScreenBufferInfo(hConsole, &csbiInfo);
	wOldColorAttrs = csbiInfo.wAttributes;

	std::cout << "Controller: ← ↑ → ↓   TNT: T" << std::endl;
	for (int i = 0; i < mina.plano.nfilas;i++) {
		for (int j = 0; j < mina.plano.nColumnas;j++) {
			int color = getColorOfCasilla(mina.plano.cuadricula[i][j]);
			SetConsoleTextAttribute(hConsole, 16 * color + color);
			std::cout << "  ";
		}
		std::cout << std::endl;
	}
	SetConsoleTextAttribute(hConsole, wOldColorAttrs);
	std::cout << "Score: " << mina.score << "\tTNT: " << mina.tnt << std::endl;
}

int getColorOfCasilla(const tCasilla& c) {
	/*
	#define BLACK			0
	#define BLUE			1
	#define GREEN			2
	#define CYAN			3
	#define RED				4
	#define MAGENTA			5
	#define BROWN			6
	#define LIGHTGRAY		7
	#define DARKGRAY		8
	#define LIGHTBLUE		9
	#define LIGHTGREEN		10
	#define LIGHTCYAN		11
	#define LIGHTRED		12
	#define LIGHTMAGENTA	13
	#define YELLOW			14
	#define WHITE			15
	*/
	switch (c) {
	case LIBRE: return 0;
	case TIERRA: return 6;
	case GEMA: return 11;
	case PIEDRA: return 8;
	case MURO: return 7;
	case SALIDA: return 10;
	case MINERO: return 15;
	default: return 0;
	}
}