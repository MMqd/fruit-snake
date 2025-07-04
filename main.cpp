#include <iostream>
#include <string>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fstream>
#include <signal.h>
#include <thread>

//Used for getting user's home directory
#include  <sys/types.h>
#include  <pwd.h>

using namespace std;

bool draw=true;
string DrawBuffer="";
int aX=0;
int aY=0;
int dX=1;
int dY=0;
int sX=30;
int sY=30;
int X=sX/2;
int Y=sY/2;
int Tail[30][30];
int Length=3;
int Score=0;
int HighScore=0;
bool GameOver=false;
bool Pause=false;
string ScoreBefore="";
string Before="";
string After="";
char g[3]{0};

//Gets the user's local directory to store highscore
const string highscoreFile = string(getpwuid(getuid())->pw_dir) + "/.local/share/fruit-snake highscore";

struct termios old;

void Input(){
	while(true){
		read(0,&g,3);
		if(g[0]==27 && g[1]=='[' && 64<g[2] && g[2]<69){g[0]=g[2]-48;}
	}
}

void resizeHandler(int sig){
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	Before=string((w.ws_col-sX*2-2)/2,' ');
	After=string((w.ws_row-sY-2)/2+1,'\n');
	ScoreBefore="";
	for(int i=0; i<(sX-11)/2; i++){ScoreBefore+="  ";}
	draw=true;
}

void signalHandler(int signum) {
	tcsetattr(0, TCSANOW, &old);
	system("clear");
	cout<<"\e[?25h"<<flush;
	exit(signum);  
}

