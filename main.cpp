/*--+----1----+----2----+----3----+----4----+----5-----+----6----+----7----+----8----+----9----+---*/
//あれ、今何個？

//########## ヘッダーファイル読み込み ##########
#include "DxLib.h"
#include "main.hpp"

//########## プログラムで最初に実行される関数 ##########
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	SetOutApplicationLogValidFlag(FALSE);
	ChangeWindowMode(TRUE);						//ウィンドウモードに設定
	SetGraphMode(GAME_WIDTH, GAME_HEIGHT, GAME_COLOR);	//指定の数値でウィンドウを表示する
	SetWindowStyleMode(GAME_WINDOW_BAR);		//タイトルバーはデフォルトにする
	SetMainWindowText(TEXT(GAME_WINDOW_NAME));	//ウィンドウのタイトルの文字
	SetAlwaysRunFlag(TRUE);						//非アクティブでも実行する

	if (DxLib_Init() == -1) { return -1; }	//ＤＸライブラリ初期化処理

	//フォントを一時的にインストール
	if (MY_FONT_INSTALL_ONCE() == FALSE) { return -1; }

	//フォントハンドルを作成
	if (MY_FONT_CREATE() == FALSE) { return -1; }

	//画像を読み込む
	if (MY_LOAD_IMAGE() == FALSE) { return -1; }

	//音楽を読み込む
	if (MY_LOAD_MUSIC() == FALSE) { return -1; }

	GameScene = GAME_SCENE_START;   //ゲームシーンはスタート画面から

	SetDrawScreen(DX_SCREEN_BACK);	//Draw系関数は裏画面に描画

	//無限ループ
	while (TRUE)
	{
		if (ProcessMessage() != 0) { break; }	//メッセージ処理の結果がエラーのとき、強制終了

		if (ClearDrawScreen() != 0) { break; }	//画面を消去できなかったとき、強制終了

		MY_ALL_KEYDOWN_UPDATE();				//押しているキー状態を取得する

		MY_FPS_UPDATE();						//FPSの処理(更新)

		//シーンごとに処理を行う
		switch (GameScene)
		{
		case GAME_SCENE_START:
			MY_START();   //スタート画面
			break;
		case GAME_SCENE_PLAY:
			MY_PLAY();    //プレイ画面
			break;
		case GAME_SCENE_END:
			MY_END();     //エンド画面
			break;
		case GAME_SCENE_MENU:
			MY_MENU();    //操作説明画面
			break;
		case GAME_SCENE_LEVEL:
			MY_LEVEL();	  //level説明画面
			break;
		}

		MY_FPS_DRAW();	   //FPSの処理(描画)

		ScreenFlip();		//モニタのリフレッシュレートの速さで裏画面を再描画

		MY_FPS_WAIT();     //FPSの処理(待つ)
	}

	//画像ハンドルを破棄
	MY_DELETE_IMAGE();

	//フォントハンドルを破棄
	MY_FONT_DELETE();

	//一時的にインストールしたフォントをアンインストール
	MY_FONT_UNINSTALL_ONCE();

	//音楽ハンドルを破棄
	MY_DELETE_MUSIC();

	DxLib_End();	//ＤＸライブラリ使用の終了処理

	return 0;
}

//FPS値を計測、待つ関数
VOID MY_FPS_UPDATE(VOID)
{
	if (CountFps == 0)  //1フレーム目なら時刻を記憶
	{
		StartTimeFps = GetNowCount();
	}

	if (CountFps == SampleNumFps)   //60フレーム目なら平均を計算
	{
		int now = GetNowCount();
		//現在の時間から、0フレームの時間を引き、FPSの数値で割る = 1FPS辺りの平均時間が計算される
		CalcFps = 1000.f / ((now - StartTimeFps) / (float)SampleNumFps);
		CountFps = 0;
		StartTimeFps = now;
	}
	CountFps++;
	return;
}

//FPS値を描画する関数
VOID MY_FPS_DRAW(VOID)
{
	//文字列を描画
	DrawFormatString(0, GAME_HEIGHT - 20, GetColor(255, 255, 255), "FPS:%.1f", CalcFps);
	return;
}

//FPS値を計測し、待つ関数
VOID MY_FPS_WAIT(VOID)
{
	int resultTime = GetNowCount() - StartTimeFps;			//かかった時間
	int waitTime = CountFps * 1000 / GAME_FPS - resultTime; //待つべき時間

	if (waitTime > 0)
	{
		WaitTimer(waitTime);    //待つ
	}

	return;
}

//キーの入力状態を更新する関数
VOID MY_ALL_KEYDOWN_UPDATE(VOID)
{
	char TempKey[256];  //一時的に、現在のキーの入力状態を格納する

	//直前のキー入力を取っておく
	for (int i = 0; i < 256; i++)
	{
		OldAllKeyState[i] = AllKeyState[i];
	}

	GetHitKeyStateAll(TempKey); // 全てのキーの入力状態を得る

	for (int i = 0; i < 256; i++)
	{
		if (TempKey[i] != 0)	//押されているキーのキーコードを押しているとき
		{
			AllKeyState[i]++;	//押されている
		}
		else
		{
			AllKeyState[i] = 0;	//押されていない
		}
	}
	return;
}

//キーを押しているか、キーコードで判断する
//引　数：int：キーコード：KEY_INPUT_???
BOOL MY_KEY_DOWN(int KEY_INPUT_)
{
	//キーコードのキーを押している時
	if (AllKeyState[KEY_INPUT_] != 0)
	{
		return TRUE;	//キーを押している
	}
	else
	{
		return FALSE;	//キーを押していない
	}
}

//動物の画像変更用・操作説明画面用(常に押しっぱなしの状況を避ける)
BOOL MY_KEYDOWN_1SECOND(int KEY_INPUT_)
{
	//キーコードのキーを押している時
	if (AllKeyState[KEY_INPUT_] == 1)
	{
		return TRUE;	//キーを押している
	}
	else
	{
		return FALSE;	//キーを押していない
	}
}

