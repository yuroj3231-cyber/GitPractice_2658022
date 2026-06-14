#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>    
#include <stdbool.h>			
#include "engine.h"

// 함수 정의

bool engine_init()
{
    // SDL 초기화, 비디오와 오디오 서브시스템 초기화
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
        return 0;
    // 창 생성 및 렌더러 초기화
    window = SDL_CreateWindow("Raising Fishes", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!window)
        return 0;
    // TTF 초기화 및 폰트 로드
    if (TTF_Init() != 0)
        return 0;
    font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 20);       // 폰트 파일 경로(절대 혹은 상대) 필요
    if (!font) {
        printf("폰트 로드 실패: %s\n", TTF_GetError());
        SDL_Quit();
        return 0;
    }
    // 물고기 텍스처 로드
    fishTexture = loadTexture("fish.bmp");          // 물고기 이미지 파일 경로 필요
    if (!fishTexture) {                             // 텍스처 로드 실패 시 에러 메시지 출력
        SDL_Quit();
        return 0;
    }
    // 오디오 초기화
    if (!initAudio()) {                            // 오디오 초기화 실패 시 에러 메시지 출력
        SDL_Quit();
        return 0;
    }
    return 1;
}

// 오디오 초기화 함수 정의
bool initAudio()
{
    // WAV 파일 로드, water.wav 파일을 메모리에 로드하여 wavSpec, wavBuffer, wavLength에 정보 저장
    if (SDL_LoadWAV("water.wav", &wavSpec, &wavBuffer, &wavLength) == NULL) {
        printf("WAV 파일 로드 실패: %s\n", SDL_GetError());
        return false;
    }
    // 오디오 출력 장치 열기
    audioDevice = SDL_OpenAudioDevice(
        NULL,       // 기본 오디오 장치 사용
        0,          // 0: 출력 장치
        &wavSpec,   // wav 파일의 오디오 형식
        NULL,       // 원하는 형식 그대로 사용
        0
    );
    if (audioDevice == 0) {
        printf("오디오 장치 열기 실패: %s\n", SDL_GetError());
        SDL_FreeWAV(wavBuffer);
        wavBuffer = NULL;
        return false;
    }
    // 오디오 장치를 재생가능한 상태로 만든다.
    SDL_PauseAudioDevice(audioDevice, 0); // 오디오 장치 재생 시작, 0: 재생, 1: 일시정지

    return true;
}
// 물 주는 소리 재생 함수 정의
void playWaterSound()
{
    if (audioDevice != 0 && wavBuffer != NULL) {
        SDL_ClearQueuedAudio(audioDevice);                  // 기존에 큐에 남아있는 소리 데이터 제거
        SDL_QueueAudio(audioDevice, wavBuffer, wavLength);  // wavBuffer에 저장된 소리 데이터를 오디오 장치에 보내서 재생
    }
}

void initGame() {
    for (int i = 0; i < NUM; i++) {
        fishTanks[i].fish = 10;
        fishTanks[i].water = 100;
        fishTanks[i].isAlive = 1;

        //  체력, 성장도, 이미지 초기화
        fishTanks[i].health = 100;   // 처음 체력은 100으로 꽉 채우기
        fishTanks[i].growth = 0;     // 성장도는 0부터 시작
        fishTanks[i].texture = NULL; // 이미지는 나중에 넣을 거라 일단 비워둠

        // 필수조건: 어항마다 다른 물고기 종류 지정하기
        if (i % 4 == 0) {
            fishTanks[i].type = NORMAL_FISH;
        }
        else if (i % 4 == 1) {
            fishTanks[i].type = FAST_FISH;
        }
        else if (i % 4 == 2) {
            fishTanks[i].type = BIG_FISH;
        }
        else {
            fishTanks[i].type = SPECIAL_FISH;
        }
    }
    startTime = SDL_GetTicks();             // 게임 시작 시간 기록
    lastUpdateTime = startTime;             // 마지막 업데이트 시간 초기화
}

