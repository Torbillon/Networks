#include <getopt.h>
#include <string.h>
//
#include <signal.h>
#include <sys/types.h>
// #include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
/* select */
#include <sys/select.h>
/* curses */
#include <curses.h>
#include <locale.h>
#define PROTOPORT 36764 /* assigned protocol port number */
#define QLEN 6 /* size of request queue */
#define NUMWINS 7
#define RES_BUF_SIZE 80
WINDOW *w[NUMWINS];
WINDOW *sw[NUMWINS];
struct cursor {
  int x[7];
  int y[7];
};
struct connection {
  char *localPort;
  char *remotePort;
  char *specialPort;
  char *localAddr;
  char *remoteAddr;
  char *specialAddr;
  char *dotAddr;
  int sd; int sdw;
  char listening, pass;
};
typedef struct cursor cursor;
typedef struct connection connection;
struct info {
  connection rConnect;
  connection lConnect;
  fd_set tds;
  fd_set wds;
  int index;
  char *file;
  char insert,command;
  char outputl,outputr;
  char dsplr, dsprl;
  char persl, persr;
  char source;
  char loopr, loopl;
  char left, right; /* do I have left or right or did I disconnect? */
};

typedef struct info info;
void delta(char ch, int *index, char *buf);
void getChar(struct info *omega, char ch, char left, char right, char *bul);
void transmitSock(int sdL, int sdR, char outputl, char outputr, char *buf, char left, char right, struct info *omega);
void action(struct info *omega, char *bul);
void itoa(int i, char *p);
char isDigit(char *str);
void aggrConnect(struct info *omega, char left);
void passConnect(struct info *omega, char left);
void fdReset(info *omega);
void getInfo(info *omega, char left, char flag);
void update_win(int i);
void report(char *s);
void cleanBuff(char *buf, char flag);
void bozzo(int bozo);
struct info begin;
/* big ass bozo counter*/
int bozo = 0;
char STRLR = 0, STRRL = 0, STRLRX = 0, STRRLX = 0;
FILE *leftOutFile;
FILE *rightOutFile;
FILE *leftInFile;
FILE *rightInFile;
char EXTLR = 0, EXTRL = 0;
int fds1[2];
int fds2[2];
int fds3[2];
int fds4[2];
pid_t pid;
pid_t pid1;
FILE* stream1;
FILE *stream11;
FILE* stream2;
FILE* stream22;
int
main(argc, argv)
int argc;
char *argv[];
{
  /* right side connection variables */
  char noRight = 0,rPortF = 0, noRPort = 0, rAddrF = 0; /* flags to determine correctness of arguments */
  char *rAddr = malloc(32); strcpy(rAddr,""); /* address to connect to */
  char *rPort = malloc(32); strcpy(rPort,""); /* port string argument */
  leftOutFile = NULL; rightOutFile = NULL; leftInFile = NULL; rightInFile = NULL;
  /* left side connection variables */
  struct sockaddr_in lad; /* structure to hold left piggy address */
  char lPortF = 0, noLeft = 0, noLPort = 0; /* flags to determine correctness of arguments */
  int alen; /* length of address */
  char *lPort = malloc(32); strcpy(lPort,""); /* port */

  /* other variables */
  /* select function variables */
  fd_set fds, wds, tds; int rval;
  /* getopt variables */
  extern char *optarg; extern int optind; int opt;
  char bufr[1025], bul[1025]; memset(bufr,0,sizeof(bufr)); memset(bul,0,sizeof(bul)); /* buffer for socket and user input data */
  char bufl[1025]; memset(bufl,0,sizeof(bufl));
  char buf1[1025]; memset(buf1,0,sizeof(buf1));
  char buf2[1025]; memset(buf2,0,sizeof(buf2));
  struct info omega;
  memset((char *)&begin,0,sizeof(begin));
  struct cursor curse; memset((char *)&curse,0,sizeof(curse));
  /* command variables */
  char source = 0, persl = 0, persr = 0, dsplr = 0, dsprl = 0, loopr = 0, loopl = 0;
  char outputl = 0, outputr = 0, output = 0, display, dropr = 0, dropl = 0;
  char *filportle = NULL;
  /* Structure for long_options */
  static struct option long_options[] =
  {
    {"rraddr",required_argument,NULL,'a'},
    {"rrport",optional_argument,NULL,'b'},
    {"llport",optional_argument,NULL,'c'},
    {"noleft",no_argument,NULL,'d'},
    {"noright",no_argument,NULL,'e'},
    {"persl",no_argument,NULL,'g'},
    {"persr",no_argument,NULL,'h'},
    {"dsplr",no_argument,NULL,'i'},
    {"dsprl",no_argument,NULL,'j'},
    {"loopr",no_argument,NULL,'k'},
    {"loopl",no_argument,NULL,'l'},
    {NULL,0,NULL,0}
  };
  /* Check for no arguments */

  while((opt = getopt_long_only(argc,argv,"a:b:c:deghijkls",long_options,NULL)) > 0) {
    switch(opt) {
      case 'a':
	    if(rAddrF || noRight)
	      bozo++;
	    strcpy(rAddr,optarg);
	    rAddrF = 1;
	    break;
      case 'b':
	    if(rPortF || noRight)
        bozo++;
	    if(optarg == 0)
	      noRPort = 1;
	    else
	      strcpy(rPort,optarg);
	    rPortF = 1;
	    break;
      case 'c':
	    if(lPortF || noLeft)
	      bozo++;
	    if(optarg == 0)
	      noLPort = 1;
	    else
	      strcpy(lPort,optarg);
	    lPortF = 1;
	    break;
      case 'd':
	    if(noLeft || lPortF)
	      bozo++;
	    noLeft = 1;
	    break;
      case 'e':
	    if(noRight || rAddrF || rPortF)
	      bozo++;
	    noRight = 1;
	    break;
      case 'g':
  	  if(persl)
  	    bozo++;
  	  persl = 1;
  	  break;
      case 'h':
    	if(persr)
    	  bozo++;
    	persr = 1;
    	break;
      case 'i':
      if(dsplr || dsprl)
        bozo++;
      dsplr = 1;
      break;
      case 'j':
      if(dsprl || dsplr)
        bozo++;
      dsprl = 1;
      break;
      case 'k':
      if(loopr)
        bozo++;
      loopr = 1;
      break;
      case 'l':
      if(loopl)
        bozo++;
      loopl = 1;
      break;
      case '?':
      default:
	    printf("Invalid argument(s).\n");
	    exit(EXIT_FAILURE);
    }
  }
  if(argc == 1) {
    noRight = 1;
    noLeft = 1;
  }
  if((noLeft && noRight) || ((!noRight) && (!rAddrF)))
    ++bozo;
  if(bozo) {
    bozzo(bozo);
    printf("Setting to default settings\n");
    dsplr = 0; dsprl = 0; persl = 0; persr = 0; source = 0;
  }
  if(!rPortF)
    noRPort = 1;
  if(!lPortF)
    noLPort = 1;
  /* Create fd_set containing sockets and stdin */

  FD_ZERO(&omega.tds);
  FD_ZERO(&omega.wds);
  FD_SET(0,&omega.tds);
  /* setup everything for ncurses */
  int i,j;
  char response[RES_BUF_SIZE];
  int WPOS[NUMWINS][4]= { {16,66,0,0},{16,66,0,66},{16,66,16,0},{16,66,16,66},
    {3,132,32,0},{5,132,35,0},{3,132,40,0} };
  setlocale(LC_ALL,"");
  initscr();
  cbreak();
  noecho();
  nonl();
  intrflush(stdscr,FALSE);
  keypad(stdscr,TRUE);
  clear();
  if (!(LINES==43) || !(COLS==132) ) {
    move(0,0);
    addstr("Piggy3 requires a screen size of 132 columns and 43 rows");
    move(1,0);
    addstr("Set screen size to 132 by 43 and try again");
    move(2,0);
    addstr("Press enter to terminate program");
    refresh();
    getstr(response);            // Pause so we can see the screen
    endwin();
    exit(EXIT_FAILURE);
  }
  // create the 7 windows and the seven subwindows
  for(i=0;i<NUMWINS;i++) {
    w[i]=newwin(WPOS[i][0],WPOS[i][1],WPOS[i][2],WPOS[i][3]);
    sw[i]=subwin(w[i],WPOS[i][0]-2,WPOS[i][1]-2,WPOS[i][2]+1,WPOS[i][3]+1);
    scrollok(sw[i],TRUE);
    wborder(w[i],0,0,0,0,0,0,0,0);
    touchwin(w[i]);
    wrefresh(w[i]);
    wrefresh(sw[i]);
  }
  /* initialize struct omega */
  omega.index = 0; omega.insert = 0; omega.command = 1;
  omega.outputl = 0; omega.outputr = 1; omega.dsplr = dsplr;
  omega.dsprl = dsprl; omega.persl = persl; omega.persr = persr;
  omega.loopr = loopr; omega.loopl = loopl; omega.right = 0; omega.left = 0;
  omega.rConnect.remoteAddr = malloc(64); strcpy(omega.rConnect.remoteAddr,rAddr);
  omega.rConnect.remotePort = malloc(64); strcpy(omega.rConnect.remotePort,rPort);
  omega.rConnect.localPort = malloc(64);
  omega.rConnect.localAddr = malloc(64);
  omega.lConnect.localPort = malloc(64);
  omega.lConnect.localAddr = malloc(64);
  //omega.rConnect.remoteAddr = rAddr; omega.rConnect.remotePort = rPort;
  omega.rConnect.specialAddr = malloc(64); strcpy(omega.rConnect.specialAddr,"");
  omega.rConnect.specialPort = malloc(64); strcpy(omega.rConnect.specialPort,"");
  omega.lConnect.specialAddr = malloc(64); strcpy(omega.lConnect.specialAddr,"");
  omega.lConnect.specialPort = malloc(64); strcpy(omega.lConnect.specialPort,"");
  //omega.rConnect.specialAddr = ""; omega.rConnect.specialPort = "";
  //omega.lConnect.specialAddr = ""; omega.lConnect.specialPort = "";
  omega.lConnect.remoteAddr = malloc(64); strcpy(omega.lConnect.remoteAddr,"");
  omega.lConnect.remotePort = malloc(64); strcpy(omega.lConnect.remotePort,"");
  //omega.lConnect.remoteAddr = ""; omega.lConnect.remotePort = "";
  omega.lConnect.dotAddr = omega.rConnect.dotAddr = malloc(64); strcpy(omega.rConnect.dotAddr,"");
  omega.lConnect.listening = omega.lConnect.pass = omega.rConnect.listening = omega.rConnect.pass = 0;
  // for begin
  begin.rConnect.remoteAddr = malloc(64); strcpy(begin.rConnect.remoteAddr,rAddr);
  begin.rConnect.remotePort = malloc(64); strcpy(begin.rConnect.remotePort,rPort);
  begin.rConnect.localPort = malloc(64);
  begin.rConnect.localAddr = malloc(64);
  begin.lConnect.localPort = malloc(64);
  begin.lConnect.localAddr = malloc(64);
  //omega.rConnect.remoteAddr = rAddr; omega.rConnect.remotePort = rPort;
  begin.rConnect.specialAddr = malloc(64); strcpy(omega.rConnect.specialAddr,"");
  begin.rConnect.specialPort = malloc(64); strcpy(omega.rConnect.specialPort,"");
  begin.lConnect.specialAddr = malloc(64); strcpy(omega.lConnect.specialAddr,"");
  begin.lConnect.specialPort = malloc(64); strcpy(omega.lConnect.specialPort,"");
  //omega.rConnect.specialAddr = ""; omega.rConnect.specialPort = "";
  //omega.lConnect.specialAddr = ""; omega.lConnect.specialPort = "";
  begin.lConnect.remoteAddr = malloc(64); strcpy(omega.lConnect.remoteAddr,"");
  begin.lConnect.remotePort = malloc(64); strcpy(omega.lConnect.remotePort,"");
  //omega.lConnect.remoteAddr = ""; omega.lConnect.remotePort = "";
  begin.lConnect.dotAddr = begin.rConnect.dotAddr = malloc(64); strcpy(begin.rConnect.dotAddr,"");
  begin.lConnect.listening = begin.lConnect.pass = begin.rConnect.listening = begin.rConnect.pass = 0;
  if(strcmp(rPort,"") == 0) {
    strcpy(omega.rConnect.remotePort,"");
  }
  else {
    strcpy(begin.rConnect.remotePort,rPort);
  }
  if(strcmp(lPort,"") == 0) {
    strcpy(omega.lConnect.localPort,"");
  } else {
    strcpy(omega.lConnect.localPort,lPort);
  }
  strcpy(omega.lConnect.localAddr,"");
  strcpy(begin.rConnect.localAddr,"");
  if(!noRight && rAddrF) {
    aggrConnect(&omega,0);
  }
  if(!noLeft) {
    passConnect(&omega,1);
  }
  if(omega.left)
    FD_SET(omega.lConnect.sdw,&omega.tds);
  if(omega.right)
    FD_SET(omega.rConnect.sd,&omega.tds);
  char x = 0, y = 0;
  begin = omega;
  /*struct timeval tiempo; memset((char *) &tiempo,0,sizeof(tiempo));
  tiempo.tv_sec = 3.0;
  tiempo.tv_usec = 0;
  struct timeval temp;
  */
  if(omega.right) {
    omega.outputr = 1;
    omega.outputl = 0;
  }
  else if(omega.left) {
    omega.outputr = 0;
    omega.outputl = 1;
  }
  int go1 = 0;
  int tempo = 0;
  while(1) {
    wmove(sw[6],0,0);
    fds = omega.tds;
    wds = omega.wds;
    if((!omega.left && !omega.right) || (!omega.left && omega.rConnect.listening) || (omega.lConnect.listening && !omega.right)) {tempo = 99999;}
    if(select(FD_SETSIZE,&fds,&wds,NULL,NULL) >= 0) {
      if(EXTLR) {
        if(FD_ISSET(fds2[0],&fds)) {
          int n = read(fds2[0],buf1,sizeof(buf1));
          if(n > 0) {
            if(leftOutFile != NULL)
              fputs(buf1,leftOutFile);
            if(omega.right) {
              if(FD_ISSET(omega.rConnect.sd,&wds)) {
                wprintw(sw[1],buf1); wmove(sw[6],0,0);
                transmitSock(omega.lConnect.sd,omega.rConnect.sd,0,1,buf1,omega.left,omega.right,&omega);
              }
            }
          }
          memset(buf1,0,sizeof(buf1));
        }
      }
      if(EXTRL) {
        if(FD_ISSET(fds4[0],&fds)) {
          int n = read(fds4[0],buf2,sizeof(buf2));
          if(n > 0) {
            if(rightOutFile != NULL)
              fputs(buf2,rightOutFile);
            if(omega.left) {
              if(FD_ISSET(omega.lConnect.sd,&wds)) {
                wprintw(sw[2],buf2); wmove(sw[6],0,0);
                transmitSock(omega.lConnect.sd,omega.rConnect.sd,1,0,buf2,omega.left,omega.right,&omega);
              }
            }
          }
          memset(buf1,0,sizeof(buf1));
        }
      }
      if(FD_ISSET(0,&fds)) {
        char ch = getch();
        wmove(sw[6],0,0);
        for (i=0;i<NUMWINS;i++) {update_win(i);}
        char but[2] = {ch,'\0'};
        char input = omega.insert;
        getChar(&omega,ch,omega.left,omega.right,bul);
        if(input && omega.insert) {
          wprintw(sw[5],but);
          update_win(i);
        }
      }
      if(omega.right) {
        if(omega.rConnect.listening) {
          if(FD_ISSET(omega.rConnect.sdw,&fds)) {
            if((omega.rConnect.sd = accept(omega.rConnect.sdw,(struct sockaddr *)&lad,&alen)) > 0) {
               if(strcmp(omega.rConnect.specialAddr,"") != 0) {
                struct sockaddr_in peer; socklen_t len = sizeof(peer);
                getpeername(omega.rConnect.sd,(struct sockaddr *) &peer,&len);
                if(strcmp(inet_ntoa(peer.sin_addr),omega.rConnect.specialAddr) == 0) {
                  close(omega.rConnect.sdw);
                  omega.rConnect.listening = 0;
                  getInfo(&omega,0,0);
                } else {
                  close(omega.rConnect.sdw);
                  close(omega.rConnect.sd);
                  omega.right = omega.rConnect.pass = omega.rConnect.listening = 0;
                  passConnect(&omega,0);
                  fdReset(&omega);
                  memset((char *)&lad,0,sizeof(lad));
                  wmove(sw[6],0,0);
                  wprintw(sw[6],"SpecialAddress authentification denied"); wmove(sw[6],0,0);
                  for (i=0;i<NUMWINS;i++) {update_win(i);}
                  continue;
                }
              } else {
                close(omega.rConnect.sdw);
                omega.rConnect.listening = 0;
                if(!x) {getInfo(&omega,0,1); x = 1;}
              }
              fdReset(&omega);
            }
            memset((char *)&lad,0,sizeof(lad));
          }
        } else {
          if(FD_ISSET(omega.rConnect.sd,&fds)) {
            if(read(omega.rConnect.sd,bufr,sizeof(bufr)) == 0) {
              close(omega.rConnect.sd);
              omega.right = omega.rConnect.listening = omega.rConnect.pass = 0;
              fdReset(&omega);
            } else {
              wprintw(sw[3],bufr);
              wmove(sw[6],0,0);
              if(rightInFile != NULL)
                fputs(bufr,rightInFile);
              if(STRRL || STRRLX) {
                cleanBuff(bufr,STRRLX);
                if(rightOutFile != NULL)
                  fputs(bufr,rightOutFile);
              }
              if(omega.loopl) {
                wprintw(sw[1],bufr);
                wmove(sw[6],0,0);
                if(FD_ISSET(omega.rConnect.sd,&wds))
                  transmitSock(omega.lConnect.sd,omega.rConnect.sd,0,1,bufr,omega.left,omega.right,&omega);
              }
              if(omega.left) {
                if(EXTRL) {
                  if(FD_ISSET(fds3[1],&wds))
                    write(fds3[1],bufr,strlen(bufr));
                } else {
                  wprintw(sw[2],bufr);
                  wmove(sw[6],0,0);
                  if(FD_ISSET(omega.lConnect.sd,&wds))
                    transmitSock(omega.lConnect.sd,omega.rConnect.sd,1,0,bufr,omega.left,omega.right,&omega);
                }
              }
              memset(bufr,0,sizeof(bufr));
            }
            memset(bufr,0,sizeof(bufr));
          }
        }
      }
      if(omega.left) {
        if(omega.lConnect.listening) {
          if(FD_ISSET(omega.lConnect.sdw,&fds)) {
            if((omega.lConnect.sd = accept(omega.lConnect.sdw,(struct sockaddr *)&lad,&alen)) > 0) {
              char a,b;
              if((a = strcmp(omega.lConnect.specialAddr,"")) != 0 || (b = strcmp(omega.lConnect.specialPort,"")) != 0) {
                char port2[6]; memset(port2,0,sizeof(port2));
                struct sockaddr_in peer; socklen_t len = sizeof(peer);
                getpeername(omega.lConnect.sd,(struct sockaddr *) &peer,&len);
                itoa((int)ntohs(peer.sin_port),port2);
                if(a && strcmp(inet_ntoa(peer.sin_addr),omega.lConnect.specialAddr) == 0) {
                  close(omega.lConnect.sdw);
                  omega.lConnect.listening = 0;
                  getInfo(&omega,1,0);
                } else if (a && strcmp(inet_ntoa(peer.sin_addr),omega.lConnect.specialAddr) != 0) {
                  close(omega.lConnect.sdw);
                  close(omega.lConnect.sd);
                  omega.left = omega.lConnect.pass = omega.lConnect.listening = 0;
                  passConnect(&omega,1);
                  fdReset(&omega);
                  memset((char *)&lad,0,sizeof(lad));
                  wmove(sw[6],0,0);
                  wprintw(sw[6],"SpecialAddress authentification denied"); wmove(sw[6],0,0);
                  for (i=0;i<NUMWINS;i++) {update_win(i);}
                  continue;
                }
                if(b && strcmp(omega.lConnect.specialPort,port2) == 0) {
                  close(omega.lConnect.sdw);
                  omega.lConnect.listening = 0;
                  getInfo(&omega,1,0);
                } else if (b && strcmp(omega.lConnect.specialPort,port2) != 0) {
                  close(omega.lConnect.sdw);
                  close(omega.lConnect.sd);
                  omega.left = omega.lConnect.pass = omega.lConnect.listening = 0;
                  passConnect(&omega,1);
                  fdReset(&omega);
                  memset((char *)&lad,0,sizeof(lad));
                  wprintw(sw[6],"SpecialPort authentification denied"); wmove(sw[6],0,0);
                  for (i=0;i<NUMWINS;i++) {update_win(i);}
                  continue;
                }
              } else {
                close(omega.lConnect.sdw);
                omega.lConnect.listening = 0;
                if(!y) {getInfo(&omega,1,1); y = 1;}
              }
              fdReset(&omega);
            }
            memset((char *)&lad,0,sizeof(lad));
          }
        } else {
          if(FD_ISSET(omega.lConnect.sd,&fds)) {
            if(read(omega.lConnect.sd,bufl,sizeof(bufl)) == 0) {
              close(omega.lConnect.sd);
              omega.left = omega.lConnect.listening = omega.lConnect.pass = 0;
              fdReset(&omega);
            } else {
              wprintw(sw[0],bufl);
              if(leftInFile != NULL)
                fputs(bufl,leftInFile);
              if(STRLR || STRLRX) {
                cleanBuff(bufl,STRLRX);
                if(leftOutFile != NULL)
                  fputs(bufl,leftOutFile);
              }
              if(omega.loopr) {
                wprintw(sw[2],bufl); wmove(sw[6],0,0);
                if(FD_ISSET(omega.lConnect.sd,&wds))
                  transmitSock(omega.lConnect.sd,omega.rConnect.sd,1,0,bufl,omega.left,omega.right,&omega);
              }
              if(omega.right) {
                if(EXTLR) {
                  if(FD_ISSET(fds1[1],&wds)) {
                    write(fds1[1],bufl,sizeof(bufl));
                  }
                } else {
                  wprintw(sw[1],bufl); wmove(sw[6],0,0);
                  if(FD_ISSET(omega.rConnect.sd,&wds))
                    transmitSock(omega.lConnect.sd,omega.rConnect.sd,0,1,bufl,omega.left,omega.right,&omega);
                }
              }
            }
            memset(bufl,0,sizeof(bufl));
          }
        }
      }
    }
    tempo++;
    wmove(sw[6],0,0);
    if((tempo%=100000) == 0) {
      for (i=0;i<NUMWINS;i++) {update_win(i);}
    }
  }
}
void getChar(struct info *omega, char ch, char left, char right, char *bul) {
  int index = omega->index, sdL = omega->lConnect.sd, sdR = omega->rConnect.sd;
  char ins = omega->insert, outputl = omega->outputl, outputr = omega->outputr;
  char buf[1025]; memset(buf,0,sizeof(buf));
  buf[0] = ch;
  switch(ch) {
    case 113: // q
    if(ins)
      transmitSock(sdL,sdR,outputl,outputr,buf,left,right,omega);
    else
      delta(ch,&omega->index,bul);
    break;
    case 105: // i
    if(ins)
      transmitSock(sdL,sdR,outputl,outputr,buf,left,right,omega);
    else
      delta(ch,&omega->index,bul);
    break;
    case 27: // esc
    if(ins) {
      omega->insert = 0;
      omega->command = 1;
    } else
      delta(ch,&omega->index,bul);
    break;
    case 13: // enter
    if(ins)
      transmitSock(sdL,sdR,outputl,outputr,buf,left,right,omega);
    else {
      bul[index] = '\0';
      action(omega,bul);
    }
    break;
    default:
    if(ins)
      transmitSock(sdL,sdR,outputl,outputr,buf,left,right,omega);
    else
      delta(ch,&omega->index,bul);
  }
}
void transmitSock(int sdL, int sdR, char outputl, char outputr, char *buf, char left, char right, struct info *omega) {
  if(outputl && left && FD_ISSET(omega->lConnect.sd,&omega->wds)) {
    write(sdL,buf,strlen(buf));
  }
  else if(outputr && right && FD_ISSET(omega->rConnect.sd,&omega->wds)) {
    write(sdR,buf,strlen(buf));
  }
  if(outputr && !right)
    bozzo(++bozo);
  else if(outputl && !left)
    bozzo(++bozo);
}
void delta(char ch, int *index, char *buf) {
  wclear(w[4]);
  wclear(sw[4]);
  wborder(w[4],0,0,0,0,0,0,0,0);
  wmove(sw[4],0,0);
  //wprintw(sw[4],"                                                                                                                                                                                                                                      ");
  int ind = *index;
  if(ch == 7) {
    if(ind > 0) {
      buf[ind-1] = 0;
      (*index)--;
    }
  }
  else {
    buf[*index] = ch;
    (*index)++;
  }
  //wmove(sw[4],0,0);

  wprintw(sw[4],buf);
}
void action(struct info *omega, char *bul) {
  wclear(w[6]);
  wclear(sw[6]);
  wborder(w[6],0,0,0,0,0,0,0,0);
  update_win(6);
  if(strcmp(bul,"i") == 0) {
    omega->insert = 1; omega->command = 0;
  } else if((strcmp(bul,"stlrnpon") == 0)) {
    STRLR = 1; STRLRX = 0;
  } else if((strcmp(bul,"stlrnpoff") == 0)) {
    STRLR = 0;
  } else if((strcmp(bul,"strlnpon") == 0)) {
    STRRL = 1; STRRLX = 0;
  } else if((strcmp(bul,"strlnpoff") == 0)) {
    STRRL = 0;
  } else if((strcmp(bul,"stlrnpxeolon") == 0)) {
    STRLRX = 1; STRLR = 0;
  } else if((strcmp(bul,"stlrnpxeoloff") == 0)) {
    STRLRX = 0;
  } else if((strcmp(bul,"strlnpxeolon") == 0)) {
    STRRLX = 1; STRRL = 0;
  } else if((strcmp(bul,"strlnpxeoloff") == 0)) {
    STRRLX = 0;
  } else if((strcmp(bul,"loglrpreoff") == 0)) {
    if(leftInFile != NULL)
      fclose(leftInFile);
    leftInFile = NULL;
  } else if((strcmp(bul,"logrlpreoff") == 0)) {
    if(rightInFile != NULL)
      fclose(rightInFile);
    rightInFile = NULL;
  } else if((strcmp(bul,"loglrpostoff") == 0)) {
    if(leftOutFile != NULL)
      fclose(leftOutFile);
    leftOutFile = NULL;
  } else if((strcmp(bul,"logrlpostoff") == 0)) {
    if(rightOutFile != NULL)
      fclose(rightOutFile);
    rightOutFile = NULL;
  } else if((strcmp(bul,"extlroff") == 0)) {
    if(EXTLR) {
      close(fds1[1]);
      close(fds2[0]);
      kill(pid,SIGTERM);
      EXTLR = 0;
      fdReset(omega);
    }
  } else if((strcmp(bul,"extrloff") == 0)) {
    if(EXTRL) {
      close(fds3[1]);
      close(fds4[0]);
      kill(pid1,SIGTERM);
      EXTRL = 0;
      fdReset(omega);
    }
  } else if((strcmp(bul,"outputl") == 0) && omega->left) {
    omega->outputl = 1; omega->outputr = 0;
  } else if((strcmp(bul,"outputr") == 0) && omega->right) {
    omega->outputl = 0; omega->outputr = 1;
  } else if(strcmp(bul,"output") == 0) {
    if(omega->outputl) {wprintw(sw[6],"left");}
    else if(omega->outputr) {wprintw(sw[6],"right");}
    else {wprintw(sw[6],"no direction specified");}
  } else if((strcmp(bul,"dsplr") == 0) && omega->left) {
    omega->dsplr = 1; omega->dsprl = 0;
  } else if((strcmp(bul,"dsprl") == 0) && omega->right) {
    omega->dsplr = 0; omega->dsprl = 1;
  } else if(strcmp(bul,"display") == 0) {
    if(omega->dsplr) {wprintw(sw[6],"direction: lr");}
    else if(omega->dsprl) {wprintw(sw[6],"direction: rl");}
    else {wprintw(sw[6],"no direction specified");}
    // drop right
  } else if((strcmp(bul,"dropr") == 0) && omega->right) {
    if(omega->rConnect.listening) {close(omega->rConnect.sdw);}
    else {close(omega->rConnect.sd);}
    omega->right = omega->rConnect.listening = 0;
    if(omega->persr) {
      if(omega->rConnect.pass) {passConnect(omega,0);}
      else {aggrConnect(omega,0);}
    } else {
      omega->rConnect.pass = 0;
    }
    fdReset(omega);
    // drop left
  } else if((strcmp(bul,"dropl") == 0) && omega->left) {
    if(omega->lConnect.listening) {wprintw(sw[6],"notlistening");close(omega->lConnect.sdw);}
    else {close(omega->lConnect.sd);}
    omega->left = omega->lConnect.listening = 0;
    if(omega->persl) {
      if(omega->lConnect.pass) {passConnect(omega,1);}
      else {aggrConnect(omega,1);}
    } else {
      omega->lConnect.pass = 0;
    }
    fdReset(omega);
  } else if((strcmp(bul,"persl") == 0) && omega->left) {
    omega->persl = 1;
  } else if((strcmp(bul,"persr") == 0) && omega->right) {
    omega->persr = 1;
  } else if(strcmp(bul,"right") == 0) {
    char right = omega->right;
    if(strcmp(omega->rConnect.localPort,"") == 0) {wprintw(sw[6],"NONE\t*:*:*:*",15); memset(bul,0,sizeof(bul)); omega->index = 0; return;}
    if(!right) {wprintw(sw[6],"NONE\t");}
    else if(omega->rConnect.listening) {wprintw(sw[6],"LISTENING\t");}
    else {wprintw(sw[6],"CONNECTED\t");}
    if(strcmp(omega->rConnect.localPort,"") == 0) {
      wprintw(sw[6],"*:*:*:*"); memset(bul,0,sizeof(bul)); omega->index = 0; return;
    } else {wprintw(sw[6],omega->rConnect.dotAddr); wprintw(sw[6],":"); wprintw(sw[6],omega->rConnect.localPort);wprintw(sw[6],":");}
    if(right && !omega->rConnect.listening) {
      char port[32]; memset(port,0,sizeof(port));
      struct sockaddr_in peer;
      socklen_t len = sizeof(peer);
      getpeername(omega->rConnect.sd,(struct sockaddr *) &peer,&len);
      wprintw(sw[6],inet_ntoa(peer.sin_addr)); wprintw(sw[6],":");
      itoa((int)ntohs(peer.sin_port),port);
      wprintw(sw[6],port); wprintw(sw[6],"\t");
    } else {
      wprintw(sw[6],"*"); wprintw(sw[6],":"); wprintw(sw[6],"*"); wprintw(sw[6],"\t");
    }
  } else if(strcmp(bul,"left") == 0) {
    char left = omega->left;
    if(strcmp(omega->lConnect.localPort,"") == 0) {wprintw(sw[6],"NONE\t*:*:*:*\t"); memset(bul,0,sizeof(bul)); omega->index = 0; return;}
    else if(omega->lConnect.listening) {wprintw(sw[6],"LISTENING\t");}
    else if(!left) {wprintw(sw[6],"NONE\t");}
    else {wprintw(sw[6],"CONNECTED\t",11);}
    if(strcmp(omega->lConnect.localPort,"") == 0) {
      wprintw(sw[6],"\t");
      wprintw(sw[6],"*:*:*:*"); memset(bul,0,sizeof(bul)); omega->index = 0; return;
    }
    if(left && !omega->lConnect.listening) {
      char port[5]; memset(port,0,sizeof(port));
      struct sockaddr_in peer;
      socklen_t len = sizeof(peer);
      getpeername(omega->lConnect.sd,(struct sockaddr *) &peer,&len);
      wprintw(sw[6],inet_ntoa(peer.sin_addr)); wprintw(sw[6],":");
      itoa((int)ntohs(peer.sin_port),port);
      wprintw(sw[6],port); wprintw(sw[6],":");
    } else {
      wprintw(sw[6],"*"); wprintw(sw[6],":"); wprintw(sw[6],"*"); wprintw(sw[6],":");
    }
    wprintw(sw[6],omega->lConnect.dotAddr,15); wprintw(sw[6],":");
    wprintw(sw[6],omega->lConnect.localPort,strlen(omega->lConnect.localPort)); wprintw(sw[6],"\t");
  } else if((strcmp(bul,"loopr") == 0) && omega->left) {
    omega->loopr = 1;
  } else if((strcmp(bul,"loopl") == 0) && omega->right) {
    omega->loopl = 1;
  } else if(strcmp(bul,"q") == 0) {
    if(leftOutFile != NULL)
      fclose(leftOutFile);
    if(leftInFile != NULL)
      fclose(leftInFile);
    if(rightOutFile != NULL)
      fclose(rightOutFile);
    if(rightInFile != NULL)
      fclose(rightInFile);
    clear();
    endwin();
    reset_shell_mode();
    exit(0);
  } else if(strcmp(bul,"reset") == 0) {
    if(omega->right) {
      close(omega->rConnect.sd);
      close(omega->rConnect.sdw);
    }
    if(omega->left) {
      close(omega->lConnect.sd);
      close(omega->lConnect.sdw);
    }
    fdReset(omega);
    *omega = begin;
    int i;
    for(i = 0; i < 7; i++) {
      wclear(w[i]);
      wclear(sw[i]);
      wborder(w[i],0,0,0,0,0,0,0,0);
      update_win(i);
    }
    if(omega->right) {
      aggrConnect(omega,0);
    }
    if(omega->left) {
      passConnect(omega,1);
    }
    wmove(sw[6],0,0);
    fdReset(omega);
  } else {
    char *info[10]; //= {NULL,NULL,NULL,NULL};
    int i = 0;
    info[0] = strtok(bul," ");
    while(info[i] != NULL && i < 10) {
      i++;
      info[i] = strtok(NULL," ");
    }

    int t = i;

    for(;t < 10; t++) {
      info[t] = "";
    }
    char *op = malloc(32);
    strcpy(op,info[0]);
    // check for 0 or 1 arguments
    if(strcmp(op,"") == 0) {
      bozzo(++bozo); memset(bul,0,sizeof(bul)); omega->index = 0; free(op); return;
    }
    if(strcmp(op,"extlr") == 0 && strcmp(info[1],"") != 0) {
      if(EXTLR) {char *temp = malloc(16); strcpy(temp,"extlroff"); action(omega,temp);}
      int n = 1;
      t = i;
      if(pipe(fds1) < 0) {memset(bul,0,sizeof(bul)); omega->index = 0; free(op);return;}
      if(pipe(fds2) < 0) {memset(bul,0,sizeof(bul)); omega->index = 0; free(op); return;}
      if((pid = fork()) < 0) {memset(bul,0,sizeof(bul)); omega->index = 0; free(op); return;}
      if(pid == (pid_t) 0) {
        close(fds1[1]);
        dup2(fds1[0], STDIN_FILENO);
        close(fds2[0]);
        dup2(fds2[1], STDOUT_FILENO);
        t = i;
        int k = 0;
        for(t = 1; t < 10; t++) {
          if(strcmp(info[t],"") != 0) {
            k++;
          }
        }
        char *arggs[k+1];
        for(t = 0; t < k; t++) {
          arggs[t] = (char *) malloc(32);
          strcpy(arggs[t],info[t+1]);
        }
        arggs[k] = (char *) NULL;
        execvp(arggs[0],arggs);
      } else {
        close(fds1[0]);
        close(fds2[1]);
        EXTLR = 1;
        memset(bul,0,sizeof(bul));
        omega->index = 0;
        free(op);
        fdReset(omega);
        return;
      }
    }
    if(strcmp(op,"extrl") == 0 && strcmp(info[1],"") != 0) {
      if(EXTRL) {char *temp = malloc(16); strcpy(temp,"extrloff"); action(omega,temp);}
      int n = 1;
      t = i;
      if(pipe(fds3) < 0) {memset(bul,0,sizeof(bul)); omega->index = 0; free(op);return;}
      if(pipe(fds4) < 0) {memset(bul,0,sizeof(bul)); omega->index = 0; free(op); return;}
      if((pid1 = fork()) < 0) {memset(bul,0,sizeof(bul)); omega->index = 0; free(op); return;};
      if(pid1 == (pid_t) 0) {
        close(fds3[1]);
        dup2(fds3[0], STDIN_FILENO);
        close(fds4[0]);
        dup2(fds4[1], STDOUT_FILENO);
        int k = 0;
        for(t = 1; t < 10; t++) {
          if(strcmp(info[t],"") != 0) {
            k++;
          }
        }
        char *arggs[k+1];
        for(t = 0; t < k; t++) {
          arggs[t] = malloc(32);
          strcpy(arggs[t],info[t+1]);
          wprintw(sw[5],arggs[t]); wprintw(sw[5],"\t"); update_win(5);

        }
        arggs[k] = (char *) NULL;
        execvp(arggs[0],arggs);
      } else {
        close(fds3[0]);
        close(fds4[1]);
        stream2 = fdopen(fds3[1],"w");
        stream22 = fdopen(fds4[0],"r");
        EXTRL = 1;
        memset(bul,0,sizeof(bul));
        omega->index = 0;
        free(op);
        fdReset(omega);
        return;
      }
    }
    if(i >= 3 && strcmp(info[3],"") != 0) {
      bozzo(++bozo); memset(bul,0,sizeof(bul)); omega->index = 0;
      return;
    }
    if(strcmp(op,"connectr") == 0 && strcmp(info[1],"") != 0) {
      if(!omega->right) {
        strcpy(omega->rConnect.remoteAddr,info[1]);
        if(strcmp(info[2],"") == 0) {
          if(strcmp(omega->rConnect.remotePort,"") == 0) {
            strcpy(omega->rConnect.remotePort,"36764");
          }
        } else {
          if(isDigit(info[2]))
            strcpy(omega->rConnect.remotePort,info[2]);
          else
            strcpy(omega->rConnect.remotePort,"36764");
        }
        aggrConnect(omega,0);
      } else
        wprintw(sw[6],"You must drop right connection first.");
    } else if((strcmp(op,"connectl") == 0) && strcmp(info[1],"") != 0) {
      if(!omega->left) {
        strcpy(omega->lConnect.remoteAddr,info[1]);
        if(strcmp(info[2],"") == 0) {
          if(strcmp(omega->lConnect.remotePort,"") == 0) {
            strcpy(omega->lConnect.remotePort,"36764");
          }
        } else {
          if(isDigit(info[2]))
            strcpy(omega->lConnect.remotePort,info[2]);
          else
            strcpy(omega->lConnect.remotePort,"36764");
        }
        aggrConnect(omega,1);
      } else
        wprintw(sw[6],"You must drop left connection first.");
    } else if(strcmp(info[2],"") == 0) {
      if(strcmp(op,"listenr") == 0) {
        if(!omega->right) {
          //char b[5]; itoa(PROTOPORT,b); memset(b,0,sizeof(b));
          if(!isDigit(info[1])) {strcpy(omega->rConnect.localPort,"36764");}
          else {strcpy(omega->rConnect.localPort,info[1]);}
          passConnect(omega,0);
        } else {wprintw(sw[6],"You must drop right connection first.");}
      } else if(strcmp(op,"listenl") == 0) {
        if(!omega->left) {
          if(!isDigit(info[1])) {strcpy(omega->lConnect.localPort,"36764");}
          else {strcpy(omega->lConnect.localPort,info[1]);}
          passConnect(omega,1);
        } else {wprintw(sw[6],"You must drop left connection first.");}
      } else if(strcmp(op,"read") == 0 && strcmp(info[1],"") != 0 && (omega->right || omega->left)) {
        int c;
        char buf[1024];
        FILE *file;
        size_t nread;
        file = fopen(info[1],"r");
        if(file) {
          while((nread = fread(buf,1,sizeof(buf),file) ) > 0) {
            if(omega->left)
              transmitSock(omega->lConnect.sd,omega->rConnect.sd,omega->outputl,omega->outputr,buf,1,0, omega);
            else
              transmitSock(omega->lConnect.sd,omega->rConnect.sd,omega->outputl,omega->outputr,buf,0,1, omega);
          }
          fclose(file);
        }
      } else if(strcmp(op,"llport") == 0) {
        if(omega->left) {wprintw(sw[6],"You must drop left connection first.");}
        else {if(isDigit(info[1])) {strcpy(omega->lConnect.localPort,info[1]);}
              else {if(strcmp(omega->lConnect.localPort,"") == 0) {strcpy(omega->lConnect.localPort,"36764");}}}
      } else if(strcmp(op,"rlport") == 0) {
        if(omega->right) {wprintw(sw[6],"You must drop right connection first.");}
        else {if(isDigit(info[1])) {strcpy(omega->rConnect.localPort,info[1]);}
              else {if(strcmp(omega->rConnect.localPort,"") == 0) {strcpy(omega->rConnect.localPort,"36764");}}}
      } else if(strcmp(op,"lrport") == 0) {
        if(omega->left && !omega->lConnect.listening) {wprintw(sw[6],"You must drop left connection first.");}
        else {if(!isDigit(info[1])) {wprintw(sw[6],"Invalid port number");}
              else {strcpy(omega->lConnect.specialPort,info[1]);}}
      } else if(strcmp(op,"lraddr") == 0 && strcmp(info[1],"") != 0 && strcmp(info[2],"") == 0) {
        if(omega->left && !omega->lConnect.listening) {wprintw(sw[6],"You must drop left connection first.");}
        else {strcpy(omega->lConnect.specialAddr,info[1]);}
      } else if(strcmp(op,"rraddr") == 0 && strcmp(info[1],"") != 0 && strcmp(info[2],"") == 0) {
        if((omega->right && !omega->rConnect.listening)) {wprintw(sw[6],"You must drop right connection first.");}
        else {strcpy(omega->rConnect.specialAddr,info[1]);}
      } else if(strcmp(op,"loglrpre") == 0 && strcmp(info[1],"") != 0 && strcmp(info[2],"") == 0) {
        leftInFile = fopen(info[1],"w+");
        if(leftInFile == NULL) {
          wprintw(sw[6],"Failed to create/open file");
          memset(bul,0,sizeof(bul));
          omega->index = 0;
          free(op);
          return;
        }
      } else if(strcmp(op,"logrlpre") == 0 && strcmp(info[1],"") != 0 && strcmp(info[2],"") == 0) {
        rightInFile = fopen(info[1],"w+");
        if(rightInFile == NULL) {
          wprintw(sw[6],"Failed to create/open file");
          memset(bul,0,sizeof(bul));
          omega->index = 0;
          free(op);
          return;
        }
      } else if(strcmp(op,"loglrpost") == 0 && strcmp(info[1],"") != 0 && strcmp(info[2],"") == 0) {
        leftOutFile = fopen(info[1],"w+");
        if(leftOutFile == NULL) {
          wprintw(sw[6],"Failed to create/open file");
          memset(bul,0,sizeof(bul));
          omega->index = 0;
          free(op);
          return;
        }
      } else if(strcmp(op,"logrlpost") == 0 && strcmp(info[1],"") != 0 && strcmp(info[2],"") == 0) {
        rightOutFile = fopen(info[1],"w+");
        if(rightOutFile == NULL) {
          wprintw(sw[6],"Failed to create/open file");
          return;
        }
      } else if(strcmp(op,"rraddr") == 0 && strcmp(info[1],"") != 0 && strcmp(info[2],"") == 0) {
        if((omega->right && !omega->rConnect.listening)) {wprintw(sw[6],"You must drop right connection first.");}
        else {strcpy(omega->rConnect.specialAddr,info[1]);}
      } else if(strcmp(op,"rraddr") == 0 && strcmp(info[1],"") != 0 && strcmp(info[2],"") == 0) {
        if((omega->right && !omega->rConnect.listening)) {wprintw(sw[6],"You must drop right connection first.");}
        else {strcpy(omega->rConnect.specialAddr,info[1]);}
      } else {
        memset(bul,0,sizeof(bul));
        omega->index = 0;
        bozzo(++bozo);
        free(op);
        return;
      }
      free(op);
    } else {
      memset(bul,0,sizeof(bul));
      omega->index = 0;
      bozzo(++bozo);
      free(op);
      return;
    }
  }
  memset(bul,0,sizeof(bul));
  omega->index = 0;
}
void bozzo(int bozo) {
  wmove(sw[6],0,0);
  wprintw(sw[6],"                                                                                                                                                                                                                                      ");
  wmove(sw[6],0,0);
  wprintw(sw[6],"You are a big bozo.");
}
void itoa(int i, char *p) {
    char const digit[] = "0123456789";
    if(i<0){
      *p++ = '-';
      i *= -1;
    }
    int shifter = i;
    do {
      ++p;
      shifter = shifter/10;
    } while(shifter);
    *p = '\0';
    do{
      *--p = digit[i%10];
      i = i/10;
    } while(i);
}
char isDigit(char *str) {
  if(strcmp(str,"") == 0) {return 0;}
  char c; int i = 0, len = strlen(str);
  for(i = 0; i < len; i++) {
    if(!(48 <= (c = str[i]) && c <= 57))
      return 0;
  }
  return 1;
}
void aggrConnect(struct info *omega, char left) {
  /* right side connection variables */
  struct sockaddr_in sad; /* structure to hold current piggy address */
  struct sockaddr_in temp;
  struct protoent *ptrp; /* pointer to a protocol table entry */
  struct hostent *ptrh; /* pointer to a host table entry */
  char *addr = malloc(64); /* address to connect to */
  char *ports = malloc(64); /* port string argument */
  int port; /* protocol port number */
  int sd; /* socket descriptor*/
  char *bind1 = malloc(64);
  memset((char *)&sad,0,sizeof(sad)); memset((char *)&temp,0,sizeof(temp));
  sad.sin_family = AF_INET;
  if(left) {
    if(strcmp(omega->lConnect.localPort,"") != 0) {strcpy(bind1,omega->lConnect.localPort);}
    strcpy(ports,omega->lConnect.remotePort); strcpy(addr,omega->lConnect.remoteAddr);
  }
  else {
    if(strcmp(omega->rConnect.localPort,"") != 0) {strcpy(bind1,omega->rConnect.localPort);}
    strcpy(ports,omega->rConnect.remotePort); strcpy(addr,omega->rConnect.remoteAddr);
  }
  if(strcmp(ports,"") == 0) {
    port = PROTOPORT; /* Default port */
    strcpy(omega->rConnect.remotePort,"36764");
  }
  else {
    if(!isDigit(ports) || (port = atoi(ports)) < 0 || port > 65536) {
      report("bad port number\n"); free(addr); free(ports); free(bind1); return;
    }
    port = atoi(ports);
  }
  sad.sin_port = htons((u_short)port);
  ptrh = gethostbyname(addr);
  if(((char *)ptrh) == NULL) {
    report("invalid host\n"); free(addr); free(ports); free(bind1); return;
  }
  memcpy(&sad.sin_addr,ptrh->h_addr,ptrh->h_length);
  if(((long int) (ptrp = getprotobyname("tcp"))) == 0) {
    report("cannot map \"tcp\" to protocol number\n"); free(addr); free(ports); free(bind1); return;
  }
  if((sd = socket(PF_INET,SOCK_STREAM,ptrp->p_proto)) < 0) {
    report("socket creation failed\n"); free(addr); free(ports); free(bind1); return;
  }
  if(strcmp(bind1,"") != 0) {
    temp.sin_family = AF_INET;
    temp.sin_addr.s_addr = INADDR_ANY;
    temp.sin_port = htons((u_short)atoi(bind1));
    if(bind(sd,(struct sockaddr *)&temp,sizeof(temp)) < 0) {
      report("bind failed\n"); close(sd); free(addr); free(ports); free(bind1); return;
    }
  }
  int flag = 1;
  /* Eliminate "Address already in use" error me
      break;
      case 'l':ssage */
  if(setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(int)) == -1) {
    report("setsockopt failed\n"); close(sd); free(addr); free(ports); free(bind1); return;
  }
  if(connect(sd,(struct sockaddr *)&sad,sizeof(sad)) < 0) {
    report("connect fail\n"); close(sd); free(addr); free(ports); free(bind1); return;
  }
  if(left) {omega->left = 1; omega->lConnect.pass = omega->lConnect.listening = 0; omega->lConnect.sd = sd; getInfo(omega,1,1);}
  else {omega->right = 1; omega->rConnect.pass = omega->rConnect.listening = 0; omega->rConnect.sd = sd; getInfo(omega,0,1);}
  fdReset(omega);
  free(addr); free(ports); free(bind1);
  //wprintw(s[6],"aggrConnect success\n\r",21);
}
void passConnect(struct info *omega, char left) {
  /* left side connection variables */
  struct protoent *ptrp; /* pointer to a protocol table entry */
  struct sockaddr_in sad; /* structure to hold current piggy address */
  int sd1, sdL; /* socket descriptors */
  int port; /* protocol port number */
  int alen; /* length of address */
  char *ports = malloc(64); /* port */
  memset((char *)&sad,0,sizeof(sad));
  sad.sin_family = AF_INET; /* Set family to Internet */
  sad.sin_addr.s_addr = INADDR_ANY; /* Set local IP address */
  if(left) {strcpy(ports,omega->lConnect.localPort);}
  else {strcpy(ports,omega->rConnect.localPort);}
  if(strcmp(ports,"") == 0) {
    port = PROTOPORT; /* Default port */
    strcpy(omega->lConnect.localPort,"36764");
  }
  else {
    if(!isDigit(ports) || (port = atoi(ports)) < 0 || port > 65536) {
      report("bad port number\n\r"); free(ports); return;
    }
    port = atoi(ports);
  }
  sad.sin_port = htons((u_short)port);
  /* Acquire protocol name and map to protocol number */
  if(((long int) (ptrp = getprotobyname("tcp"))) == 0) {
    report("cannot map \"tcp\" to protocol number\n\r"); free(ports); return;
  }
  /* Create socket */
  if((sd1 = socket(PF_INET,SOCK_STREAM,ptrp->p_proto)) < 0) {
    report("socket creation failed\n\r"); free(ports); return;
  }
  int flag = 1;
  /* Eliminate "Address already in use" error message */
  if(setsockopt(sd1,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(int)) == -1) {
    report("setsockopt failed\n\r"); close(sd1); free(ports); return;
  }
  /* Bind local address to socket */
  if(bind(sd1,(struct sockaddr *)&sad,sizeof(sad)) < 0) {
    report("bind failed\n\r"); close(sd1);free(ports); return;
  }
  /* Size of queue */
  if(listen(sd1,QLEN) < 0) {
    report("listen failed\n\r"); close(sd1); free(ports); return;
  }
  if(left) {omega->left = omega->lConnect.listening = omega->lConnect.pass = 1;
            omega->lConnect.sdw = sd1;}
  else {omega->right = omega->rConnect.listening = omega->rConnect.pass = 1;
            omega->rConnect.sdw = sd1;}
  fdReset(omega);
  free(ports);
}
void fdReset(info *omega) {
  FD_ZERO(&omega->tds);
  FD_ZERO(&omega->wds);
  FD_SET(0,&omega->tds);
  if(omega->left) {
    if(omega->lConnect.listening)
      FD_SET(omega->lConnect.sdw,&omega->tds);
    else {
      FD_SET(omega->lConnect.sd,&omega->wds);
      FD_SET(omega->lConnect.sd,&omega->tds);
    }
  }
  if(omega->right) {
    if(omega->rConnect.listening)
      FD_SET(omega->rConnect.sdw,&omega->tds);
    else {
      FD_SET(omega->rConnect.sd,&omega->wds);
      FD_SET(omega->rConnect.sd,&omega->tds);
    }
  }
  if(EXTLR) {
    FD_SET(fds1[1],&omega->wds);
    FD_SET(fds2[0],&omega->tds);
  }
  if(EXTRL) {
    FD_SET(fds3[1],&omega->wds);
    FD_SET(fds4[0],&omega->tds);
  }
}
void getInfo(info *omega, char left, char flag) {
  char port[6]; memset(port,0,sizeof(port));
  struct sockaddr_in local;
  socklen_t len = sizeof(local);
  if(left)
    getsockname(omega->lConnect.sd,(struct sockaddr *) &local,&len);
  else
    getsockname(omega->rConnect.sd,(struct sockaddr *) &local,&len);
  if(flag) {
    strcpy(omega->lConnect.dotAddr,inet_ntoa(local.sin_addr));
    strcpy(omega->rConnect.dotAddr,inet_ntoa(local.sin_addr));
  }
  itoa((int)ntohs(local.sin_port),port);
  if(left) {strcpy(omega->lConnect.localPort,port);}
  else {strcpy(omega->rConnect.localPort,port);}
}
void update_win(int i) {
  touchwin(w[i]);
  touchwin(sw[i]);
  wrefresh(sw[i]);
  wrefresh(w[i]);
}
void report(char *s) {
  wprintw(sw[6],s);
  update_win(6);
}
void cleanBuff(char *buf, char flag) {
  int length = strlen(buf);
  char temp[length + 1];
  int i;
  for(i = 0; i < length; i++) {
    if(buf[i] < 32 || buf[i] > 126) {
      if(flag && (buf[i] == 13 || buf[i] == 10))
        continue;
    } else
      temp[i] = buf[i];
  }
  temp[length] = '\0';
  strcpy(buf,temp);
}
