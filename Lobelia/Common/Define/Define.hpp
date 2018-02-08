#pragma once
#define LOBELIA_ENGINE "LobeliaEngine"
#define ENGINE_VERSION LOBELIA_ENGINE
#define APPLICATION_NAME "LobeliaEngine"

#ifdef _DEBUG
#define USE_IMGUI_AND_CONSOLE
#endif

//デフォルトのシェーダーの登録名
//VS
#define D_VS3D_S "VS3D"
#define D_VS3D_D "VS3DS"
#define D_VS3D_IS "VS3DSINST"
#define D_VS3D_ID "VS3DDINST"

//PS
#define D_PS3D "PS3D"

//デフォルトパイプラインの登録名
#define D_PIPE3D_S "Default3D"
#define D_PIPE3D_D "Default3DSkin"
#define D_PIPE3D_IS "Default3DInstNoSkin"
#define D_PIPE3D_ID "Default3DInst"

#define D_PS3D_INAME_LAMBERT "Lambert"
#define D_PS3D_INAME_LAMBERT_FOG "Fog"
#define D_PS3D_INAME_PHONG "Phong"

#define D_PS3D_INS_LAMB_ID 0
#define D_PS3D_INS_FOG_ID 1
#define D_PS3D_INS_PHONG_ID 2

//ステンシルオフ
#define D_SOFF_ZOFF "Z Off Stencil Off"
#define D_SOFF_ZON "Z On Stencil Off"
//型抜き用の型作成用
#define D_SWRITER_ZOFF "Z Off Stencil Write"
#define D_SWRITE_Z_ON "Z On Stencil Write"
//作成した型の中にのみ描画
#define D_SREAD_ZOFF "Z Off Stencil Read"
#define D_SREAD_ZON "Z On Stencil Read"

#define Interface abstract

#define s_cast static_cast
#define f_cast s_cast<float>
#define i_cast s_cast<int>
#define r_cast reinterpret_cast
#define d_cast dynamic_cast

//template<class T> using u_ptr = std::unique_ptr<T>;
//template<class T> using s_ptr = std::shared_ptr<T>;
//template<class T> using w_ptr = std::weak_ptr<T>;

/**@def ソースファイル名取得*/
#define FILE_NAME __FILE__
/**@def 関数名取得*/
#define FUNCTION_NAME __func__
/**@def 実行している行取得*/
#define EXECUTE_LINE __LINE__
/**@def 変数名取得*/
#define VARIABLE_NAME(variable) # variable
/**@def catch*/
#define CATCH catch
/**@def throw*/
#define THROW throw
/**@def アライメント*/
#define ALIGN(n) __declspec(align(n))

/**@brief forマクロ*/
#define FOR(index, max_value) for(int index = 0; index < max_value; index++)
