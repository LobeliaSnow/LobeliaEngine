#pragma once
/**
*@file HdxMath.h
*@brief 数学系が定義されています
*@brief コメントをつける
*@author Lobelia_Snow
*/

#define _XM_NO_INTRINSICS_
#include <DirectXMath.h>
#include <math.h>
#include <limits>
#include <array>
#ifndef PI
#define PI 3.141592653589793f
#endif

namespace Lobelia::Math {
	/**
	*@brief floatの等値判定
	*@param[in] 左辺値
	*@param[in] 右辺値
	*@param[in] 許容値 デフォルトではものすごく小さい値が設定される
	*/
	__forceinline bool FloatEqual(float x, float y, float threshold = FLT_EPSILON) noexcept { return  (fabsf(x - y) <= threshold); };

	/**
	*@brief 度数法から弧度法に変換
	*@param[in] 度数法での角度
	*@return 弧度法での角度
	*/
	__forceinline constexpr float DegreeToRadian(float degree) { return degree * PI / 180; }

	/**
	*@brief 弧度法から度数法に変換
	*@param[in] 弧度法での角度
	*@return 度数法での角度
	*/
	__forceinline constexpr float RadianToDegree(float radian) { return radian * (180 / PI); }

	struct Vector2 {
	public:
		union {
			struct { float x, y; };
			float v[2];
		};
		/**
		*@brief 型がintのVector2<br>
		*値を取る為のみ使用なので、メンバ関数は提供しない
		*/
		struct IVector2 {
			union {
				struct { int x, y; };
				int v[2];
			};
			__forceinline IVector2(int x, int y) :x(x), y(y) {}
		};
		//__forceinline Vector2(float x, float y) noexcept : x(x), y(y) {}
		__forceinline constexpr Vector2(float x, float y) noexcept : x(x), y(y) {}
		__forceinline Vector2(IVector2 v) noexcept : Vector2(static_cast<float>(v.x), static_cast<float>(v.y)) {}
		__forceinline Vector2()noexcept : Vector2(0.0f, 0.0f) {}
		/**
		*@brief 値をint型で取得する関数
		*/
		__forceinline IVector2 Get()const  noexcept { return IVector2(static_cast<int>(x), static_cast<int>(y)); }
		/**
		*@brief int型のIVector2からの直接Vector2に代入
		*/
		__forceinline void Set(IVector2 v)noexcept {
			for (int i = 0; i < 2; i++) {
				this->v[i] = static_cast<float>(v.v[i]);
			}
		}
		/**@brief 距離(平方根あり)*/
		__forceinline float Length()const { return sqrtf(x*x + y * y); }
		/**@brief 距離(平方根なし)*/
		__forceinline float LengthSq()const { return (x*x + y * y); }
		/**@brief 正規化*/
		__forceinline void Normalize() {
			float l = Length();
			if (l != 0.0f) { x /= l;	y /= l; }
		}
		__forceinline bool IsEqual(const Vector2& v) { return (FloatEqual(x, v.x) && FloatEqual(y, v.y)); }
		/**@brief 内積*/
		__forceinline static float Dot(const Vector2& v1, const Vector2& v2) { return (v1.x*v2.x + v1.y*v2.y); }
		__forceinline static float Cross(const Vector2& v1, const Vector2& v2) { return v1.x*v2.y - v1.y*v2.x; }
		__forceinline Vector2 operator +(const Vector2& v) { return Vector2(x + v.x, y + v.y); }
		__forceinline Vector2 operator -(const Vector2& v) { return Vector2(x - v.x, y - v.y); }
		__forceinline Vector2 operator *(const Vector2& v) { return Vector2(x * v.x, y * v.y); }
		__forceinline Vector2 operator /(const Vector2& v) { return Vector2(x / v.x, y / v.y); }
		__forceinline Vector2 operator +(float scala) { return Vector2(x + scala, y + scala); }
		__forceinline Vector2 operator -(float scala) { return Vector2(x - scala, y - scala); }
		__forceinline Vector2 operator *(float scala) { return Vector2(x * scala, y * scala); }
		__forceinline Vector2 operator /(float scala) { return Vector2(x / scala, y / scala); }
		__forceinline Vector2 operator =(float scala) { x = scala; y = scala; return *this; }
		__forceinline Vector2& operator +=(const Vector2& v) { x += v.x;	y += v.y; return *this; }
		__forceinline Vector2& operator -=(const Vector2& v) { x -= v.x;	y -= v.y; return *this; }
		__forceinline Vector2& operator *=(const Vector2& v) { x *= v.x;	y *= v.y; return *this; }
		__forceinline Vector2& operator /=(const Vector2& v) { x /= v.x;	y /= v.y; return *this; }
		__forceinline Vector2& operator +=(float scala) { x += scala;	y += scala; return *this; }
		__forceinline Vector2& operator -=(float scala) { x -= scala;		y -= scala; return *this; }
		__forceinline Vector2& operator *=(float scala) { x *= scala;	y *= scala; return *this; }
		__forceinline Vector2& operator /=(float scala) { x /= scala;	y /= scala; return *this; }
		__forceinline bool operator ==(const Vector2& v) { return IsEqual(v); }
		__forceinline bool operator !=(const Vector2& v) { return !IsEqual(v); }
	};
	__forceinline Vector2 operator +(const Vector2& v0, const Vector2& v1) { return Vector2(v0.x + v1.x, v0.y + v1.y); }
	__forceinline Vector2 operator -(const Vector2& v0, const Vector2& v1) { return Vector2(v0.x - v1.x, v0.y - v1.y); }
	__forceinline Vector2 operator *(const Vector2& v0, const Vector2& v1) { return Vector2(v0.x * v1.x, v0.y * v1.y); }
	__forceinline Vector2 operator /(const Vector2& v0, const Vector2& v1) { return Vector2(v0.x / v1.x, v0.y / v1.y); }
	__forceinline Vector2 operator +(const Vector2& v, float scala) { return Vector2(v.x + scala, v.y + scala); }
	__forceinline Vector2 operator -(const Vector2& v, float scala) { return Vector2(v.x - scala, v.y - scala); }
	__forceinline Vector2 operator *(const Vector2& v, float scala) { return Vector2(v.x * scala, v.y * scala); }
	__forceinline Vector2 operator /(const Vector2& v, float scala) { return Vector2(v.x / scala, v.y / scala); }
	__forceinline Vector2 operator *(float scala, const Vector2& v) { return Vector2(v.x * scala, v.y * scala); }
	__forceinline Vector2 operator -(float scala, const Vector2& v) { return Vector2(scala - v.x, scala - v.y); }
	__forceinline Vector2 operator -(const Vector2& v) { return Vector2(-v.x, -v.y); }