//スタート画面
VOID MY_START(VOID)
{
	MY_START_PROC();    //スタート画面の処理
	MY_START_DRAW();    //スタート画面の描画

	return;
}

//スタート画面の処理
VOID MY_START_PROC(VOID)
{
	//BGMが流れていないなら
	if (CheckSoundMem(START_BGM.handle) == 0)
	{
		//BGMの音量を下げる
		ChangeVolumeSoundMem(255 * 50 / 100, START_BGM.handle);  //50%の音量にする
		PlaySoundMem(START_BGM.handle, DX_PLAYTYPE_LOOP);		 //ループ再生
	}

	//1キーを押したら、プレイシーンへ移動する(Easyモード)
	if (MY_KEY_DOWN(KEY_INPUT_1) == TRUE)
	{
		//プレイ画面に向けて準備
		HaveMask = EASY_HAVE_MASK;			//マスクの上限個数の設定
		GiveMask = EASY_GIVE_MASK_RANGE;	//上げるマスクのランダム数の設定
		GameScene = GAME_SCENE_PLAY;		//プレイ画面に遷移
		StartTime = GetNowCount();			//基準時間を取得
		TimeLimit = EASY_TIMELIMIT;			//制限時間を設定

		//BGMが流れているなら
		if (CheckSoundMem(START_BGM.handle) != 0)
		{
			StopSoundMem(START_BGM.handle);   //BGMを止める
		}
		//効果音が流れていないなら
		if (CheckSoundMem(SF_BUTTON.handle) == 0)
		{
			//効果音の音量を下げる
			ChangeVolumeSoundMem(255 * 50 / 100, SF_BUTTON.handle);  //50%の音量にする
			PlaySoundMem(SF_BUTTON.handle, DX_PLAYTYPE_BACK);		 //バックグラウンド再生
		}
	}
	//2キーを押したら、プレイシーンへ移動する(Normalモード)
	else if (MY_KEY_DOWN(KEY_INPUT_2) == TRUE)
	{
		//プレイ画面に向けて準備
		HaveMask = NORMAL_HAVE_MASK;		//マスクの上限個数の設定
		GiveMask = NORMAL_GIVE_MASK_RANGE;	//上げるマスクのランダム数の設定
		GameScene = GAME_SCENE_PLAY;		//プレイ画面に遷移
		StartTime = GetNowCount();			//基準時間を取得
		TimeLimit = NORMAL_TIMELIMIT;		//制限時間を設定

		//BGMが流れているなら
		if (CheckSoundMem(START_BGM.handle) != 0)
		{
			StopSoundMem(START_BGM.handle);   //BGMを止める
		}
		//効果音が流れていないなら
		if (CheckSoundMem(SF_BUTTON.handle) == 0)
		{
			//効果音の音量を下げる
			ChangeVolumeSoundMem(255 * 50 / 100, SF_BUTTON.handle);  //50%の音量にする
			PlaySoundMem(SF_BUTTON.handle, DX_PLAYTYPE_BACK);		 //バックグラウンド再生
		}
	}
	//3キーを押したら、プレイシーンへ移動する(Hardモード)
	else if (MY_KEY_DOWN(KEY_INPUT_3) == TRUE)
	{
		//プレイ画面に向けて準備
		HaveMask = HARD_HAVE_MASK;			//マスクの上限個数の設定
		GiveMask = HARD_GIVE_MASK_RANGE;	//上げるマスクのランダム数の設定
		GameScene = GAME_SCENE_PLAY;		//プレイ画面に遷移
		StartTime = GetNowCount();			//基準時間を取得
		TimeLimit = HARD_TIMELIMIT;			//制限時間を設定

		//BGMが流れているなら
		if (CheckSoundMem(START_BGM.handle) != 0)
		{
			StopSoundMem(START_BGM.handle);   //BGMを止める
		}
		//効果音が流れていないなら
		if (CheckSoundMem(SF_BUTTON.handle) == 0)
		{
			//効果音の音量を下げる
			ChangeVolumeSoundMem(255 * 50 / 100, SF_BUTTON.handle);  //50%の音量にする
			PlaySoundMem(SF_BUTTON.handle, DX_PLAYTYPE_BACK);		 //バックグラウンド再生
		}
	}

	//シフトキー(左 or 右)を押したら、操作説明画面に移動する
	if (MY_KEY_DOWN(KEY_INPUT_LSHIFT) || MY_KEY_DOWN(KEY_INPUT_RSHIFT) == TRUE)
	{
		GameScene = GAME_SCENE_MENU;
	}

	//コントロールキー(左 or 右)を押したら、level説明画面に移動する
	if (MY_KEY_DOWN(KEY_INPUT_LCONTROL) || MY_KEY_DOWN(KEY_INPUT_RCONTROL) == TRUE)
	{
		GameScene = GAME_SCENE_LEVEL;
	}

	return;
}

//スタート画面の描画
VOID MY_START_DRAW(VOID)
{
	//背景・タイトルを描画する
	DrawGraph(ImageSTARTBG.x, ImageSTARTBG.y, ImageSTARTBG.handle, TRUE);
	DrawGraph(ImageTITLE.x, ImageTITLE.y, ImageTITLE.handle, TRUE);

	//操作説明画面へ促すボタン
	DrawGraph(ImageMENUBtn.x, ImageMENUBtn.y, ImageMENUBtn.handle, TRUE);

	//各モードの描画
	DrawGraph(ImageEasyMode.x, ImageEasyMode.y, ImageEasyMode.handle, TRUE);
	DrawGraph(ImageNormalMode.x, ImageNormalMode.y, ImageNormalMode.handle, TRUE);
	DrawGraph(ImageHardMode.x, ImageHardMode.y, ImageHardMode.handle, TRUE);

	//level説明画面へ促すボタン
	DrawGraph(ImageLEVELBtn.x, ImageLEVELBtn.y, ImageLEVELBtn.handle, TRUE);

	return;
}

//操作説明画面
VOID MY_MENU(VOID)
{
	MY_MENU_PROC();     //操作説明画面の処理
	MY_MENU_DRAW();     //操作説明画面の描画

	return;
}

