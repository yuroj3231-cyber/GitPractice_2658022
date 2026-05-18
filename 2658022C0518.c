#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <conio.h> // 실시간 키 입력을 위한 

#define NUM 6

// FishTank 구조체 선언
typedef struct {
    int water;         // 물 높이
    int isAlive;       // 물고기 생존 여부, 1이면 살아있음, 0이면 죽음
    int fishSize;      // 물고기 크기, 레벨이 올라갈수록 커짐
} FishTank;

// 전역 변수 설정
FishTank tanks[NUM]; // 어항 구조체 배열 
FishTank* cursor;    // 현재 선택된 어항을 가리키는 구조체 포인터 
int level = 1;
long startTime;
long prevElapsedTime = 0;

// 함수 선언 (구조체 포인터를 매개변수로 사용)
void initData();
void printTanks();
void decreaseWater(FishTank* tank, long elapsed);
void giveWater(FishTank* tank);
void checkLevelUp(long totalSec);
int checkAllDead();

int main(void) {
    initData();
    cursor = tanks; // 커서 포인터를 첫 번째 어항 주소로 초기화
    startTime = clock();

    printf("=== 구조체 확장 버전 물고기 기르기 게임 ===\n");
    printf("조작법: j(왼쪽 어항 이동), l(오른쪽 어항 이동), k(물 주기)\n");
    printf("아무 키나 누르면 게임이 시작됩니다.\n");
    _getch();

    while (1) {
        system("cls"); // 화면 지우기 및 실시간 갱신

        long totalElapsedTime = (clock() - startTime) / CLOCKS_PER_SEC;
        long elapsed = totalElapsedTime - prevElapsedTime;

        if (elapsed > 0) {
            // 구조체 포인터를 함수에 전달하여 물 감소 처리
            for (int i = 0; i < NUM; i++) {
                decreaseWater(&tanks[i], elapsed);
            }
            prevElapsedTime = totalElapsedTime;
        }

        // 시간 경과에 따른 레벨 업 및 물고기 성장 관리
        checkLevelUp(totalElapsedTime);

        // 현재 어항 상태 화면에 출력
        printTanks();
        printf("\n[게임 정보] 현재 레벨: %d | 총 경과 시간: %ld초\n", level, totalElapsedTime);
        printf("[조작 키] j: 왼쪽 이동 | l: 오른쪽 이동 | k: 현재 선택된 어항 물 주기\n");

        // 게임 종료 조건 분기
        if (checkAllDead()) {
            printf("\n[GAME OVER] 모든 물고기가 죽었습니다. 게임이 종료됩니다.\n");
            break;
        }
        if (level >= 5) {
            printf("\n[GAME CLEAR] ★ 축하합니다! 레벨 5 달성! 최종 승리! ★\n");
            break;
        }

        // 실시간 키보드 입력 처리
        if (_kbhit()) {
            char ch = _getch();
            if (ch == 'j' || ch == 'J') {
                if (cursor > tanks) cursor--; // 왼쪽 구조체 주소로 이동
            }
            else if (ch == 'l' || ch == 'L') {
                if (cursor < &tanks[NUM - 1]) cursor++; // 오른쪽 구조체 주소로 이동
            }
            else if (ch == 'k' || ch == 'K') {
                // 현재 커서 포인터가 가리키는 구조체를 함수로 전달
                giveWater(cursor);
            }
        }

        Sleep(100); // CPU 과점유 방지를 위한 미세 대기
    }

    return 0;
}

// 데이터 초기화 함수
void initData() {
    for (int i = 0; i < NUM; i++) {
        tanks[i].water = 100;
        tanks[i].isAlive = 1;
        tanks[i].fishSize = 1; // 초기 물고기 크기는 1
    }
}

// 현재 어항 상태 출력 함수
void printTanks() {
    printf("\n==================== 현재 어항 상태 ====================\n");
    for (int i = 0; i < NUM; i++) {
        // 커서 포인터가 가리키는 위치에 ▶ 표시
        if (cursor == &tanks[i]) {
            printf("▶ ");
        }
        else {
            printf("   ");
        }

        printf("%d번 어항: ", i + 1);
        if (tanks[i].isAlive) {
            printf("물 높이 [%3d/100] | 물고기 크기: %d (물 소비 가속도: x%d)\n",
                tanks[i].water, tanks[i].fishSize, tanks[i].fishSize);
        }
        else {
            printf("물 높이 [  0/100] | [ 사망 ]\n");
        }
    }
    printf("========================================================\n");
}

// 필수 구현 조건 1: 구조체 포인터와 -> 연산자를 사용하는 물 감소 함수 
// 필수 구현 조건 2: 물고기가 성장(fishSize가 증가)하면 물 소비량이 증가하도록 구현
void decreaseWater(FishTank* tank, long elapsed) {
    if (tank->isAlive) {
        // 물고기 크기(fishSize)에 비례하여 초당 물 소비량이 커짐
        int consumption = (tank->fishSize * 2) * (int)elapsed;
        tank->water -= consumption;

        // 물 높이가 0 이하가 되면 죽은 것으로 처리
        if (tank->water <= 0) {
            tank->water = 0;
            tank->isAlive = 0;
        }
    }
}

// 필수 구현 조건 3: 구조체 포인터와 -> 연산자를 활용한 물 주기 함수 
void giveWater(FishTank* tank) {
    // 이미 죽은 어항에는 물을 줄 수 없음
    if (tank->isAlive == 0) {
        printf("\n[경고] 물고기가 이미 죽은 어항에는 물을 줄 수 없습니다!\n");
        Sleep(800);
        return;
    }

    tank->water += 15;
    if (tank->water > 100) {
        tank->water = 100;
    }
    printf("\n[성공] 선택된 어항에 물을 주었습니다. (+15)\n");
    Sleep(400);
}

// 20초마다 레벨 업 및 물고기 크기를 동기화하여 성장시키는 함수
void checkLevelUp(long totalSec) {
    int newLevel = (int)(totalSec / 20) + 1;
    if (newLevel > level) {
        level = newLevel;
        for (int i = 0; i < NUM; i++) {
            if (tanks[i].isAlive) {
                tanks[i].fishSize = level; // 레벨이 올라갈수록 물고기 크기 증가
            }
        }
    }
}

// 모든 물고기의 생존 여부를 판별하는 함수
int checkAllDead() {
    for (int i = 0; i < NUM; i++) {
        if (tanks[i].isAlive) return 0; // 한 마리라도 살아있으면 0 반환
    }
    return 1; // 모두 죽었으면 1 반환
}