void renderFishTanks() {                        // 어항과 물고기 렌더링 함수
    for (int i = 0; i < NUM; i++) {
        int x = 50 + i * (FISHTANK_WIDTH + 10);
        SDL_Rect bowl = { x, 300, FISHTANK_WIDTH, FISHTANK_HEIGHT };
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // 파란색 테두리
        SDL_RenderDrawRect(renderer, &bowl);

        // 물 높이 표시
        int waterHeight = fishTanks[i].water * FISHTANK_HEIGHT / 100;
        SDL_Rect water = { x, 300 + FISHTANK_HEIGHT - waterHeight, FISHTANK_WIDTH, waterHeight };
        SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);
        SDL_RenderFillRect(renderer, &water);

        //  물고기 종류별 색상 및 죽은 상태 처리
        if (fishTexture != NULL) {
            if (fishTanks[i].isAlive) {
                // 살아있을 때: 종류별로 색상(Tint) 적용
                switch (fishTanks[i].type) {
                case NORMAL_FISH:  SDL_SetTextureColorMod(fishTexture, 255, 255, 255); break; // 기본색
                case FAST_FISH:    SDL_SetTextureColorMod(fishTexture, 255, 180, 180); break; // 약간 붉은빛
                case BIG_FISH:     SDL_SetTextureColorMod(fishTexture, 180, 255, 180); break; // 약간 녹색빛
                case SPECIAL_FISH: SDL_SetTextureColorMod(fishTexture, 180, 180, 255); break; // 약간 푸른빛
                }
            }
            else {
                // 죽었을 때: 회색조(Grayscale) 필터 적용
                SDL_SetTextureColorMod(fishTexture, 100, 100, 100);
            }

            // 물고기 이미지 그리기
            SDL_Rect fishRect = { x + 20, 300 + FISHTANK_HEIGHT - waterHeight - 30, 60, 30 };
            SDL_RenderCopy(renderer, fishTexture, NULL, &fishRect);

            // 다음 물고기를 위해 색상을 기본(흰색)으로 되돌려놓기
            SDL_SetTextureColorMod(fishTexture, 255, 255, 255);
        }

        // 물고기 상태 텍스트 출력
        char status[64];
        if (fishTanks[i].isAlive)
            sprintf_s(status, sizeof(status), "F:%d W:%d", fishTanks[i].fish, fishTanks[i].water);
        else
            sprintf_s(status, sizeof(status), "DEAD");
        renderText(status, x + 10, 520);

        // 커서 표시
        if (i == position) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // 노란색
            SDL_RenderDrawRect(renderer, &bowl);
        }
    }
}

void renderGame() {                        // 게임 화면 렌더링
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);    // 배경을 검은색으로 설정
    SDL_RenderClear(renderer);                        // 화면 초기화

    renderFishTanks();                 // 어항과 물고기 렌더링 (우리가 아까 정리한 함수)

    // 레벨 텍스트 렌더링
    char levelText[64];
    sprintf_s(levelText, sizeof(levelText), "Level %d", level);
    renderText(levelText, 10, 10);

    SDL_RenderPresent(renderer);       // 렌더링 업데이트
}

void cleanupGame() {
    // 종료 메시지 화면
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);             // 배경을 검은색으로 설정
    SDL_RenderClear(renderer);                                  // 화면 초기화
    if (gameWin) {
        renderText("You Win! All levels completed!", 200, 200); // 승리 메시지 렌더링
    }
    else if (gameOver) {
        renderText("Game Over! All fish are dead!", 200, 200); // 게임 오버 메시지 렌더링
    }
    else {
        renderText("Game Over", 200, 200);      // 일반 게임 오버 메시지 렌더링
    }
    SDL_RenderPresent(renderer);                // 렌더링 업데이트   
    SDL_Delay(3000);                            // 메시지 표시 후 3초 대기

    SDL_DestroyTexture(fishTexture);            // 물고기 텍스처 메모리 해제
    if (audioDevice != 0) {
        SDL_CloseAudioDevice(audioDevice);      // 오디오 장치 닫기
    }

    if (wavBuffer != NULL) {
        SDL_FreeWAV(wavBuffer);                 // WAV 버퍼 메모리 해제
    }
    TTF_CloseFont(font);                        // 폰트 메모리 해제
    SDL_DestroyRenderer(renderer);              // 렌더러 메모리 해제
    SDL_DestroyWindow(window);                  // 창 메모리 해제
    TTF_Quit();                                 // TTF 종료
    SDL_Quit();                                 // SDL 종료   
}