//操作説明画面の処理
VOID MY_MENU_PROC(VOID)
{
	//バックスペースキーを押したら
	if (MY_KEY_DOWN(KEY_INPUT_BACK) == TRUE)
	{
		//スタート画面に戻る
		GameScene = GAME_SCENE_START;
	}

	//2つの説明画像を交互に切り替え
	if (MY_KEYDOWN_1SECOND(KEY_INPUT_RETURN) == TRUE)
	{
		if (ImageMENU1.IsDraw == TRUE) {
			ImageMENU1.IsDraw = FALSE;	//消去
			ImageMENU2.IsDraw = TRUE;	//表示
		}
		else {
			ImageMENU1.IsDraw = TRUE;	//表示
			ImageMENU2.IsDraw = FALSE;	//消去
		}
	}

	return;
}

//操作説明画面の描画
VOID MY_MENU_DRAW(VOID)
{
	//背景を描画する
	DrawGraph(ImageMENUBG.x, ImageMENUBG.y, ImageMENUBG.handle, TRUE);

	if (ImageMENU1.IsDraw == TRUE)  //説明画像の1枚目
		DrawGraph(ImageMENU1.image.x, ImageMENU1.image.y, ImageMENU1.image.handle, TRUE);
	if (ImageMENU2.IsDraw == TRUE)  //説明画像の2枚目
		DrawGraph(ImageMENU2.image.x, ImageMENU2.image.y, ImageMENU2.image.handle, TRUE);
	return;
}

//level説明画面
VOID MY_LEVEL(VOID)
{
	MY_LEVEL_PROC();	//level説明画面の処理
	MY_LEVEL_DRAW();	//level説明画面の描画

	return;
}

//level説明画面の処理
VOID MY_LEVEL_PROC(VOID)
{
	//バックスペースキーを押したら
	if (MY_KEY_DOWN(KEY_INPUT_BACK) == TRUE)
	{
		//スタート画面に戻る
		GameScene = GAME_SCENE_START;
	}

	return;
}

//level説明画面の描画
VOID MY_LEVEL_DRAW(VOID)
{
	//背景の描画
	DrawGraph(ImageMENUBG.x, ImageMENUBG.y, ImageMENUBG.handle, TRUE);

	//level説明の描画
	DrawGraph(ImageLEVELExp.x, ImageLEVELExp.y, ImageLEVELExp.handle, TRUE);

	return;
}

//プレイ画面
VOID MY_PLAY(VOID)
{
	MY_PLAY_PROC();    //プレイ画面の処理
	MY_PLAY_DRAW();    //プレイ画面の描画

	return;
}

//プレイ画面の処理
VOID MY_PLAY_PROC(VOID)
{
	//BGMが流れていないなら
	if (CheckSoundMem(PLAY_BGM.handle) == 0)
	{
		//BGMの音量を下げる
		ChangeVolumeSoundMem(255 * 50 / 100, PLAY_BGM.handle);  //50%の音量にする
		PlaySoundMem(PLAY_BGM.handle, DX_PLAYTYPE_LOOP);		//ループ再生
	}

	if (First_flg)  //まずカウントダウンからスタート
	{
		if (CountDown)  //基準時間を取得
		{
			StartTime = GetNowCount();
			CountDown = FALSE;		//これ以降,このif文は行わない
		}

		int NowCount = GetNowCount();
		CDTimeLimit = 3 * 1000;  //3秒間のカウントダウンを行う
		ElaTime = CDTimeLimit - (NowCount - StartTime);

		////経過時間が0秒になったら(3,2,1 で終了させるため <=)
		if (ElaTime <= 0)
		{
			StartTime = GetNowCount();  //最初の問題用に基準時間を設定
			First_flg = FALSE;			//これ以降はカウントダウンは行わない
		}
		
		//効果音が流れていないなら
		if (CheckSoundMem(SF_TIME.handle) == 0)
		{
			//効果音の音量を下げる
			ChangeVolumeSoundMem(255 * 50 / 100, SF_TIME.handle);  //50%の音量にする
			PlaySoundMem(SF_TIME.handle, DX_PLAYTYPE_BACK);		   //バックグラウンド再生
		}
	}
	else
	{
		//現在の時間を取得
		int NowCount = GetNowCount();

		//制限時間(降順で時間表示) - (現在の時間 - 基準の時間)、ミリ秒単位
		ElaTime = TimeLimit - (NowCount - StartTime);

		//効果音が流れていないなら
		if (CheckSoundMem(SF_TIME.handle) == 0)
		{
			//効果音の音量を下げる
			ChangeVolumeSoundMem(255 * 50 / 100, SF_TIME.handle);  //50%の音量にする
			PlaySoundMem(SF_TIME.handle, DX_PLAYTYPE_BACK);		   //バックグラウンド再生
		}

		//経過時間が0秒になったら(・・・3,2,1 で終了させるため <=)
		if (ElaTime <= 0)
		{
			Jude = JUDE_OVER;  //判定はゲームオーバー

			GameScene = GAME_SCENE_END;  //エンド画面へ

			//画像の消去・初期化
			MY_INIT();

			//BGMが流れているなら
			if (CheckSoundMem(PLAY_BGM.handle) != 0)
			{
				StopSoundMem(PLAY_BGM.handle);   //BGMを止める
			}

			return;
		}

		//最初の問題を表示
		if (First_Qus)
		{
			animal[order].IsDraw = TRUE;			//表示
			order++;

			//乱数を取得
			Mask_num = GetRand(GiveMask) + 1;

			First_Qus = FALSE;  //次以降は表示しない
		}

		//エンターキーを押す際の行動パターン(マスクを「あげる」)
		if (MY_KEYDOWN_1SECOND(KEY_INPUT_RETURN) == TRUE)
		{
			//加算していく
			Mask_sum += Mask_num;

			//乱数を取得
			Mask_num = GetRand(GiveMask) + 1;

			//一定量を超えたら終了
			if (Mask_sum > HaveMask)
			{
				Jude = JUDE_OVER;  //判定は「失敗」

				GameScene = GAME_SCENE_END;  //エンド画面に移動

				//画像の消去・初期化
				MY_INIT();

				//BGMが流れているなら
				if (CheckSoundMem(PLAY_BGM.handle) != 0)
				{
					StopSoundMem(PLAY_BGM.handle);   //BGMを止める
				}

				return;
			}

			//単体で表示する
			if (order == 0)  //1番目用
			{
				//描画する
				animal[ANIMAL_MAX - 1].IsDraw = FALSE;  //一個前の絵を消す(最後尾の絵)
				animal[order].IsDraw = TRUE;			//表示
				order++;
			}
			else if (order == ANIMAL_MAX - 1)  //最後尾用
			{
				//描画
				animal[order - 1].IsDraw = FALSE;		//一個前の絵を消す
				animal[order].IsDraw = TRUE;			//表示
				order = 0;								//最初から
			}
			else  //それ以外
			{
				//描画
				animal[order - 1].IsDraw = FALSE;		//一個前の絵を消す
				animal[order].IsDraw = TRUE;			//表示
				order++;
			}
			//再度、基準時間を更新する
			StartTime = GetNowCount();
		}

		//デリートーキーを押す際の行動パターン(マスクを「あげない」)
		if (MY_KEYDOWN_1SECOND(KEY_INPUT_DELETE) == TRUE)
		{
			//成功パターン (マスクが無くなる手前でストップ)
			if ((Mask_sum + Mask_num) >= HaveMask)
			{
				Jude = JUDE_CLEAR;  //判定は「成功」

				GameScene = GAME_SCENE_END;  //エンド画面に移動

				//画像の消去・初期化
				MY_INIT();

				//BGMが流れているなら
				if (CheckSoundMem(PLAY_BGM.handle) != 0)
				{
					StopSoundMem(PLAY_BGM.handle);   //BGMを止める
				}

				return;
			}
			//失敗パターン (マスクが手元に残ってない)
			else if ((Mask_sum + Mask_num) < HaveMask)
			{
				Jude = JUDE_OVER;  //判定は「失敗」

				GameScene = GAME_SCENE_END;  //エンド画面に移動

				//画像の消去・初期化
				MY_INIT();

				//BGMが流れているなら
				if (CheckSoundMem(PLAY_BGM.handle) != 0)
				{
					StopSoundMem(PLAY_BGM.handle);   //BGMを止める
				}

				return;
			}
		}
	}

	return;
}