int main(int argc, const char* argv[]) {
	srand(time(0));

	if(argc!=1){
		if(argc>2){
			cout<<"Too many arguments"<<endl;
			return 0;
		} else if(string(argv[1])=="-h" || string(argv[1])=="--help") {
			cout<<
			"Simple Fruit Snake Game\n"
			"  Controls:\n"
			"    h/j/k/l, Arrow Left/Down/Up/Right, n/e/i/o - Turn left/down/up/right\n"
			"    p,ESC - Pause/Unpause\n"
			"    SPACE - Unpause/Restart game after game over"<<endl;
			return 0;
		} else if(argc!=1) {
			cout<<"Unknown argument: "+string(argv[1])<<endl;
			return 0;
		}
	}

	signal(SIGWINCH,resizeHandler);
	signal(SIGINT,signalHandler);
	resizeHandler(SIGWINCH);

	struct termios current;
	tcgetattr(0, &old); /* grab old terminal i/o settings */
	current = old; /* make new settings same as old settings */
	current.c_lflag &= ~ICANON; /* disable buffered i/o */
	current.c_lflag &= ~ECHO; /* set no echo mode */
	tcsetattr(0, TCSANOW, &current);

	ifstream Tmp(highscoreFile);
	Tmp>>HighScore;
	Tmp.close();

	aX=rand()%sX;
	aY=rand()%sY;

	thread(Input).detach();
	system("clear");
	cout<<"\e[1H\e[?25l";
	while(true){
		if(GameOver){
			if(g[0]==' '){
				X=sX/2;
				Y=sY/2;
				dX=1;
				dY=0;
				Length=3;
				Score=0;
				aX=rand()%sX;
				aY=rand()%sY;
				for(int j=0; j<sX; j++){
					for(int i=0; i<sY; i++){Tail[j][i]=0;}
				}
				GameOver=false;
			}
		} else {
			if(Pause){
				if(g[0]==' ' || g[0]=='\033' || g[0]=='p'){Pause=false;}
			} else {
				X+=dX;
				Y+=dY;
				for(int j=0; j<sX; j++){
					for(int i=0; i<sY; i++){
						if(Tail[j][i]>0){Tail[j][i]--;}
					}
				}
				if(X==-1 | Y==-1 | X==sX | Y==sY | Tail[X][Y]>0){
					GameOver=true;
					if(Score>HighScore){
						ofstream Tmp(highscoreFile);
						HighScore=Score;
						Tmp<<HighScore;
						Tmp.close();
						Score=0;
					}
				} else {Tail[X][Y]=Length;}

				if(X==aX && Y==aY){
					aX=rand()%sX;
					aY=rand()%sY;
					Score++;
					Length++;
				}

				draw=true;
				if(!GameOver){//Input
					if(g[0]=='h' || g[0]=='n' || g[0]==20){
						if(dX!=1){dX=-1;dY=0;}
					} else if(g[0]=='j' || g[0]=='e' || g[0]==18){
						if(dY!=-1){dX=0;dY=1;}
					} else if(g[0]=='k' || g[0]=='i' || g[0]==17){
						if(dY!=1){dX=0;dY=-1;}
					} else if(g[0]=='l' || g[0]=='o' || g[0]==19){
						if(dX!=-1){dX=1;dY=0;}
					} else if(g[0]=='p' || g[0]=='\033'){
						Pause=!Pause;draw=true;
					}
					
				}//Input
			}

			if(g[0]!=0){g[0]=0;}
		}

		if(draw){//Draw
			DrawBuffer="";

			for(int i=0; i<sY; i++){
				if(Y==i && X==-1){DrawBuffer+=Before+"\e[47m \e[41m \e[0m";
				} else{DrawBuffer+=Before+"\e[47m  \e[0m";}

				if(GameOver | Pause){
					string tmp="";
					if(i==sY/2){
						DrawBuffer+="\e[30;47m";
						if(GameOver){
							tmp=string((sX-6),' ');
							//for(int j=0; j<(sX-5)/2; j++){tmp+="  ";}
							DrawBuffer+=tmp+" GAME OVER  "+tmp;
						} else if(Pause){
							tmp=string((sX-6),' ');
							//for(int j=0; j<(sX-6)/2; j++){tmp+="  ";}
							DrawBuffer+=tmp+"GAME PAUSED "+tmp;
						}
						if(Y==i && X==sX){DrawBuffer+="\e[41m \e[47m \e[0m\n";
						} else{DrawBuffer+="\e[47m  \e[0m\n";}
						continue;
					} else if(i==sY/2-1 | i==sY/2+1){
						for(int j=0; j<sX; j++){tmp+="  ";}
						DrawBuffer+="\e[30;47m"+tmp;
						if(Y==i && X==sX){DrawBuffer+="\e[41m \e[47m \e[0m\n";
						} else{DrawBuffer+="\e[47m  \e[0m\n";}
						continue;
					}
				}


				if(GameOver){
					for(int j=0; j<sX; j++){
						if(j==aX && i==aY){
							DrawBuffer+="\e[0;41m  \e[0m";
						} else if(Tail[j][i]>0){
							if(X==j && Y==i){
								if(dX==1){DrawBuffer+="\e[41m \e[42m ";
								} else if(dX==-1) {DrawBuffer+="\e[42m \e[41m ";
								} else if(dY==1) {DrawBuffer+="\e[31;42m▀▀";
								} else if(dY==-1) {DrawBuffer+="\e[31;42m▄▄";}
							} else {DrawBuffer+="\e[42m  ";}
						} else{DrawBuffer+="\e[40m  ";}
					}
				} else {
					for(int j=0; j<sX; j++){
						if(j==aX && i==aY){DrawBuffer+="\e[0;41m  \e[0m";
						} else if(Tail[j][i]>0){DrawBuffer+="\e[42m  \e[0m";
						} else{DrawBuffer+="  ";}
					}

				}
				if(Y==i && X==sX){DrawBuffer+="\e[41m \e[47m \e[0m\n";
				} else{DrawBuffer+="\e[47m  \e[0m\n";}
			}

			string Top="";
			for(int i=0; i<sX+2; i++){Top+="  ";}
			if(Y==-1){
				string tmp="";
				for(int i=0; i<sX; i++){
					if(i==X){ tmp+="\e[31;47m▄▄";} else {tmp+="  ";}
				}
				tmp="\e[47m  "+tmp+"\e[47m  \e[0m";
				DrawBuffer=After+Before+"\e[0;47m"+tmp+"\e[0m\n"+ DrawBuffer
						   +Before+"\e[47m"+Top;

			}else if(Y==sY){
				string tmp="";
				for(int i=0; i<sX; i++){
					if(i==X){tmp+="\e[31;47m▀▀";} else {tmp+="  ";}
				}
				tmp="\e[47m  "+tmp+"\e[47m  \e[0m";
				DrawBuffer=After+Before+"\e[47m"+Top+"\e[0m\n"+ DrawBuffer
						   +Before+"\e[47m"+tmp;

			}else{
				DrawBuffer=After+Before+"\e[0;47m"+Top+"\e[0m\n"+ DrawBuffer
						   +Before+"\e[47m"+Top;
			}

			DrawBuffer+="\e[0m\n\n"+Before+ScoreBefore+"Score: "+to_string(Score)+"\tHigh Score: "+to_string(HighScore)+After;

			cout<<DrawBuffer<<flush;
			draw=false;
		}//Draw
		usleep(50000);
	}
}