void renderText(const char* text, int x, int y) {           // 텍스트 렌더링 함수
    SDL_Color color = { 255, 255, 255 };                    // 흰색 텍스트 색상
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);         // 텍스트를 표면으로 렌더링
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface); // 표면에서 텍스처 생성
    SDL_Rect dest = { x, y, surface->w, surface->h };       // 텍스처를 화면에 렌더링, 위치와 크기 설정
    SDL_RenderCopy(renderer, texture, NULL, &dest);         // 텍스처 렌더링
    SDL_FreeSurface(surface);                               // 표면 메모리 해제
    SDL_DestroyTexture(texture);                            // 텍스처 메모리 해제
}

void updateGame() {
    long currentTime = SDL_GetTicks();
    long elapsed = (currentTime - lastUpdateTime) / 1000; // 초 단위

    if (elapsed > 0) {
        int aliveCount = 0;

        for (int i = 0; i < NUM; i++) {
            if (fishTanks[i].isAlive == 1) {

                //  [필수 : 물고기 종류에 따른 물 소비량 및 성장률 차이]
                int consumeRate = 1; // 기본 물 소비 속도
                int growthRate = 1;  // 기본 성장 속도

                switch (fishTanks[i].type) {
                case NORMAL_FISH:
                    consumeRate = 1; growthRate = 1; break;
                case FAST_FISH:
                    consumeRate = 2; growthRate = 2; break; // 빨리 자라고 물도 빨리 먹음
                case BIG_FISH:
                    consumeRate = 3; growthRate = 1; break; // 물 소비량이 엄청 많음
                case SPECIAL_FISH:
                    consumeRate = 1; growthRate = 3; break; // 물은 적게 먹고 성장은 빠름
                }

                //  [물 소비 로직: consumeRate 적용]
                fishTanks[i].water -= (level * (fishTanks[i].fish / 20 + 1) * consumeRate * elapsed);

                // [필수: 물고기 상태 및 체력 변화]
                if (fishTanks[i].water <= 0) {
                    fishTanks[i].water = 0;
                    // 물이 없으면 바로 죽는 게 아니라, 체력이 서서히 깎임 (초당 10씩)
                    fishTanks[i].health -= (10 * elapsed);

                    if (fishTanks[i].health <= 0) {
                        fishTanks[i].health = 0;
                        fishTanks[i].isAlive = 0; // 체력이 0이 되면 최종 사망
                    }
                }
                else {
                    // 물이 충분하면 체력이 다시 회복됨
                    if (fishTanks[i].health < 100) {
                        fishTanks[i].health += (5 * elapsed);
                        if (fishTanks[i].health > 100) fishTanks[i].health = 100;
                    }

                    // 물이 충분하면 성장도 증가
                    fishTanks[i].growth += (growthRate * 5 * elapsed);
                    if (fishTanks[i].growth > 100) fishTanks[i].growth = 100;

                    // 물고기 기본 수치 증가
                    fishTanks[i].fish += ((fishTanks[i].water / 100 + 1) * growthRate * elapsed);
                    if (fishTanks[i].fish > 100) fishTanks[i].fish = 100;
                }

                if (fishTanks[i].isAlive == 1) {
                    aliveCount++;
                }
            }
        }

        if (aliveCount == 0) {
            gameOver = true;
            running = false;
        }

        // 레벨 업 조건: 시간 경과
        long totalElapsed = (currentTime - startTime) / 1000;
        if (totalElapsed / 20 > level - 1) {
            level++;
            if (level > 5) {
                level = 5;
                gameWin = true;
                running = false;
            }
        }

        lastUpdateTime = currentTime;
    }
}

void handleInput(SDL_Event* e) {
    if (e->type == SDL_KEYDOWN) {
        switch (e->key.keysym.sym) {
        case SDLK_j:
            if (position > 0) position--;
            break;
        case SDLK_l:
            if (position < NUM - 1) position++;
            break;
        case SDLK_k:
            if (fishTanks[position].water >= 0 && fishTanks[position].water < 100)  // 물이 0 이상 100 미만일 때만 물을 줄 수 있도록 조건 추가
                fishTanks[position].water += 5;
            playWaterSound(); // 물 주는 소리 재생 
            if (fishTanks[position].water > 100) fishTanks[position].water = 100; // 물이 100을 초과하지 않도록 제한
            break;
        case SDLK_ESCAPE:
            running = false;
            break;
        }
    }
}

SDL_Texture* loadTexture(const char* path) {
    SDL_Surface* surface = SDL_LoadBMP(path);
    if (!surface) {
        printf("이미지 로드 실패: %s\n", SDL_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}