//プレイ画面の描画
VOID MY_PLAY_DRAW(VOID)
{
	//プレイ画面の背景
	DrawGraph(ImagePLAYENDBG.x, ImagePLAYENDBG.y, ImagePLAYENDBG.handle, TRUE);

	if (First_flg)  //最初のカウントダウン
	{
		DrawFormatStringToHandle(GAME_WIDTH / 2 - 125, GAME_HEIGHT / 2 - 120, GetColor(255, 0, 0), CD_TANUKI.handle, "%d", (ElaTime / 1000) + 1);
	}
	else 
	{
		//制限時間の表示
		//1000で割って「ミリ秒単位」から「秒単位」に
		//0 が出てきてしまうので +1する
		if((ElaTime / 1000 + 1) <= 3)
			DrawFormatStringToHandle(0, 0, GetColor(255, 0, 0), TANUKI.handle, "%d秒", (ElaTime / 1000) + 1);
		else
			DrawFormatStringToHandle(0, 0, GetColor(255, 255, 255), TANUKI.handle, "%d秒", (ElaTime / 1000) + 1);
	}

	//トークシーンの背景
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 192);  //透明度を25%上げる
	DrawBox(0, GAME_HEIGHT - 180, GAME_WIDTH, GAME_HEIGHT, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);  //元に戻す

	//動物の情報を生成
	for (int cnt = 0; cnt < ANIMAL_MAX; cnt++)
	{
		//描画できるなら
		if (animal[cnt].IsDraw == TRUE)
		{
			//画像を1.5倍に縮小表示
			DrawExtendGraph(
				animal[cnt].x, animal[cnt].y,
				animal[cnt].x + animal[cnt].width / 1.5, animal[cnt].y + (int)animal[cnt].height / 1.5,
				GHandle[cnt], TRUE
			);

			//欲しいマスクの表示
			DrawFormatStringToHandle(170, GAME_HEIGHT - 170, GetColor(255, 255, 255), TANUKI.handle, "マスク %d個 ちょうだい！！", Mask_num);

			//デバッグ用(マスクの個数を表示する)
			//DrawFormatStringToHandle(0, 0, GetColor(255, 255, 255), TANUKI.handle, "%d個", Mask_sum);

			//「あげる？」「あげない？」の追加
			DrawBox(200, GAME_HEIGHT - 100, 410, GAME_HEIGHT - 50, GetColor(255, 0, 0), TRUE);
			DrawStringToHandle(200, GAME_HEIGHT - 100, "あげる？", GetColor(255, 255, 255), TANUKI.handle);
			DrawBox(500, GAME_HEIGHT - 100, 770, GAME_HEIGHT - 50, GetColor(0, 0, 255), TRUE);
			DrawStringToHandle(500, GAME_HEIGHT - 100, "あげない？", GetColor(255, 255, 255), TANUKI.handle);
		}
	}

	return;
}

//エンド画面
VOID MY_END(VOID)
{
	MY_END_PROC();    //エンド画面の処理
	MY_END_DRAW();    //エンド画面の描画

	return;
}