	struct Vector3 {
	public:
		union {
			struct { float x, y, z; };
			float v[3];
			Vector2 xy;
		};
		/**
		*@brief 型がintのVector3<br>
		*値を取る為のみ使用なので、メンバ関数は提供しない
		*/
		struct IVector3 {
			union {
				struct { int x, y, z; };
				int v[3];
			};
			__forceinline IVector3(int x, int y, int z) :x(x), y(y), z(z) {}
		};
		__forceinline Vector3(float x, float y, float z) noexcept : x(x), y(y), z(z) {}
		__forceinline Vector3()noexcept : Vector3(0.0f, 0.0f, 0.0f) {}
		/**
		*@brief 値をint型で取得する関数
		*/
		__forceinline IVector3 Get()const  noexcept { return IVector3(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z)); }
		/**
		*@brief int型のIVector3からの直接Vector3に代入
		*/
		__forceinline void Set(IVector3 v)noexcept {
			for (int i = 0; i < 3; i++) {
				this->v[i] = static_cast<float>(v.v[i]);
			}
		}
		/**@brief 距離(平方根あり)*/
		__forceinline float Length()const { return sqrtf(x*x + y * y + z * z); }
		/**@brief 距離(平方根なし)*/
		__forceinline float LengthSq()const { return (x*x + y * y + z * z); }
		/**@brief 正規化*/
		__forceinline void Normalize() {
			float l = Length();
			if (l != 0.0f) { x /= l;	y /= l; z /= l; }
		}
		__forceinline bool IsEqual(const Vector3& v) { return (FloatEqual(x, v.x) && FloatEqual(y, v.y) && FloatEqual(z, v.z)); }
		/**@brief 内積*/
		__forceinline static float Dot(const Vector3& v1, const Vector3& v2) { return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z); }
		/**@brief 外積*/
		__forceinline static Vector3 Cross(const Vector3& v1, const Vector3& v2) {
			Vector3 ret = {};
			ret.x = v1.y*v2.z - v1.z*v2.y;
			ret.y = v1.z*v2.x - v1.x*v2.z;
			ret.z = v1.x*v2.y - v1.y*v2.x;
			return ret;
		}
		__forceinline Vector3 operator +(const Vector3& v) { return Vector3(x + v.x, y + v.y, z + v.z); }
		__forceinline Vector3 operator -(const Vector3& v) { return Vector3(x - v.x, y - v.y, z - v.z); }
		__forceinline Vector3 operator *(const Vector3& v) { return Vector3(x * v.x, y * v.y, z * v.z); }
		__forceinline Vector3 operator /(const Vector3& v) { return Vector3(x / v.x, y / v.y, z / v.z); }
		__forceinline Vector3 operator +(float scala) { return Vector3(x + scala, y + scala, z + scala); }
		__forceinline Vector3 operator -(float scala) { return Vector3(x - scala, y - scala, z - scala); }
		__forceinline Vector3 operator *(float scala) { return Vector3(x * scala, y * scala, z * scala); }
		__forceinline Vector3 operator /(float scala) { return Vector3(x / scala, y / scala, z / scala); }
		__forceinline Vector3 operator =(float scala) { x = scala; y = scala; z = scala; return *this; }
		__forceinline Vector3& operator +=(const Vector3& v) { x += v.x; y += v.y; z += v.z; return *this; }
		__forceinline Vector3& operator -=(const Vector3& v) { x -= v.x;	y -= v.y;	z -= v.z; return *this; }
		__forceinline Vector3& operator *=(const Vector3& v) { x *= v.x;	y *= v.y;	z *= v.z; return *this; }
		__forceinline Vector3& operator /=(const Vector3& v) { x /= v.x;	y /= v.y;	z /= v.z; return *this; }
		__forceinline Vector3& operator +=(float scala) { x += scala;	y += scala;	z += scala; return *this; }
		__forceinline Vector3& operator -=(float scala) { x -= scala;	y -= scala;	z -= scala; return *this; }
		__forceinline Vector3& operator *=(float scala) { x *= scala;	y *= scala;	z *= scala; return *this; }
		__forceinline Vector3& operator /=(float scala) { x /= scala;	y /= scala;	z /= scala; return *this; }
		__forceinline bool operator ==(const Vector3& v) { return IsEqual(v); }
		__forceinline bool operator !=(const Vector3& v) { return !IsEqual(v); }
	};
	__forceinline Vector3 operator *(float scala, const Vector3& v) { return Vector3(v.x * scala, v.y * scala, v.z * scala); }
	__forceinline Vector3 operator *(const Vector3& v, float scala) { return Vector3(v.x * scala, v.y * scala, v.z * scala); }
	__forceinline Vector3 operator -(float scala, const Vector3& v) { return Vector3(scala - v.x, scala - v.y, scala - v.z); }
	__forceinline Vector3 operator -(const Vector3& v0, const Vector3& v1) { return Vector3(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z); }
	__forceinline Vector3 operator +(const Vector3& v0, const Vector3& v1) { return Vector3(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z); }
	__forceinline Vector3 operator -(const Vector3& v) { return Vector3(-v.x, -v.y, -v.z); }
	__forceinline bool operator ==(const Vector3& v0, const Vector3& v1) { return (v0.x == v1.x&&v0.y == v1.y&&v0.z == v1.z); }

	struct Vector4 {
	public:
		union {
			union {
				struct { float x, y, z, w; };
				float v[4];
				Vector3 xyz;
				Vector2 xy;
			};
		};
		/**
		*@brief 型がintのVector4<br>
		*値を取る為のみ使用なので、メンバ関数は提供しない
		*/
		struct IVector4 {
			union {
				struct { int x, y, z, w; };
				int v[4];
			};
			__forceinline IVector4(int x, int y, int z, int w) :x(x), y(y), z(z), w(w) {}
		};
		__forceinline Vector4(float x, float y, float z, float w) noexcept :x(x), y(y), z(z), w(w) {}
		__forceinline Vector4()noexcept : Vector4(0.0f, 0.0f, 0.0f, 0.0f) {}
		/**
		*@brief 値をint型で取得する関数
		*/
		__forceinline IVector4 Get()const  noexcept { return IVector4(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z), static_cast<int>(w)); }
		/**
		*@brief int型のIVector4からの直接Vector4に代入
		*/
		__forceinline void Set(IVector4 v)noexcept {
			for (int i = 0; i < 4; i++) {
				this->v[i] = static_cast<float>(v.v[i]);
			}
		}
		/**@brief 距離(平方根あり)*/
		__forceinline float Length()const { return sqrtf(x*x + y * y + z * z + w * w); }
		/**@brief 距離(平方根なし)*/
		__forceinline float LengthSq()const { return (x*x + y * y + z * z + w * w); }
		/**@brief 正規化*/
		__forceinline void Normalize() {
			float l = Length();
			if (l != 0.0f) { x /= l;	y /= l; z /= l; }
		}
		__forceinline bool IsEqual(const Vector4& v) { return (FloatEqual(x, v.x) && FloatEqual(y, v.y) && FloatEqual(z, v.z) && FloatEqual(w, v.w)); }
		/**@brief 内積*/
		__forceinline static float Dot(const Vector4& v1, const Vector4& v2) { return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w); }
		__forceinline Vector4 operator +(const Vector4& v) { return Vector4(x + v.x, y + v.y, z + v.z, w + v.w); }
		__forceinline Vector4 operator -(const Vector4& v) { return Vector4(x - v.x, y - v.y, z - v.z, w - v.w); }
		__forceinline Vector4 operator *(const Vector4& v) { return Vector4(x * v.x, y * v.y, z * v.z, w * v.w); }
		__forceinline Vector4 operator /(const Vector4& v) { return Vector4(x / v.x, y / v.y, z / v.z, w / v.w); }
		__forceinline Vector4 operator +(float scala) { return Vector4(x + scala, y + scala, z + scala, w + scala); }
		__forceinline Vector4 operator -(float scala) { return Vector4(x - scala, y - scala, z - scala, w - scala); }
		__forceinline Vector4 operator *(float scala) { return Vector4(x * scala, y * scala, z * scala, w * scala); }
		__forceinline Vector4 operator /(float scala) { return Vector4(x / scala, y / scala, z / scala, w / scala); }
		__forceinline Vector4 operator =(float scala) { x = scala; y = scala; z = scala; w = scala; return *this; }
		__forceinline Vector4& operator +=(const Vector4& v) { x += v.x;	y += v.y;	z += v.z; w += v.w; return *this; }
		__forceinline Vector4& operator -=(const Vector4& v) { x -= v.x;	y -= v.y;	z -= v.z; w -= v.w; return *this; }
		__forceinline Vector4& operator *=(const Vector4& v) { x *= v.x;	y *= v.y;	z *= v.z; w *= v.w; return *this; }
		__forceinline Vector4& operator /=(const Vector4& v) { x /= v.x;	y /= v.y;	z /= v.z; w /= v.w; return *this; }
		__forceinline Vector4& operator +=(float scala) { x += scala;	y += scala;	z += scala; w += scala; return *this; }
		__forceinline Vector4& operator -=(float scala) { x -= scala;	y -= scala;	z -= scala; w -= scala; return *this; }
		__forceinline Vector4& operator *=(float scala) { x *= scala;	y *= scala;	z *= scala; w *= scala; return *this; }
		__forceinline Vector4& operator /=(float scala) { x /= scala;	y /= scala;	z /= scala; w /= scala; return *this; }
		__forceinline bool operator ==(const Vector4& v) { return IsEqual(v); }
		__forceinline bool operator !=(const Vector4& v) { return !IsEqual(v); }
	};
	__forceinline Vector4 operator *(float scala, const Vector4& v) { return Vector4(v.x * scala, v.y * scala, v.z * scala, v.w * scala); }
	__forceinline Vector4 operator -(float scala, const Vector4& v) { return Vector4(scala - v.x, scala - v.y, scala - v.z, scala - v.w); }
	__forceinline Vector4 operator -(const Vector4& v) { return Vector4(-v.x, -v.y, -v.z, -v.w); }
	//カメラ視点でnearからfarの順番で反時計回り
	using FrustumVertices = std::array<Math::Vector3, 8>;
	inline void CreateFrustumVertices(const Vector3& pos, const Vector3& target, const Vector3& up, float fov, float near_z, float far_z, float aspect, FrustumVertices* frustum) {
		//カメラ空間軸を求める
		Vector3 camZ = target - pos; camZ.Normalize();
		Vector3 camX = Vector3::Cross(up, camZ); camX.Normalize();
		Vector3 camY = Vector3::Cross(camZ, camX); camY.Normalize();
		//視野角から高さを算出
		float nearHeight = tanf(fov * 0.5f) * near_z;
		float farHeight = tanf(fov * 0.5f) * far_z;
		//アスペクト比から横幅を求める
		float nearWidth = nearHeight * aspect;
		float farWidth = farHeight * aspect;
		//平面の中心点を算出
		Vector3 nearPlaneCenter = pos + camZ * near_z;
		Vector3 farPlaneCenter = pos + camZ * far_z;
		//各頂点算出
		//カメラ視点でnearからfarの順番で反時計回り
		//near平面
		//左上
		(*frustum)[0] = nearPlaneCenter - camX * nearWidth + camY * nearHeight;
		//左下
		(*frustum)[1] = nearPlaneCenter - camX * nearWidth - camY * nearHeight;
		//右下
		(*frustum)[2] = nearPlaneCenter + camX * nearWidth - camY * nearHeight;
		//右上
		(*frustum)[3] = nearPlaneCenter + camX * nearWidth + camY * nearHeight;
		//far平面
		//左上
		(*frustum)[4] = farPlaneCenter - camX * farWidth + camY * farHeight;
		//左下
		(*frustum)[5] = farPlaneCenter - camX * farWidth - camY * farHeight;
		//右下
		(*frustum)[6] = farPlaneCenter + camX * farWidth - camY * farHeight;
		//右上
		(*frustum)[7] = farPlaneCenter + camX * farWidth + camY * farHeight;
	}
	__forceinline float CalcRadianToVectors(const Vector2& v1, const Vector2& v2)noexcept {
		Vector2 temp1 = v1, temp2 = v2;
		float cosTheata = Vector2::Dot(temp1, temp2) / (temp1.Length()*temp2.Length());
		float rad = acosf(cosTheata);
		float cross = Vector2::Cross(temp1, temp2);
		if (cross < 0.0f)rad *= -1;
		return rad;
	}
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// 線分と点の最短距離を求める
	__forceinline float CalcPointSegmentLength(const Math::Vector3& p0, const Math::Vector3& sp, const Math::Vector3& ep) {
		Lobelia::Math::Vector3 sp_ep = ep - sp;
		Lobelia::Math::Vector3 sp_p0 = p0 - sp;
		Lobelia::Math::Vector3 ep_p0 = p0 - ep;
		// 線分が縮退している場合
		if (sp_ep.Length() <= .0f)
			return (p0 - sp).Length();
		if (Lobelia::Math::Vector3::Dot(sp_ep, sp_p0) < 0)  return sp_p0.Length();
		if (Lobelia::Math::Vector3::Dot(-sp_ep, ep_p0) < 0) return ep_p0.Length();
		return (Lobelia::Math::Vector3::Cross(sp_ep, sp_p0).Length() / sp_ep.Length());
	}
	// 線分と点の最短距離を求める(垂線の足座標取得ver,計算コストはちょい高い)
	__forceinline float CalcPointSegmentLength(const Math::Vector3& p0, const Math::Vector3& sp, const Math::Vector3& ep, Lobelia::Math::Vector3* h_position) {
		Lobelia::Math::Vector3 sp_ep = ep - sp;
		Lobelia::Math::Vector3 sp_p0 = p0 - sp;
		Lobelia::Math::Vector3 ep_p0 = p0 - ep;
		if (Lobelia::Math::Vector3::Dot(sp_ep, sp_p0) < 0) {
			*h_position = sp;
			return sp_p0.Length();
		}
		if (Lobelia::Math::Vector3::Dot(-sp_ep, ep_p0) < 0) {
			*h_position = ep;
			return ep_p0.Length();
		}
		Lobelia::Math::Vector3 sp_ep_normalize = sp_ep;
		sp_ep_normalize.Normalize();
		float t = Lobelia::Math::Vector3::Dot(sp_p0, sp_ep_normalize);
		*h_position = sp + sp_ep_normalize * t;
		return (*h_position - p0).Length();
	}
	// 線分と線分の最短距離を求める
	__forceinline float CalcSegmentSegmentLength(const Math::Vector3& sp0, const Math::Vector3& ep0, const Math::Vector3& sp1, const Math::Vector3& ep1) {
		// 0~1の間にクランプする
		auto Saturate = [](float val)->float {
			float ret = val;
			if (ret >= 1.0f) ret = 1.0f;
			else if (ret <= .0f)  ret = .0f;
			return ret;
		};
		// 各線分のベクトル
		Math::Vector3 sp0_ep0 = ep0 - sp0;
		Math::Vector3 sp0_ep0_n = sp0_ep0; sp0_ep0_n.Normalize();
		Math::Vector3 sp1_ep1 = ep1 - sp1;
		Math::Vector3 sp1_ep1_n = sp1_ep1; sp1_ep1_n.Normalize();
		// 垂線の足
		Lobelia::Math::Vector3 p1, p2;
		// ベクトル係数
		float t1, t2;

		// 線分0が縮退している場合
		if (sp0_ep0.Length() < FLT_EPSILON) {
			if (sp1_ep1.Length() < FLT_EPSILON) return (sp1 - sp0).Length();
			else return CalcPointSegmentLength(sp0, sp1, ep1);
		}
		// 線分1が縮退している場合
		else if (sp1_ep1.Length() < FLT_EPSILON) return CalcPointSegmentLength(sp1, sp0, ep0);  // 点と線分の距離判定

		// 線分同士が平行な場合
		if (FloatEqual(Math::Vector3::Cross(sp0_ep0, sp1_ep1).Length(), .0f)) {
			Math::Vector3 h(.0f, .0f, .0f);
			float length = CalcPointSegmentLength(sp0, sp1, ep1, &h);
			// ベクトル係数を計算
			t2 = (h - sp1).Length() / sp1_ep1.Length();
			if (.0f <= t2 && t2 <= 1.0f) return length;
		}
		// 線分同士がねじれの関係にある場合
		else {
			float dot_v1v2 = Math::Vector3::Dot(sp0_ep0_n, sp1_ep1);
			Math::Vector3 sp0_sp1 = sp1 - sp0;
			// ベクトル係数1を計算
			t1 = (dot_v1v2 * Math::Vector3::Dot(sp1_ep1, sp0_sp1)
				- sp1_ep1.LengthSq() * Math::Vector3::Dot(sp0_ep0, sp0_sp1)) / (sp0_ep0.LengthSq() * sp1_ep1.LengthSq() - dot_v1v2 * dot_v1v2);
			// ベクトル係数2を計算
			Lobelia::Math::Vector3 point1 = sp0 + sp0_ep0_n * t1;
			Lobelia::Math::Vector3 v = point1 - sp1; v.Normalize();
			t2 = Math::Vector3::Dot(sp1_ep1_n, v);
			// 最短距離(仮)を計算
			Lobelia::Math::Vector3 point2 = sp1 + sp1_ep1_n * t2;
			float length = (point2 - point1).Length();
			if (.0f <= t1 && t1 <= 1.0f && .0f <= t2 && t2 <= 1.0f) {
				return length;
			}
		}
		// 垂線の足が外側にあることが判明
		t1 = Saturate(t1);
		p1 = sp0 + sp0_ep0_n * t1;
		Math::Vector3 h(.0f, .0f, .0f);
		float length = CalcPointSegmentLength(sp0, sp1, ep1, &h);
		// ベクトル係数を計算
		t2 = (h - sp1).Length() / sp1_ep1.Length();
		if (.0f <= t2 && t2 <= 1.0f) return length;

		// 線分2が外であることが判明
		t2 = Saturate(t2);
		p2 = sp1 + sp1_ep1_n * t2;
		length = CalcPointSegmentLength(sp0, sp1, ep1, &h);
		t1 = (h - sp0).Length() / sp0_ep0.Length();
		if (0.0f <= t1 && t1 <= 1.0f) return length;
		t1 = Saturate(t1);
		p1 = sp0 + sp0_ep0_n * t1;

		return (p2 - p1).Length();
	}
	//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	__forceinline Math::Vector3 Lerp(const Math::Vector3& v0, const Math::Vector3& v1, float ratio) {
		Math::Vector3 ret;
		ret = v0 - (v0 - v1) * ratio;
		ret.Normalize();
		return ret;
	}
	__forceinline Math::Vector3 CalcNormal(const Math::Vector3& p0, const Math::Vector3& p1, const Math::Vector3& p2) { return Math::Vector3::Cross(p1 - p0, p2 - p1); }
	__forceinline Math::Vector4 CalcNormal(const Math::Vector4& p0, const Math::Vector4& p1, const Math::Vector4& p2) {
		Math::Vector4 ret;
		ret.xyz = Math::Vector3::Cross(p1.xyz - p0.xyz, p2.xyz - p1.xyz);
		return ret;
	}
	//vpvp(view*projection*viewport)
	__forceinline Math::Vector2 CalcScreenPos(const DirectX::XMMATRIX vpvp, const Math::Vector3& pos) {
		DirectX::XMFLOAT4 storagePos(pos.x, pos.y, pos.z, 1.0f);
		DirectX::XMVECTOR calcPos = DirectX::XMLoadFloat4(&storagePos);
		//行列合成
		calcPos = DirectX::XMVector3Transform(calcPos, vpvp);
		DirectX::XMStoreFloat4(&storagePos, calcPos);
		//NDC正規化
		Math::Vector3 tempPos(storagePos.x, storagePos.y, storagePos.z);
		tempPos /= storagePos.w;
		return tempPos.xy;
	}
	template<class T, class... Args> constexpr T Max(T&& head, Args&&... tail) {
		T result = head;
		using swallow = std::initializer_list<int>;
		(void)swallow {
			(void(result = max(result, tail)), 0)...
		};
		return result;
	}
	template<class T, class... Args> constexpr T Min(T&& head, Args&&... tail) {
		T result = head;
		using swallow = std::initializer_list<int>;
		(void)swallow {
			(void(result = min(result, tail)), 0)...
		};
		return result;
	}
}
