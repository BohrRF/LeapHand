/******************************************************************************\
* Copyright (C) 2012-2014 Leap Motion, Inc. All rights reserved.               *
* Leap Motion proprietary and confidential. Not for distribution.              *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement         *
* between Leap Motion and you, your company or other organization.             *
\******************************************************************************/

#include <iostream>
#include <cstring>
#include <windows.h>
#include <chrono>
#include "analyze.h"

int main(int argc, char** argv) {
    
    /*
    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;
    */

    // Leap Motionのコントローラーおよびイベントを受け取るリスナー（のサブクラス）を作成する
    SampleListener listener;
    Controller controller;

    // イベントを受け取るリスナーを登録する
    controller.addListener(listener);

    
    // 起動時の引数に"--bg"が設定されていた場合は
    // バックグラウンドモード(アプリケーションがアクティブでない場合にもデータを取得する)
    // で動作させる
    if (argc > 1 && strcmp(argv[1], "--bg") == 0) {
        controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);
    }


    //［Enter］キーが押されるまでLeap Motionの処理を続ける
    std::cout << "Press Enter to quit..." << std::endl;
    while (!(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        auto t = std::chrono::high_resolution_clock::now();
        listener.con.refresh(std::chrono::duration_cast<std::chrono::microseconds>(t.time_since_epoch()).count(), listener.fft);
    }

    // アプリケーションの終了時にはリスナーを削除する
    controller.removeListener(listener);

    return 0;
}