//エンド画面の処理
VOID MY_END_PROC(VOID)
{
	//クリアか失敗か判定
	switch (Jude)
	{
	case JUDE_CLEAR:  //クリアパターン
		//BGMが流れていないなら
		if (CheckSoundMem(END_CLEAR_BGM.handle) == 0)
		{
			//BGMの音量を下げる
			ChangeVolumeSoundMem(255 * 50 / 100, END_CLEAR_BGM.handle);  //50%の音量にする
			PlaySoundMem(END_CLEAR_BGM.handle, DX_PLAYTYPE_LOOP);		 //バックグラウンド再生
		}
		break;

	case JUDE_OVER:  //失敗パターン
		//BGMが流れていないなら
		if (CheckSoundMem(END_FAIL_BGM.handle) == 0)
		{
			//BGMの音量を下げる
			ChangeVolumeSoundMem(255 * 50 / 100, END_FAIL_BGM.handle);  //50%の音量にする
			PlaySoundMem(END_FAIL_BGM.handle, DX_PLAYTYPE_LOOP);		//バックグラウンド再生
		}
		break;
	}

	//バックスペースキーを押したら、スタートシーンへ移動する
	if (MY_KEY_DOWN(KEY_INPUT_BACK) == TRUE)
	{
		HaveMask = 0;					//初期化
		GiveMask = 0;					//初期化
		GameScene = GAME_SCENE_START;   //スタート画面に移動

		//BGMが流れているなら(クリアパターン)
		if (CheckSoundMem(END_CLEAR_BGM.handle) != 0)
		{
			StopSoundMem(END_CLEAR_BGM.handle);   //BGMを止める
		}
		//BGMが流れているなら(失敗パターン)
		if (CheckSoundMem(END_FAIL_BGM.handle) != 0)
		{
			StopSoundMem(END_FAIL_BGM.handle);   //BGMを止める
		}
	}

	return;
}

//エンド画面の描画
VOID MY_END_DRAW(VOID)
{
	//背景を描画
	DrawGraph(ImagePLAYENDBG.x, ImagePLAYENDBG.y, ImagePLAYENDBG.handle, TRUE);

	//成功、失敗に応じて表示する画像を変更
	switch (Jude)
	{
		//クリアパターン
	case JUDE_CLEAR:
		DrawGraph(ImageEndClear.x, ImageEndClear.y, ImageEndClear.handle, TRUE);
		DrawGraph(ImageMessage1.x, ImageMessage1.y, ImageMessage1.handle, TRUE);
		break;

		//失敗パターン
	case JUDE_OVER:
		DrawGraph(ImageEndFail.x, ImageEndFail.y, ImageEndFail.handle, TRUE);
		DrawGraph(ImageMessage2.x, ImageMessage2.y, ImageMessage2.handle, TRUE);
		break;
	}

	DrawGraph(ImageBack_BSK.x, ImageBack_BSK.y, ImageBack_BSK.handle, TRUE);

	return;
}

