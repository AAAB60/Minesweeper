#include <stdio.h>
#include <windows.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>

typedef struct {
    char X;
    char Y;
}RELATE_POS;
RELATE_POS Eight[8] = { {-1, -1},{0, -1},{1, -1},{1, 0},{-1, 0},{-1, 1},{0, 1},{1, 1} };
RELATE_POS Four[4] = { {0, -1},{1, 0},{-1, 0},{0, 1} };

HANDLE hConsole;

// 调整光标位置
COORD setCursorPosition(int x, int y) {
    COORD pos;
    pos.X = x;
    pos.Y = y;
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(hConsole, &info);
    SetConsoleCursorPosition(hConsole, pos);
    return info.dwCursorPosition;
}
// 调整文字颜色
void setConsoleTextColor( WORD color) {
    SetConsoleTextAttribute(hConsole, color);
}
int setWindowSize(int width, int height) {
    if (width < 8 || height < 4) return -1;
    // 设置窗口大小
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SMALL_RECT windowSize = { 0, 0, width - 1, height - 1 }; // 设置左上角和右下角的坐标（以0-based索引表示）
    SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
    return 0;
}
//调整光标闪烁
void setCursorBlink(int dos) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;

    // 获取光标信息
    GetConsoleCursorInfo(hConsole, &cursorInfo);

    // 开启或关闭光标闪烁
	cursorInfo.bVisible = dos ? TRUE : FALSE; // 设置为 TRUE 开启闪烁，设置为 FALSE 关闭闪烁

    // 设置光标信息
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}
void CheckSize() {
    char inp;
    while (1) {
        scanf_s("%c\n", &inp);
        HWND hwnd = GetConsoleWindow();
        RECT rect;

        GetWindowRect(hwnd, &rect);

        printf("%ld %ld %ld %ld\n", rect.left, rect.right, rect.top, rect.bottom);
        printf("%ld %ld\n", (rect.right - rect.left) / 8, (rect.bottom - rect.top) / 26);

    }
}
void printMenu(HANDLE hConsle, int hightLine) {
    setWindowSize(42, 11);
    setCursorPosition(11, 4);
    printf("扫雷 v0.0.1");
    setCursorPosition(11, 6);
    if (hightLine == 0)
    {
        setConsoleTextColor(0x2f);
        printf("[easy  ]");
        setConsoleTextColor(0xf);
        setCursorPosition(11, 7);
        printf(" midium ");
        setCursorPosition(11, 8);
        printf(" hard   ");
    }
    else if (hightLine == 1) {
        printf(" easy   ");
        setConsoleTextColor(0x2f);
        setCursorPosition(11, 7);
        printf("[midium]");
        setConsoleTextColor(0xf);
        setCursorPosition(11, 8);
        printf(" hard   ");
    }
    else if (hightLine == 2) {
        printf(" easy   ");
        setCursorPosition(11, 7);
        printf(" midium ");
        setConsoleTextColor(0x2f);
        setCursorPosition(11, 8);
        printf("[hard  ]");
        setConsoleTextColor(0xf);
    }
    else {
        printf(" easy   ");
        setCursorPosition(11, 7);
        printf(" midium ");
        setCursorPosition(11, 8);
        printf(" hard   ");
    }
}
typedef char DIFFI;
void printTopBar(int dif_ind) {
    setConsoleTextColor(0x70);
    setCursorPosition(0, 0);
    printf("time:");//6, 19, 36;
    setCursorPosition(12, 0);
    printf("score:");
    setCursorPosition(0, dif_ind + 1);
    printf("mine left:");
    setConsoleTextColor(15);
}
void drawBox(COORD pos1, COORD pos2, WORD color) {
    if (pos1.X > pos2.X) {
        pos1.X ^= pos2.X;
        pos2.X ^= pos1.X;
        pos1.X ^= pos2.X;
    }
    if (pos1.Y > pos2.Y) {
        pos1.Y ^= pos2.Y;
        pos2.Y ^= pos1.Y;
        pos1.Y ^= pos2.Y;
    }
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = pos1;
    SetConsoleCursorPosition(hConsole, pos);
    setConsoleTextColor(color);
    for (int i = 0; i < pos2.Y - pos1.Y + 1; i++) {
        SetConsoleCursorPosition(hConsole, pos);
        pos.Y++;
        printf(" ");
    }
    pos.X = pos2.X;
    pos.Y = pos1.Y;
    for (int i = 0; i < pos2.Y - pos1.Y + 1; i++) {
        SetConsoleCursorPosition(hConsole, pos);
        pos.Y++;
        printf(" ");
    }
    pos.Y--;
    pos.X = pos1.X;
    SetConsoleCursorPosition(hConsole, pos1);
    for (int i = 0; i < pos2.X - pos1.X + 1; i++) {
        printf(" ");
    }
    SetConsoleCursorPosition(hConsole, pos);
    for (int i = 0; i < pos2.X - pos1.X + 1; i++) {
        printf(" ");
    }
}
clock_t start; // 记录程序开始运行的时间
char stop = 0;
pthread_t thread_timer;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 创建互斥锁
void* changeTime(void* arg) {
    stop = 0;
    start = clock();
    while (!stop) {
        clock_t end = clock(); // 记录程序运行结束的时间

        double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC; // 计算运行时间（单位：秒）
        int minutes = (int)(elapsed_time / 60); // 计算分钟数
        int seconds = (int)(elapsed_time) % 60; // 计算秒数
        pthread_mutex_lock(&mutex); // 上锁，防止其他线程访问共享资源
        setConsoleTextColor(15);
        COORD position = {5, 0};
        SetConsoleCursorPosition(hConsole, position);
        printf("%02d:%02d\n", minutes, seconds);
        pthread_mutex_unlock(&mutex); // 解锁
        Sleep(1000);
    }
    pthread_exit(NULL);
    return 0;
}
void printInterFace(DIFFI dif) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    setWindowSize(50, 11);
    if (dif == 0) {
        SetConsoleTextAttribute(hConsole, 0xea | 0x0400);
        printf("%c",1);
    }
}

