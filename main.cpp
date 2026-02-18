//#include <cstdio>
//#include <cstdlib>
#include <asm-generic/ioctls.h>
//#include <cctype>
#include <cstdlib>
#include <ncurses.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <string.h>
#include <fstream>
#include <bits/stdc++.h>
//#include <stdio.h>

class TextEditor{
 
 int x;
 int y;
 int screenrows;
 int screencols;
 std::vector<std::string> lines;
 struct termios original_state;
 int open_flag = 0;
 const char* filename;
 enum navigate{
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN
 };

public:

// ioctl systemcall me TIOCGWINSZ request
 void getWindowSize(int* rows, int* cols){
  struct winsize size;
  if(ioctl(STDIN_FILENO, TIOCGWINSZ, &size) == -1){
   return;
  }else {
   *rows = size.ws_row;
   *cols = size.ws_col;
  }
 }
 
void openFile(const char* filename) {
 std::ifstream file(filename);
 std::string line;

 if (file.is_open()) {
   while (std::getline(file, line)) {
   lines.push_back(line);
  }
  file.close();
 }

 if (lines.empty()) {
  lines.push_back("");
 }
}

void saveAndExit(const char* filename){
 std::ofstream file(filename);

 if(file.is_open()){
  for(const auto& line : lines){
   file << line << "\r\n";
  }
  file.close();
 }
}

void newAndExit(){
 int random;
 srand(time(0));
 random = rand();
 std::string filename = std::to_string(random) + ".txt";
 std::ofstream file(filename);
 saveAndExit(filename.c_str());
}
 // Constructor, abhi ke liye sirf default
 TextEditor(){
  clearScreen();
  y = 1;
  getWindowSize(&screenrows, &screencols);
  enableRawMode();
  lines.push_back("");
  x = 2;
  //write(STDOUT_FILENO, "\x1b[1;0H", 6);
  
 }

 
 TextEditor(const char* filename){
  this-> filename = filename;
  open_flag = 1;
  clearScreen();
  y = 1;
  x = 2;
  getWindowSize(&screenrows, &screencols);
  enableRawMode();
  //lines.push_back("~ ");
  openFile(filename);
 }

 ~TextEditor(){
  disableRawMode();
 }

 void editor_exit(){
  //write(STDOUT_FILENO, "Hello", 5);
  if (open_flag) {
   saveAndExit(this->filename);
  }else {
   newAndExit();
  }
  clearScreen();
  disableRawMode();
  exit(0);
 }

 void disableRawMode(){
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_state);
 }

 void enableRawMode(){
  tcgetattr(STDIN_FILENO, &original_state);

  struct termios raw = original_state;
    
  // Next 5 lines are copy pasted
  raw.c_iflag &= ~(IXON | ICRNL);
  raw.c_oflag &= ~(OPOST);
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
 }

 void clearScreen(){
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", 1, 1);
  write(STDOUT_FILENO, buf, strlen(buf));
 }

 int getKey(){
  char ipt;
  int nread = read(STDIN_FILENO, &ipt, 1);
  if (nread != 1) {
   return 0;
  }
  if(ipt != '\x1b'){// agar escape sequence nahi hai toh simple key return kardo
   return ipt;
  }

  char next[2];
  if(read(STDIN_FILENO, &next[0], 1) != 1) return '\x1b';
  if(read(STDIN_FILENO, &next[1], 1) != 1) return '\x1b';

  if (next[0] == '[') {
   switch (next[1]) {
    case 'A': return ARROW_UP;
    case 'B': return ARROW_DOWN;
    case 'C': return ARROW_RIGHT;
    case 'D': return ARROW_LEFT;
   }
  }
  return '\x1b';
 }


 void listenForInput(){
  int key = getKey();

  //lines.at(y-1) += "\n";
  switch (key) {
   case '\r':  
    lines.insert(lines.begin() + y, "");
    y++;
    x = 2;
    break;
   case '\x1b': editor_exit();
   case ARROW_DOWN: if(this->y < (int)lines.size()) this->y++; break;
   case ARROW_UP: if(this->y > 1) this->y--; break;
   case ARROW_RIGHT: if(this->x <= lines.at(y - 1).length()+ 1) this->x++; break;
   case ARROW_LEFT: if(this->x > 2) this->x--; break;
   default:
    if (key == 127) {
     if (x > 2) {
       lines.at(y-1).erase(x - 3, 1); 
       x--;
      }
    } 
    else if (key >= 32 && key <= 255) {
     if (x - 2 <= lines.at(y-1).length()) {
      lines.at(y-1).insert(x - 2, 1, (char)key);
      x++;
     }
    }
   }
   editorScreenRefresh();
   return;
}

 void editorScreenRefresh(){
  
  clearScreen();
  for(int i = 0; i < lines.size(); i++){
   write(STDOUT_FILENO, "~ ", 2);
   write(STDOUT_FILENO, lines.at(i).c_str(), lines.at(i).length());
   write(STDOUT_FILENO, "\r\n", 2);
  }
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", y, x+1);
  write(STDOUT_FILENO, buf, strlen(buf));
 
 }
};


int main(int argc, char* argv[]) {

 if (argc > 1) {
  TextEditor atex(argv[1]);
  atex.editorScreenRefresh();
  while (true) {
    atex.listenForInput();
   }

  } else {

   TextEditor atex; 
   atex.editorScreenRefresh();
   while (true) {
    atex.listenForInput();
   }

  }
 
 return 0;
}