//画像をまとめて読み込む関数
BOOL MY_LOAD_IMAGE(VOID)
{
	//スタート画面の背景画像
	strcpy_s(ImageSTARTBG.path, IMAGE_START_BG_PATH);  //パスの設定
	ImageSTARTBG.handle = LoadGraph(ImageSTARTBG.path);   //読み込み
	if (ImageSTARTBG.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), IMAGE_START_BG_PATH, IMAGE_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}
	GetGraphSize(ImageSTARTBG.handle, &ImageSTARTBG.width, &ImageSTARTBG.height);  //幅と高さを取得
	ImageSTARTBG.x = GAME_WIDTH / 2 - ImageSTARTBG.width / 2;		//X位置を決める
	ImageSTARTBG.y = GAME_HEIGHT / 2 - ImageSTARTBG.height / 2;     //Y位置を決める

	//タイトル画像
	strcpy_s(ImageTITLE.path, IMAGE_TITLE_PATH);  //パスの設定
	ImageTITLE.handle = LoadGraph(ImageTITLE.path);   //読み込み
	if (ImageTITLE.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), IMAGE_TITLE_PATH, IMAGE_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}
	GetGraphSize(ImageTITLE.handle, &ImageTITLE.width, &ImageTITLE.height);  //幅と高さを取得
	ImageTITLE.x = GAME_WIDTH / 2 - ImageTITLE.width / 2;		//X位置を決める
	ImageTITLE.y = 25;											//Y位置を決める

	//プレイ画面とエンド画面の背景画像
	strcpy_s(ImagePLAYENDBG.path, IMAGE_PLAY_BG_PATH);  //パスの設定
	ImagePLAYENDBG.handle = LoadGraph(ImagePLAYENDBG.path);   //読み込み
	if (ImagePLAYENDBG.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), IMAGE_PLAY_BG_PATH, IMAGE_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}
	GetGraphSize(ImagePLAYENDBG.handle, &ImagePLAYENDBG.width, &ImagePLAYENDBG.height);  //幅と高さを取得
	ImagePLAYENDBG.x = GAME_WIDTH / 2 - ImagePLAYENDBG.width / 2;	//X位置を決める
	ImagePLAYENDBG.y = GAME_HEIGHT / 2 - ImagePLAYENDBG.height / 2; //Y位置を決める

	//ボタンの画像(ルール説明行き)
	strcpy_s(ImageMENUBtn.path, IMAGE_MENU_BUTTON_PATH);  //パスの設定
	ImageMENUBtn.handle = LoadGraph(ImageMENUBtn.path);   //読み込み
	if (ImageMENUBtn.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), IMAGE_MENU_BUTTON_PATH, IMAGE_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}
	GetGraphSize(ImageMENUBtn.handle, &ImageMENUBtn.width, &ImageMENUBtn.height);  //幅と高さを取得
	ImageMENUBtn.x = GAME_WIDTH - ImageMENUBtn.width - 20;			//X位置を決める
	ImageMENUBtn.y = GAME_HEIGHT - ImageMENUBtn.height - 20;		//Y位置を決める

	//操作説明画面の背景
	strcpy_s(ImageMENUBG.path, IMAGE_MENU_BG_PATH);  //パスの設定
	ImageMENUBG.handle = LoadGraph(ImageMENUBG.path);   //読み込み
	if (ImageMENUBG.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), IMAGE_MENU_BG_PATH, IMAGE_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}
	GetGraphSize(ImageMENUBG.handle, &ImageMENUBG.width, &ImageMENUBG.height);  //幅と高さを取得
	ImageMENUBG.x = GAME_WIDTH / 2 - ImageMENUBG.width / 2;			//X位置を決める
	ImageMENUBG.y = GAME_HEIGHT / 2 - ImageMENUBG.height / 2;		//Y位置を決める

	//動物チップ
	int animalRes = LoadDivGraph(
		GAME_animal1_CHIP_PATH,										//動物チップのパス
		CHIP_DIV_NUM, GAME_animal1_DIV_TATE, GAME_animal1_DIV_YOKO, //分割する数
		CHIP_DIV_WIDTH, CHIP_DIV_HEIGHT,							//画像を分割する幅と高さ
		GHandle														//ハンドルをここに保存する
	);

	//正しく分割できなかったら
	if (animalRes == -1)
	{
		MessageBox(GetMainWindowHandle(), GAME_animal1_CHIP_PATH, IMAGE_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}

	//それぞれ幅と高さを取得
	for (int cnt = 0; cnt < ANIMAL_MAX; cnt++)
	{
		GetGraphSize(GHandle[cnt], &animal[cnt].width, &animal[cnt].height);
	}

	//情報を与える
	for (int cnt = 0; cnt < ANIMAL_MAX; cnt++)
	{
		strcpyDx(animal[cnt].path, TEXT(GAME_animal1_CHIP_PATH));						//パス
		animal[cnt].width = animal[0].width;											//幅
		animal[cnt].height = animal[0].height;											//高さ
		animal[cnt].x = (GAME_WIDTH / 2 - animal[0].width / 2) + animal[0].width / 10;	//X座標
		animal[cnt].y = animal[0].height / 10;											//Y座標
		animal[cnt].IsDraw = FALSE;														//描画できるか(初期値は 描画できない)
	}

	//クリアの画像
	strcpy_s(ImageEndClear.path, IMAGE_END_CLEAR_PATH);  //パスの設定
	ImageEndClear.handle = LoadGraph(ImageEndClear.path);   //読み込み
	if (ImageEndClear.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), IMAGE_END_CLEAR_PATH, IMAGE_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}
	GetGraphSize(ImageEndClear.handle, &ImageEndClear.width, &ImageEndClear.height);  //幅と高さを取得
	ImageEndClear.x = GAME_WIDTH / 2 - ImageEndClear.width / 2;			//X位置を決める
	ImageEndClear.y = GAME_HEIGHT / 2 - ImageEndClear.height / 2 - 135;  //Y位置を決める

	//失敗の画像
	strcpy_s(ImageEndFail.path, IMAGE_END_FAIL_PATH);  //パスの設定
	ImageEndFail.handle = LoadGraph(ImageEndFail.path);   //読み込み
	if (ImageEndFail.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), IMAGE_END_FAIL_PATH, IMAGE_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}
	GetGraphSize(ImageEndFail.handle, &ImageEndFail.width, &ImageEndFail.height);  //幅と高さを取得
	ImageEndFail.x = GAME_WIDTH / 2 - ImageEndFail.width / 2;			//X位置を決める
	ImageEndFail.y = GAME_HEIGHT / 2 - ImageEndFail.height / 2 - 135;    //Y位置を決める

	//操作説明の1枚目の背景
	strcpy_s(ImageMENU1.image.path, IMAGE_MENU_1_PATH);  //パスの設定
	ImageMENU1.image.handle = LoadGraph(ImageMENU1.image.path);   //読み込み
	if (ImageMENU1.image.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), IMAGE_MENU_1_PATH, IMAGE_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}
	GetGraphSize(ImageMENU1.image.handle, &ImageMENU1.image.width, &ImageMENU1.image.height);  //幅と高さを取得
	ImageMENU1.image.x = 0;				//X位置を決める
	ImageMENU1.image.y = 0;				//Y位置を決める
	ImageMENU1.IsDraw = TRUE;			//最初は描画できる

	//操作説明の2枚目の背景
	strcpy_s(ImageMENU2.image.path, IMAGE_MENU_2_PATH);  //パスの設定
	ImageMENU2.image.handle = LoadGraph(ImageMENU2.image.path);   //読み込み
	if (ImageMENU2.image.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), IMAGE_MENU_2_PATH, IMAGE_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}
	GetGraphSize(ImageMENU2.image.handle, &ImageMENU2.image.width, &ImageMENU2.image.height);  //幅と高さを取得
	ImageMENU2.image.x = 0;				//X位置を決める
	ImageMENU2.image.y = 0;				//Y位置を決める
	ImageMENU2.IsDraw = FALSE;			//最初は描画しない

	//お客様からのメッセージ(成功パターン)
	strcpy_s(ImageMessage1.path, IMAGE_MESSAGE_1_PATH);  //パスの設定
	ImageMessage1.handle = LoadGraph(ImageMessage1.path);   //読み込み
	if (ImageMessage1.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), IMAGE_MESSAGE_1_PATH, IMAGE_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}
	GetGraphSize(ImageMessage1.handle, &ImageMessage1.width, &ImageMessage1.height);  //幅と高さを取得
	ImageMessage1.x = GAME_WIDTH / 2 - ImageMessage1.width / 2;			//X位置を決める
	ImageMessage1.y = ImageEndClear.y + ImageEndClear.height;			//Y位置を決める

	//お客様からのメッセージ(失敗パターン)
	strcpy_s(ImageMessage2.path, IMAGE_MESSAGE_2_PATH);  //パスの設定
	ImageMessage2.handle = LoadGraph(ImageMessage2.path);   //読み込み
	if (ImageMessage2.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), IMAGE_MESSAGE_2_PATH, IMAGE_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}
	GetGraphSize(ImageMessage2.handle, &ImageMessage2.width, &ImageMessage2.height);  //幅と高さを取得
	ImageMessage2.x = GAME_WIDTH / 2 - ImageMessage2.width / 2;			//X位置を決める
	ImageMessage2.y = ImageEndFail.y + ImageEndFail.height;				//Y位置を決める

	strcpy_s(ImageBack_BSK.path, IMAGE_BACK_BACKSPACE_KEY_PATH);  //パスの設定
	ImageBack_BSK.handle = LoadGraph(ImageBack_BSK.path);   //読み込み
	if (ImageBack_BSK.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), IMAGE_BACK_BACKSPACE_KEY_PATH, IMAGE_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}
	GetGraphSize(ImageBack_BSK.handle, &ImageBack_BSK.width, &ImageBack_BSK.height);  //幅と高さを取得
	ImageBack_BSK.x = GAME_WIDTH - ImageBack_BSK.width - 10;			//X位置を決める
	ImageBack_BSK.y = 0 + 10;												//Y位置を決める

	//Easyモードへ促すためのボタン
	strcpy_s(ImageEasyMode.path, IMAGE_EASYMODE_PATH);  //パスの設定
	ImageEasyMode.handle = LoadGraph(ImageEasyMode.path);   //読み込み
	if (ImageEasyMode.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), IMAGE_EASYMODE_PATH, IMAGE_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}
	GetGraphSize(ImageEasyMode.handle, &ImageEasyMode.width, &ImageEasyMode.height);  //幅と高さを取得
	ImageEasyMode.x = GAME_WIDTH / 2 - ImageEasyMode.width / 2;			//X位置を決める
	ImageEasyMode.y = ImageTITLE.y + ImageTITLE.height + 45;			//Y位置を決める

	//Normalモードへ促すためのボタン
	strcpy_s(ImageNormalMode.path, IMAGE_NORMALMODE_PATH);  //パスの設定
	ImageNormalMode.handle = LoadGraph(ImageNormalMode.path);   //読み込み
	if (ImageNormalMode.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), IMAGE_NORMALMODE_PATH, IMAGE_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}
	GetGraphSize(ImageNormalMode.handle, &ImageNormalMode.width, &ImageNormalMode.height);  //幅と高さを取得
	ImageNormalMode.x = GAME_WIDTH / 2 - ImageNormalMode.width / 2;			//X位置を決める
	ImageNormalMode.y = ImageEasyMode.y + ImageEasyMode.height + 30;		//Y位置を決める

	//Hardモードへ促すためのボタン
	strcpy_s(ImageHardMode.path, IMAGE_HARDMODE_PATH);  //パスの設定
	ImageHardMode.handle = LoadGraph(ImageHardMode.path);   //読み込み
	if (ImageHardMode.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), IMAGE_HARDMODE_PATH, IMAGE_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}
	GetGraphSize(ImageHardMode.handle, &ImageHardMode.width, &ImageHardMode.height);  //幅と高さを取得
	ImageHardMode.x = GAME_WIDTH / 2 - ImageHardMode.width / 2;			//X位置を決める
	ImageHardMode.y = ImageNormalMode.y + ImageNormalMode.height + 30;	//Y位置を決める

	//ボタンの画像(level説明行き)
	strcpy_s(ImageLEVELBtn.path, IMAGE_LEVEL_BUTTON_PATH);  //パスの設定
	ImageLEVELBtn.handle = LoadGraph(ImageLEVELBtn.path);   //読み込み
	if (ImageLEVELBtn.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), IMAGE_LEVEL_BUTTON_PATH, IMAGE_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}
	GetGraphSize(ImageLEVELBtn.handle, &ImageLEVELBtn.width, &ImageLEVELBtn.height);  //幅と高さを取得
	ImageLEVELBtn.x = 0 + 20;											//X位置を決める
	ImageLEVELBtn.y = GAME_HEIGHT - ImageLEVELBtn.height - 20;			//Y位置を決める

	//level説明
	strcpy_s(ImageLEVELExp.path, IMAGE_LEVEL_EXP_PATH);  //パスの設定
	ImageLEVELExp.handle = LoadGraph(ImageLEVELExp.path);   //読み込み
	if (ImageLEVELExp.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), IMAGE_LEVEL_EXP_PATH, IMAGE_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}
	GetGraphSize(ImageLEVELExp.handle, &ImageLEVELExp.width, &ImageLEVELExp.height);  //幅と高さを取得
	ImageLEVELExp.x = 0;												//X位置を決める
	ImageLEVELExp.y = 0;												//Y位置を決める

	return TRUE;
}