int coverCount;
void setCover(char(*cover)[30], char(*mines)[30], RELATE_POS pos, int dif_ind) {

    cover[pos.X][pos.Y] = 0;
    if (coverCount == 1) {
        stop = 1;
        if (pthread_join(thread_timer, NULL) != 0) {
            fprintf(stderr, "Failed to join thread\n");
            return;
        }
        setConsoleTextColor(15);
        system("cls");
        COORD position = { 0,0 };
        SetConsoleCursorPosition(hConsole, position);
        printf("you win!");
        return;
    }
    coverCount--;
    if (mines[pos.X][pos.Y]) {
		// print num
		pthread_mutex_lock(&mutex); // 上锁，防止其他线程访问共享资源
        COORD position = { (short)(pos.X * 2 + 1), (short)(pos.Y + 1) };
        SetConsoleCursorPosition(hConsole, position);
		setConsoleTextColor(0xb);
		printf("%d ", mines[pos.X][pos.Y]);
		pthread_mutex_unlock(&mutex); // 解锁
	}
    else {
		// print num
		pthread_mutex_lock(&mutex); // 上锁，防止其他线程访问共享资源
        COORD position = { (short)(pos.X * 2 + 1), (short)(pos.Y + 1) };
        SetConsoleCursorPosition(hConsole, position);
		setConsoleTextColor(0xb);
		printf("  ");
		pthread_mutex_unlock(&mutex); // 解锁
        for (int i = 0; i < 8; i++) {
            if (pos.X + Eight[i].X >= 0 &&
                pos.Y + Eight[i].Y >= 0 &&
                pos.X + Eight[i].X < dif_ind &&
                pos.Y + Eight[i].Y < dif_ind &&
                cover[pos.X + Eight[i].X][pos.Y + Eight[i].Y]) {
                RELATE_POS position = { (char)(pos.X + Eight[i].X), (char)(pos.Y + Eight[i].Y) };
				setCover(cover, mines, position, dif_ind);
            }
        }
    }
}
int main() {
    //初始化
    start = clock();
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    setCursorBlink(0);

    //禁用最大化按钮
    {
        HWND consoleWindow = GetConsoleWindow();
        DWORD windowStyle = GetWindowLong(consoleWindow, GWL_STYLE);

        // 清除 WS_MAXIMIZEBOX 样式
        windowStyle = windowStyle & ~WS_MAXIMIZEBOX;
        SetWindowLong(consoleWindow, GWL_STYLE, windowStyle);
    }


    HANDLE hConsoleInput;
    hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD fdwMode;
    INPUT_RECORD irInBuf;
    DWORD cNumRead;
    // 设置输入模式
    GetConsoleMode(hConsoleInput, &fdwMode);
    SetConsoleMode(hConsoleInput, ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);

    
    printMenu(hConsole, -1);
    COORD lastPos = { 0 };
    DIFFI dif = 0;

    char loop = 1;
    while (loop) {
        // 读取输入事件
        ReadConsoleInput(hConsoleInput, &irInBuf, 1, &cNumRead);
        if (irInBuf.EventType == MOUSE_EVENT) {
            if (irInBuf.Event.MouseEvent.dwButtonState == 0) {
                if (irInBuf.Event.MouseEvent.dwMousePosition.Y != lastPos.Y) {
                    lastPos = irInBuf.Event.MouseEvent.dwMousePosition;
                    printMenu(hConsole, lastPos.Y - 6);
                }
            }
            else if (irInBuf.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) {
                dif = irInBuf.Event.MouseEvent.dwMousePosition.Y - 6;
                while (1) {
                    ReadConsoleInput(hConsoleInput, &irInBuf, 1, &cNumRead);
                    if (irInBuf.EventType == MOUSE_EVENT) {
                        if (!(irInBuf.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED))
                            // key up
                        {
                            if (irInBuf.Event.MouseEvent.dwMousePosition.Y - 6 == dif) {
                                loop = 0;
                            }
                            break;
                        }
                    }
                }
                
            }
        }
    }
    int dif_ind = (dif + 1) * 10;
    // main
    char cover[30][30];
    char mines[30][30];
    memset(cover, 1, sizeof(cover));
    memset(mines, 0, sizeof(mines));
    // summon mine
    srand(0xAAAB60);
    int loopNum = dif_ind * dif_ind / 9;
    for (int i = 0; i < loopNum; i++) {
        COORD pos = { rand() % dif_ind,rand() % dif_ind };
        while (mines[pos.X][pos.Y]) {
            pos.X = (short)(rand() % dif_ind);
            pos.Y = (short)(rand() % dif_ind);
        }
        mines[pos.X][pos.Y] = 'm';

        // num
        if (pos.X == 0) {
            if (pos.Y == 0) {
                mines[pos.X + 1][pos.Y] += 1;
                mines[pos.X][pos.Y + 1] += 1;
                mines[pos.X + 1][pos.Y + 1] += 1;
            }
            else if (pos.Y == dif_ind - 1) {
                mines[pos.X + 1][pos.Y] += 1;
                mines[pos.X][pos.Y - 1] += 1;
                mines[pos.X + 1][pos.Y - 1] += 1;
            }
            else {
                mines[pos.X + 1][pos.Y] += 1;
                mines[pos.X][pos.Y + 1] += 1;
                mines[pos.X + 1][pos.Y + 1] += 1;
                mines[pos.X][pos.Y - 1] += 1;
                mines[pos.X + 1][pos.Y - 1] += 1;
            }
        }
        else if (pos.X == dif_ind - 1) {
            if (pos.Y == 0) {
                mines[pos.X - 1][pos.Y] += 1;
                mines[pos.X][pos.Y + 1] += 1;
                mines[pos.X - 1][pos.Y + 1] += 1;
            }
            else if (pos.Y == dif_ind - 1) {
                mines[pos.X - 1][pos.Y] += 1;
                mines[pos.X][pos.Y - 1] += 1;
                mines[pos.X - 1][pos.Y - 1] += 1;
            }
            else {
                mines[pos.X - 1][pos.Y] += 1;
                mines[pos.X][pos.Y + 1] += 1;
                mines[pos.X - 1][pos.Y + 1] += 1;
                mines[pos.X][pos.Y - 1] += 1;
                mines[pos.X - 1][pos.Y - 1] += 1;
            }
        }
        else {
            mines[pos.X][pos.Y + 1] += 1;
            mines[pos.X][pos.Y - 1] += 1;
            mines[pos.X - 1][pos.Y] += 1;
            mines[pos.X - 1][pos.Y + 1] += 1;
            mines[pos.X - 1][pos.Y - 1] += 1;
            mines[pos.X + 1][pos.Y] += 1;
            mines[pos.X + 1][pos.Y + 1] += 1;
            mines[pos.X + 1][pos.Y - 1] += 1;
        }
    }

    setWindowSize((dif_ind + 1) * 2, dif_ind + 2);
    COORD pos1 = { 0,0 };
    COORD pos2 = { (short)((dif_ind) * 2 + 1),(short)(dif_ind + 1) };
    drawBox(pos1, pos2, 0x70);
    printTopBar(dif_ind);

    coverCount = dif_ind * (dif_ind - 1);
    //print main
    pthread_mutex_lock(&mutex); // 上锁，防止其他线程访问共享资源
    setConsoleTextColor(0xb0);
    for (int i = 0; i < dif_ind; i++) {
        COORD position = { 1,(short)(1 + i) };
        SetConsoleCursorPosition(hConsole, position);
        for (int j = 0; j < dif_ind; j++) {
            printf("%c ", 1);
        }
    }
    setConsoleTextColor(15);
    pthread_mutex_unlock(&mutex); // 解锁

    int score = 0;
    int mineCount = dif_ind;
    if (pthread_create(&thread_timer, NULL, changeTime, NULL)) {
        fprintf(stderr, "Failed to create timer thread\n");
        return 1;
    }
    lastPos.X = 0;
    lastPos.Y = 0;
    char doLastPos = 0;
    while (1) {
        // 读取输入事件
        ReadConsoleInput(hConsoleInput, &irInBuf, 1, &cNumRead);
        if (irInBuf.EventType == MOUSE_EVENT) {
            if (irInBuf.Event.MouseEvent.dwButtonState == 0) {
                COORD pos = irInBuf.Event.MouseEvent.dwMousePosition;
                COORD pos_related = { (pos.X - 1) / 2 ,pos.Y - 1 };
                pos.X = pos_related.X * 2 + 1;
                if (doLastPos) {
                    pthread_mutex_lock(&mutex); // 上锁，防止其他线程访问共享资源
                    SetConsoleCursorPosition(hConsole, lastPos);
                    setConsoleTextColor(0xb0);
                    if (cover[(lastPos.X - 1) / 2][lastPos.Y - 1] == 1)
						printf("%c ", 1);
					else
						printf("旗");
                    pthread_mutex_unlock(&mutex); // 解锁
                    doLastPos = 0;
                }
                if (pos_related.X >= 0 && pos_related.X <= dif_ind && pos_related.Y >= 0 && pos_related.Y <= dif_ind && cover[pos_related.X][pos_related.Y]) {
					pthread_mutex_lock(&mutex); // 上锁，防止其他线程访问共享资源
					SetConsoleCursorPosition(hConsole, pos);
					setConsoleTextColor(0x93);
                    if (cover[pos_related.X][pos_related.Y] == 1)
                        printf("%c ", 1);
                    else
                        printf("旗");
					pthread_mutex_unlock(&mutex); // 解锁
                    doLastPos = 1;
                    lastPos = pos;
                }
            }
            else if (irInBuf.Event.MouseEvent.dwButtonState == 1) {
                lastPos = irInBuf.Event.MouseEvent.dwMousePosition;
                // wait for key up
                while (1) {
                    ReadConsoleInput(hConsoleInput, &irInBuf, 1, &cNumRead);
                    if (irInBuf.EventType == MOUSE_EVENT && irInBuf.Event.MouseEvent.dwButtonState == 0) {
                        doLastPos = 0;
                        if (irInBuf.Event.MouseEvent.dwMousePosition.X == lastPos.X && irInBuf.Event.MouseEvent.dwMousePosition.Y == lastPos.Y) {
                            // click
                            COORD pos = irInBuf.Event.MouseEvent.dwMousePosition;
                            pos.X = (pos.X - 1) / 2;
                            pos.Y--;
                            if (pos.X >= 0 && pos.X <= dif_ind && pos.Y >= 0 && pos.Y <= dif_ind && cover[pos.X][pos.Y]) {
                                if (mines[pos.X][pos.Y] == 'm') {
                                    cover[pos.X][pos.Y] = 0;
                                    // game over
                                    stop = 1;
                                    if (pthread_join(thread_timer, NULL) != 0) {
                                        fprintf(stderr, "Failed to join thread\n");
                                        return 1;
                                    }
                                    setConsoleTextColor(15);
                                    system("cls");
                                    COORD position = { 0,0 };
                                    SetConsoleCursorPosition(hConsole, position);
                                    printf("game over");
                                }
                                else {
                                    RELATE_POS position = { (char)(pos.X), (char)(pos.Y) };
                                    setCover(cover, mines, position, dif_ind);
                                }
                            }
                        }
                        break;
                    }
                }
            }
            else if (irInBuf.Event.MouseEvent.dwButtonState == 2) {
                lastPos = irInBuf.Event.MouseEvent.dwMousePosition;
                // wait for key up
                while (1) {
                    ReadConsoleInput(hConsoleInput, &irInBuf, 1, &cNumRead);
                    if (irInBuf.EventType == MOUSE_EVENT && irInBuf.Event.MouseEvent.dwButtonState == 0) {
                        doLastPos = 0;
                        if (irInBuf.Event.MouseEvent.dwMousePosition.X == lastPos.X && irInBuf.Event.MouseEvent.dwMousePosition.Y == lastPos.Y) {
                            // click
                            COORD pos = irInBuf.Event.MouseEvent.dwMousePosition;
                            pos.X = (pos.X - 1) / 2;
                            pos.Y--;
                            if (pos.X >= 0 && pos.X <= dif_ind && pos.Y >= 0 && pos.Y <= dif_ind && cover[pos.X][pos.Y]) {
                                if (cover[pos.X][pos.Y] == 1){
                                    cover[pos.X][pos.Y] = 2;
                                    // print num
                                    pthread_mutex_lock(&mutex); // 上锁，防止其他线程访问共享资源
                                    COORD position = { (short)(pos.X * 2 + 1), (short)(pos.Y + 1) };
                                    SetConsoleCursorPosition(hConsole, position);
                                    setConsoleTextColor(0xb0);
                                    printf("旗");
                                    pthread_mutex_unlock(&mutex); // 解锁
                                }
                                else {
                                    cover[pos.X][pos.Y] = 1;
                                    // print num
                                    pthread_mutex_lock(&mutex); // 上锁，防止其他线程访问共享资源
                                    COORD position = { (short)(pos.X * 2 + 1), (short)(pos.Y + 1) };
                                    SetConsoleCursorPosition(hConsole, position);
                                    setConsoleTextColor(0xb0);
                                    printf("%c ", 1);
                                    pthread_mutex_unlock(&mutex); // 解锁
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }
    }

    stop = 1;
    if (pthread_join(thread_timer, NULL) != 0) {
        fprintf(stderr, "Failed to join thread\n");
        return 1;
    }
    return 0;
}
