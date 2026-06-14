#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include "engine.h"

// 전역 변수 정의
FishTank fishTanks[NUM];    // 물고기 어항 배열
int level = 1;
int position = 0;
bool running = true;
bool gameOver = false;
bool gameWin = false;
long startTime = 0;
long lastUpdateTime = 0;
// SDL 관련 변수
SDL_Window* window = NULL;          // SDL 창
SDL_Renderer* renderer = NULL;      // SDL 렌더러
TTF_Font* font = NULL;              // 폰트
SDL_Texture* fishTexture = NULL;    // 물고기 텍스처
// 오디오 관련 변수
SDL_AudioDeviceID audioDevice = 0;  // 소리를 출력할 오디오 장치 식별자
SDL_AudioSpec wavSpec;              // wav 파일의 오디오 형식 정보
Uint8* wavBuffer = NULL;            // wav 파일의 실제 소리 데이터
Uint32 wavLength = 0;               // 소리 데이터의 길이, 단위는 바이트

// 메인 함수
int main(int argc, char* argv[]) {
    // 엔진 초기화
    if (!engine_init()) {
        printf("Error initializing engine: %s\n", SDL_GetError());  // 초기화 실패 시 에러 메시지 출력
        return 1;                   // 초기화 실패 시 프로그램 종료
    }

    initGame();                     // 게임 초기화

    while (running) {               // 게임 루프
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
            handleInput(&event);
        }
        // update the game
        updateGame();
        // render the game
        renderGame();
        // update the window
        SDL_Delay(100);             // 게임 루프 간격 조절, 100ms마다 업데이트 및 렌더링
    }
    // 게임 종료 및 자원 해제
    cleanupGame();

    return 0;
}