//画像をまとめて削除する関数
VOID MY_DELETE_IMAGE(VOID)
{
	DeleteGraph(ImageSTARTBG.handle);		//スタート画面の背景
	DeleteGraph(ImageTITLE.handle);			//タイトル画像
	DeleteGraph(ImagePLAYENDBG.handle);		//プレイ・エンド画面の背景
	DeleteGraph(ImageMENUBtn.handle);		//ボタン
	DeleteGraph(ImageMENUBG.handle);        //操作説明画面の背景

	//動物チップ
	for (int i_num = 0; i_num < CHIP_DIV_NUM; i_num++)
	{
		DeleteGraph(animal[0].handle[i_num]);
	}

	//成功・失敗の画像
	DeleteGraph(ImageEndClear.handle);
	DeleteGraph(ImageEndFail.handle);

	//操作説明関連の画像
	DeleteGraph(ImageMENU1.image.handle);
	DeleteGraph(ImageMENU2.image.handle);

	//メッセージの画像
	DeleteGraph(ImageMessage1.handle);
	DeleteGraph(ImageMessage2.handle);

	//スタート画面に戻るよう促すための画像
	DeleteGraph(ImageBack_BSK.handle);

	//各レベルへ促すためのボタン
	DeleteGraph(ImageEasyMode.handle);
	DeleteGraph(ImageNormalMode.handle);
	DeleteGraph(ImageHardMode.handle);

	//level説明関連の画像
	DeleteGraph(ImageLEVELBtn.handle);
	DeleteGraph(ImageLEVELExp.handle);

	return;
}

//画像の消去・初期化
VOID MY_INIT(VOID)
{
	//終了する際は全て消す
	for (int cnt = 0; cnt < ANIMAL_MAX; cnt++)
	{
		animal[cnt].IsDraw = FALSE;
	}
	//再開しても最初から
	order = 0;

	//初期化
	Mask_num = 0;
	Mask_sum = 0;
	First_flg = TRUE;
	First_Qus = TRUE;

	return;
}

