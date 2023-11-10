#pragma once

#include "Common.h"
#include "ER_Ray.h"

namespace EveryRay_Core
{
	enum FrustumPlane
	{
		FrustumPlaneNear = 0,
		FrustumPlaneFar,
		FrustumPlaneLeft,
		FrustumPlaneRight,
		FrustumPlaneTop,
		FrustumPlaneBottom
	};

	struct FrustumDistance
	{
		float NearDistance;
		float FarDistance;
	};

	class ER_Frustum
	{
	public:
		ER_Frustum(CXMMATRIX matrix);

		const XMFLOAT4& Near() const;
		const XMFLOAT4& Far() const;
		const XMFLOAT4& Left() const;
		const XMFLOAT4& Right() const;
		const XMFLOAT4& Top() const;
		const XMFLOAT4& Bottom() const;

		XMVECTOR NearVector() const;
		XMVECTOR FarVector() const;
		XMVECTOR LeftVector() const;
		XMVECTOR RightVector() const;
		XMVECTOR TopVector() const;
		XMVECTOR BottomVector() const;

		const XMFLOAT3* Corners() const;
		const XMFLOAT4* Planes() const;


		XMMATRIX Matrix() const;
		void SetMatrix(CXMMATRIX matrix);
		void SetMatrix(const XMFLOAT4X4& matrix);

	private:
		ER_Frustum();

		static ER_Ray ComputeIntersectionLine(FXMVECTOR p1, FXMVECTOR p2);
		static XMVECTOR ComputeIntersection(FXMVECTOR& plane, ER_Ray& ray);

		XMFLOAT4X4 mMatrix;
		XMFLOAT3 mCorners[8];
		XMFLOAT4 mPlanes[6];
	};
}