//フォントを一時的にインストール
BOOL MY_FONT_INSTALL_ONCE(VOID)
{
	//フォントを一時的に読み込み
	if (AddFontResourceEx(FONT_TANUKI_PATH, FR_PRIVATE, NULL) == 0)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), FONT_TANUKI_NAME, FONT_INSTALL_ERR_TITLE, MB_OK);
		return FALSE;
	}

	return TRUE;
}

//フォントを一時的にアンインストール
VOID MY_FONT_UNINSTALL_ONCE(VOID)
{
	//フォントを一時的にアンインストール
	RemoveFontResourceEx(FONT_TANUKI_PATH, FR_PRIVATE, NULL);

	return;
}

//フォントを読み込む
BOOL MY_FONT_CREATE(VOID)
{
	//フォントデータを作成
	strcpy_s(TANUKI.path, sizeof(TANUKI.path), FONT_TANUKI_PATH);  //パスをコピー
	strcpy_s(TANUKI.name, sizeof(TANUKI.name), FONT_TANUKI_NAME);  //フォント名をコピー
	TANUKI.handle = -1;								//ハンドルを初期化
	TANUKI.size = 50;								//サイズ: 50
	TANUKI.bold = 4;								//太さ: 4
	TANUKI.type = DX_FONTTYPE_ANTIALIASING_EDGE;    //アンチエイリアシング付き

	//フォントハンドル作成
	TANUKI.handle = CreateFontToHandle(TANUKI.name, TANUKI.size, TANUKI.bold, TANUKI.type);
	//フォントハンドルを作成できないときエラー
	if (TANUKI.handle == -1) { MessageBox(GetMainWindowHandle(), FONT_TANUKI_NAME, FONT_CREATE_ERR_TITLE, MB_OK); return FALSE; }

	//フォントデータを作成(カウントダウン用)
	strcpy_s(CD_TANUKI.path, sizeof(CD_TANUKI.path), FONT_TANUKI_PATH);  //パスをコピー
	strcpy_s(CD_TANUKI.name, sizeof(CD_TANUKI.name), FONT_TANUKI_NAME);  //フォント名をコピー
	CD_TANUKI.handle = -1;								//ハンドルを初期化
	CD_TANUKI.size = 300;								//サイズ: 300
	CD_TANUKI.bold = 5;								    //太さ: 5
	CD_TANUKI.type = DX_FONTTYPE_ANTIALIASING_EDGE;     //アンチエイリアシング付き

	//フォントハンドル作成
	CD_TANUKI.handle = CreateFontToHandle(CD_TANUKI.name, CD_TANUKI.size, CD_TANUKI.bold, CD_TANUKI.type);
	//フォントハンドルを作成できないときエラー
	if (CD_TANUKI.handle == -1) { MessageBox(GetMainWindowHandle(), FONT_TANUKI_NAME, FONT_CREATE_ERR_TITLE, MB_OK); return FALSE; }


	return TRUE;
}

//フォントを削除する
VOID MY_FONT_DELETE(VOID)
{
	//フォントデータを削除
	DeleteFontToHandle(TANUKI.handle);

	return;
}

//音楽をまとめて読み込む関数
BOOL MY_LOAD_MUSIC(VOID)
{
	//スタート画面の音楽
	strcpy_s(START_BGM.path, MUSIC_START_BGM_PATH);		//パスの設定
	START_BGM.handle = LoadSoundMem(START_BGM.path);	//読み込み
	if (START_BGM.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), MUSIC_START_BGM_PATH, MUSIC_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}

	//プレイ画面の音楽
	strcpy_s(PLAY_BGM.path, MUSIC_PLAY_BGM_PATH);		//パスの設定
	PLAY_BGM.handle = LoadSoundMem(PLAY_BGM.path);		//読み込み
	if (PLAY_BGM.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), MUSIC_PLAY_BGM_PATH, MUSIC_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}

	//エンド画面(クリアパターン)の音楽
	strcpy_s(END_CLEAR_BGM.path, MUSIC_END_CLEAR_PATH);			//パスの設定
	END_CLEAR_BGM.handle = LoadSoundMem(END_CLEAR_BGM.path);	//読み込み
	if (END_CLEAR_BGM.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), MUSIC_END_CLEAR_PATH, MUSIC_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}

	//エンド画面(失敗パターン)の音楽
	strcpy_s(END_FAIL_BGM.path, MUSIC_END_FAIL_PATH);			//パスの設定
	END_FAIL_BGM.handle = LoadSoundMem(END_FAIL_BGM.path);		//読み込み
	if (END_FAIL_BGM.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), MUSIC_END_FAIL_PATH, MUSIC_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}

	//時間用の効果音
	strcpy_s(SF_TIME.path, SOUND_EFFECT_TIME_PATH);		//パスの設定
	SF_TIME.handle = LoadSoundMem(SF_TIME.path);		//読み込み
	if (SF_TIME.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), SOUND_EFFECT_TIME_PATH, MUSIC_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}

	//ボタンの効果音
	strcpy_s(SF_BUTTON.path, SOUND_EFFECT_BUTTON_PATH);		//パスの設定
	SF_BUTTON.handle = LoadSoundMem(SF_BUTTON.path);		//読み込み
	if (SF_BUTTON.handle == -1)
	{
		//エラーメッセージ表示
		MessageBox(GetMainWindowHandle(), SOUND_EFFECT_BUTTON_PATH, MUSIC_LOAD_ERR_TITLE, MB_OK);
		return FALSE;
	}

	return TRUE;
}

//音楽をまとめて削除する関数
VOID MY_DELETE_MUSIC(VOID)
{
	DeleteSoundMem(START_BGM.handle);
	DeleteSoundMem(PLAY_BGM.handle);
	DeleteSoundMem(END_CLEAR_BGM.handle);
	DeleteSoundMem(END_FAIL_BGM.handle);
	DeleteSoundMem(SF_TIME.handle);
	DeleteSoundMem(SF_BUTTON.handle);

	return;
}